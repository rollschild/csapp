/* Private global variables */
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WSIZE 4              // word and header/footer size (bytes)
#define DSIZE 8              // double word size
#define CHUNK_SIZE (1 << 12) // extend heap by this amount (bytes)

#define MAX_HEAP 4096

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

/**
 * read & write word at address p
 */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/**
 * Read size and allocated fields from a header/footer at address p
 */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/**
 * given block ptr bp, compute address of its header & footer
 */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/**
 * given block ptr bp, compute address of next and previous blocks
 */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

static char *mem_heap;     // points to first byte of heap
static char *mem_brk;      // points to last byte of heap plus 1
static char *mem_max_addr; // max legal heap addr plus 1
static char *heap_listp;

void *mem_sbrk(int);
static void *extend_heap(size_t);

/**
 * coalesce - use boundary-tag coalescing to merge the freed block with any
 * adjacent free blocks in constant time
 */
static void *coalesce(void *bp) {
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  // case 1
  if (prev_alloc && next_alloc) {
    return bp;
  } else if (prev_alloc && !next_alloc) {
    // case 2
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  } else if (!prev_alloc && next_alloc) {
    // case 3
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  } else {
    // case 4
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP((PREV_BLKP(bp))), PACK(size, 0));
    PUT(FTRP((NEXT_BLKP(bp))), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }

  return bp;
}

/**
 * find_fit
 */
static void *find_fit(size_t a_size) {
  void *bp;
  for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    if (!GET_ALLOC(HDRP(bp)) && (a_size <= GET_SIZE(HDRP(bp)))) {
      return bp;
    }
  }

  return NULL;
}

/**
 * place
 */
static void place(void *bp, size_t a_size) {
  size_t c_size = GET_SIZE(HDRP(bp));

  if ((c_size - a_size) >= (2 * DSIZE)) {
    // split the block
    PUT(HDRP(bp), PACK(a_size, 1));
    PUT(FTRP(bp), PACK(a_size, 1));
    bp = NEXT_BLKP(bp);

    PUT(HDRP(bp), PACK(c_size - a_size, 0));
    PUT(FTRP(bp), PACK(c_size - a_size, 0));
  } else {
    PUT(HDRP(bp), PACK(c_size, 1));
    PUT(FTRP(bp), PACK(c_size, 1));
  }
}

/**
 * mm_malloc - allocates a block from the free list
 */
void *mm_malloc(size_t size) {
  size_t a_size;      // adjusted block size
  size_t extend_size; // amount to extend heap if no fit
  char *bp;

  // ignore spurious requests
  if (size == 0) {
    return NULL;
  }

  // adjust block size to include overhead and alignment requirements
  if (size <= DSIZE) {
    a_size = 2 * DSIZE;
  } else {
    a_size = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
  }

  // search the free list for a fit
  if ((bp = find_fit(a_size)) != NULL) {
    place(bp, a_size);
    return bp;
  }

  // NO fit found; get more memory and place the block
  extend_size = MAX(a_size, CHUNK_SIZE);
  if ((bp = extend_heap(extend_size / WSIZE)) == NULL) {
    return NULL;
  }

  place(bp, a_size);
  return bp;
}

/**
 * mm_free - frees a block
 */
void mm_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

/**
 * extend_heap - extend the heap with a new free block
 */
static void *extend_heap(size_t words) {
  char *bp;
  size_t size;

  // allocate an even number of words to maintain alignment
  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  if ((long)(bp = mem_sbrk(size)) == -1) {
    return NULL;
  }

  // initialize free block header/footer and epilogue header
  PUT(HDRP(bp), PACK(size, 0));         // free block header
  PUT(FTRP(bp), PACK(size, 0));         // free block footer
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // NEW epilogue header

  // coalesce if the previous block was free
  return coalesce(bp);
}

/**
 * mem_init - initialize the memory system model
 */
void mem_init(void) {
  mem_heap = (char *)malloc(MAX_HEAP);
  mem_brk = (char *)mem_heap;
  mem_max_addr = (char *)(mem_heap + MAX_HEAP);
}

/**
 * mem_sbrk - extends the heap by incr bytes and return the start address of the
 * new area
 */
void *mem_sbrk(int incr) {
  char *old_brk = mem_brk;

  if ((incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
    errno = ENOMEM;
    fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
    return (void *)-1;
  }

  mem_brk += incr;
  return (void *)old_brk;
}

/**
 * mm_init - create a heap with an initial free block
 */
int mm_init(void) {
  // create the initial empty heap
  if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) {
    return -1;
  }

  PUT(heap_listp, 0);                            // alignment padding
  PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // prologue header
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // prologue footer
  PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     // epilogue header
  heap_listp += (2 * WSIZE);

  // extend the empty heap with a free block of CHUNKSIZE bytes
  if (extend_heap(CHUNK_SIZE / WSIZE) == NULL) {
    return -1;
  }

  return 0;
}
