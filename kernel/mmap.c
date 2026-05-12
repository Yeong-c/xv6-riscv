// edit in project3
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "proc.h"
#include "defs.h"

struct mmap_area mmap_areas[NMMAP];
struct spinlock mmap_lock;

void
mmapinit(void)
{
  initlock(&mmap_lock, "mmap");
  for(int i = 0; i < NMMAP; i++)
    mmap_areas[i].used = 0;
}

static struct mmap_area *
alloc_slot(void)
{
  acquire(&mmap_lock);
  for(int i = 0; i < NMMAP; i++){
    if(!mmap_areas[i].used){
      memset(&mmap_areas[i], 0, sizeof(mmap_areas[i]));
      mmap_areas[i].used = 1;
      release(&mmap_lock);
      return &mmap_areas[i];
    }
  }
  release(&mmap_lock);
  return 0;
}

uint64
mmap(uint64 addr, int length, int prot, int flags, int fd, int offset)
{
  struct proc *p = myproc();
  struct file *f = 0;

  if(addr % PGSIZE != 0) return 0;
  if(length <= 0 || (length % PGSIZE) != 0) return 0;

  int anon = (flags & MAP_ANONYMOUS) ? 1 : 0;

  if(!anon){
    if(fd < 0 || fd >= NOFILE) return 0;
    f = p->ofile[fd];
    if(f == 0) return 0;
    if((prot & PROT_READ) && !f->readable) return 0;
    if((prot & PROT_WRITE) && !f->writable) return 0;
    if(offset < 0) return 0;
  } else {
    if(fd != -1) return 0;
    if(offset != 0) return 0;
  }

  uint64 va_start = MMAPBASE + addr;
  struct mmap_area *area = alloc_slot();
  if(!area) return 0;

  area->addr = va_start;
  area->length = length;
  area->offset = offset;
  area->prot = prot;
  area->flags = flags;
  area->p = p;
  area->f = anon ? 0 : filedup(f);

  if(flags & MAP_POPULATE){
    for(uint64 a = va_start; a < va_start + length; a += PGSIZE){
      char *mem = kalloc();
      if(!mem) goto fail;
      memset(mem, 0, PGSIZE);
      if(area->f){
        ilock(area->f->ip);
        readi(area->f->ip, 0, (uint64)mem, offset + (a - va_start), PGSIZE);
        iunlock(area->f->ip);
      }
      int perm = PTE_U;
      if(prot & PROT_READ) perm |= PTE_R;
      if(prot & PROT_WRITE) perm |= PTE_W;
      if(mappages(p->pagetable, a, PGSIZE, (uint64)mem, perm) < 0){
        kfree(mem);
        goto fail;
      }
    }
  }
  return va_start;

fail:
  for(uint64 a = va_start; a < va_start + length; a += PGSIZE){
    pte_t *pte = walk(p->pagetable, a, 0);
    if(pte && (*pte & PTE_V)){
      kfree((void*)PTE2PA(*pte));
      *pte = 0;
    }
  }
  if(area->f) fileclose(area->f);
  area->used = 0;
  return 0;
}

int
munmap(uint64 addr)
{
  struct proc *p = myproc();
  uint64 target = (addr < MMAPBASE) ? (addr + MMAPBASE) : addr;

  for(int i = 0; i < NMMAP; i++){
    struct mmap_area *a = &mmap_areas[i];
    if(!a->used || a->p != p || a->addr != target) continue;

    for(uint64 va = a->addr; va < a->addr + a->length; va += PGSIZE){
      pte_t *pte = walk(p->pagetable, va, 0);
      if(pte && (*pte & PTE_V)){
        kfree((void *)PTE2PA(*pte));
        *pte = 0;
      }
    }
    if(a->f) fileclose(a->f);
    a->used = 0;
    return 1;
  }
  return -1;
}

int
mmap_handle_fault(uint64 va, int write)
{
  struct proc *p = myproc();
  uint64 page_va = PGROUNDDOWN(va);

  struct mmap_area *area = 0;
  for(int i = 0; i < NMMAP; i++){
    struct mmap_area *a = &mmap_areas[i];
    if(!a->used || a->p != p) continue;
    if(page_va < a->addr || page_va >= a->addr + a->length) continue;
    area = a;
    break;
  }
  if(!area) return -1;
  if(write && !(area->prot & PROT_WRITE)) return -1;

  char *mem = kalloc();
  if(!mem) return -1;
  memset(mem, 0, PGSIZE);
  if(area->f){
    ilock(area->f->ip);
    readi(area->f->ip, 0, (uint64)mem, area->offset + (page_va - area->addr), PGSIZE);
    iunlock(area->f->ip);
  }
  int perm = PTE_U;
  if(area->prot & PROT_READ) perm |= PTE_R;
  if(area->prot & PROT_WRITE) perm |= PTE_W;
  if(mappages(p->pagetable, page_va, PGSIZE, (uint64)mem, perm) < 0){
    kfree(mem);
    return -1;
  }
  return 1;
}

void
mmap_copy(struct proc *parent, struct proc *child)
{
  for(int i = 0; i < NMMAP; i++){
    struct mmap_area *src = &mmap_areas[i];
    if(!src->used || src->p != parent) continue;

    struct mmap_area *dst = alloc_slot();
    if(!dst) break;
    dst->addr = src->addr;
    dst->length = src->length;
    dst->offset = src->offset;
    dst->prot = src->prot;
    dst->flags = src->flags;
    dst->p = child;
    dst->f = src->f ? filedup(src->f) : 0;

    for(uint64 va = src->addr; va < src->addr + src->length; va += PGSIZE){
      pte_t *pte = walk(parent->pagetable, va, 0);
      if(!pte || !(*pte & PTE_V)) continue;
      char *mem = kalloc();
      if(!mem) break;
      memmove(mem, (char *)PTE2PA(*pte), PGSIZE);
      if(mappages(child->pagetable, va, PGSIZE, (uint64)mem, PTE_FLAGS(*pte)) < 0){
        kfree(mem);
        break;
      }
    }
  }
}

void
mmap_cleanup(struct proc *p)
{
  for(int i = 0; i < NMMAP; i++){
    struct mmap_area *a = &mmap_areas[i];
    if(!a->used || a->p != p) continue;

    for(uint64 va = a->addr; va < a->addr + a->length; va += PGSIZE){
      pte_t *pte = walk(p->pagetable, va, 0);
      if(pte && (*pte & PTE_V)){
        kfree((void *)PTE2PA(*pte));
        *pte = 0;
      }
    }
    if(a->f) fileclose(a->f);
    a->used = 0;
  }
}
