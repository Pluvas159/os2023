// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKETS 13
#define HASH(dev, blockno) ((blockno) % BUCKETS)

struct {
  struct spinlock lock[BUCKETS];
  struct spinlock global_lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[BUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;
  for(int i = 0; i < BUCKETS; i++){
    initlock(&bcache.lock[i], "bcache.bucket");

  }
  initlock(&bcache.global_lock, "bcache_global");

  for(int i = 0; i < BUCKETS; i++){
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }


  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}

void move_to_bucket(struct buf *b, uint blockno) {
  int hash = HASH(b->dev, blockno);
  b->next->prev = b->prev;
  b->prev->next = b->next;
  b->next = bcache.head[hash].next;
  b->prev = &bcache.head[hash];
  bcache.head[hash].next->prev = b;
  bcache.head[hash].next = b;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int hash = HASH(dev, blockno);
  acquire(&bcache.lock[hash]);

  // Is the block already cached?
  for(b = bcache.head[hash].next; b != &bcache.head[hash]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[hash]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.lock[hash]);
  acquire(&bcache.global_lock);
  acquire(&bcache.lock[hash]);

  for(b = bcache.head[hash].next; b != &bcache.head[hash]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[hash]);
      release(&bcache.global_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache.buf; b != bcache.buf + NBUF ; b++){
    int ehash = HASH(b->dev, b->blockno);
    if (ehash != hash) {
      acquire(&bcache.lock[ehash]);
    }
    if(b->refcnt == 0) {
      b->dev = dev;
      if (ehash != hash) {
        move_to_bucket(b, blockno);
        release(&bcache.lock[ehash]);
      }
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[hash]);
      release(&bcache.global_lock);
      acquiresleep(&b->lock);
      return b;
    }
    if (ehash != hash) {
      release(&bcache.lock[ehash]);
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int hash = HASH(b->dev, b->blockno);

  acquire(&bcache.lock[hash]);
  b->refcnt--;
  release(&bcache.lock[hash]);
}

void
bpin(struct buf *b) {
  int hash = HASH(b->dev, b->blockno);
  acquire(&bcache.lock[hash]);
  b->refcnt++;
  release(&bcache.lock[hash]);
}

void
bunpin(struct buf *b) {
  int hash = HASH(b->dev, b->blockno); 
  acquire(&bcache.lock[hash]);
  b->refcnt--;
  release(&bcache.lock[hash]);
}


