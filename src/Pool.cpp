/*
 * Copyright (c) 2010 John Nesky
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "Pool.hpp"

void private_allocate(Pool* pool) {
  printf("Allocating new buffers of size %d\n", pool->size);
  
  char* p = (char*)malloc(pool->size * pool->increment + (pool->align32 ? 0x20 : 0));
  // TODO: Figure out alignment:
  //if (pool->align32) p = (p & ~0x1f) + 0x20;
  
  for (int i = 0; i < pool->increment; i++) {
    Pool_return(pool, p);
    p += pool->size;
  }
  
  pool->increment++;
}


void Pool_init(Pool* pool, size_t size, bool align32) {
  pool->align32 = align32;
  if (align32 && size & 0x1f) {
    size = (size & ~0x1f) + 0x20;
  }
  pool->size = size;
  pool->increment = 4;
  pool->list = 0;
  private_allocate(pool);
}

void* Pool_draw(Pool* pool) {
  if (pool->list == 0) {
    private_allocate(pool);
  }
  SinglyLinkedList* l = pool->list;
  void* result = l->val;
  pool->list = l->next;
  free(l);
  return result;
}

void Pool_return(Pool* pool, void* item) {
  SinglyLinkedList* l = (SinglyLinkedList*) malloc(sizeof(SinglyLinkedList));
  l->val = item;
  l->next = pool->list;
  pool->list = l;
}
