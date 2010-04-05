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

#ifndef SHARED_MANAGER_H
#define SHARED_MANAGER_H

#include <vector>
#include "Pool.h"
#include "SharedManagerBase.h"
#include "Sharer.h"
#include "SinglyLinkedList.h"

template <class Clone>
class SharedManager : public SharedManagerBase {
public:
  SharedManager(): SharedManagerBase(), sharedData(0), oldClones(0) {}
  
  Clone* sharedData;
  
  Clone* getSharedDataClone() {
    return sharer.read();
  }
  
protected:
  
  // Override this function! 
  virtual void cleanSharedData() = 0;
  
  // Override this function! 
  virtual void destroyOldClone(Clone* newClone, Clone* oldClone) = 0;
  
  // But don't override this function!
  virtual void updateClone() {
    printf("SharedManager::updateClone()\n");
    // Make any last minute fixes to the data before sharing it:
    cleanSharedData();
    
    // Create a clone and share it:
    publishSharedData();
    Clone* newClone = getSharedDataClone();
    
    // Now add this clone to the list of old clones so that
    // we can destroy it later. 
    SinglyLinkedList *newHead;
    newHead = (SinglyLinkedList*) malloc(sizeof(SinglyLinkedList));
    newHead->val = newClone;
    newHead->next = oldClones;
    oldClones = newHead;
  }
  
  // Don't override this function!
  // Returns true if there's no more than one clone left.
  virtual bool tryHarvestingExtraClones() {
    printf("SharedManager::tryHarvestingExtraClones()\n");
    SinglyLinkedList *node1, *node2;
    node1 = oldClones;
    
    Clone *clone1, *clone2;
    while (node1 != 0) {
      clone1 = (Clone*) node1->val;
      if (clone1->used) {
        // all Clones following this one are unused. 
        
        // first pass: free any unused buffers after node1.
        node2 = node1->next;
        while (node2 != 0) {
          clone2 = (Clone*) node2->val;
          
          destroyOldClone(clone1, clone2);
          
          // This would also be a good place to free any unused loop functions, except I'm 
          // planning on saving those in a map indefinitely...
          clone1 = clone2;
          node2 = node2->next;
        }
        
        // second pass: truncate the linked list after node1.
        node2 = node1->next;
        node1->next = 0;
        while (node2 != 0) {
          node1 = node2;
          node2 = node2->next;
          Pool_return(sharedPool(), node1->val);
          free(node1);
        }
        
        break;
      }
      node1 = node1->next;
    }
    if (oldClones == 0 ||
        oldClones->next == 0)
    {
      return true; // No more than one clone!
    } else {
      return false; // There's still extra clones!
    }
  }
  
  static Pool* sharedPool() {
    printf("SharedManager::sharedPool()\n");
    static Pool* pool = 0;
    if (pool == 0) {
      pool = (Pool*)malloc(sizeof(Pool));
      Pool_init(pool, sizeof(Clone), false);
    }
    return pool;
  }
  
  void publishSharedData() {
    printf("SharedManager::publishSharedData()\n");
    Clone* copy = (Clone*) Pool_draw(sharedPool());
    memcpy(copy, sharedData, sizeof(Clone));
    sharer.write(copy);
  }
  
private:
  Sharer<Clone*, true> sharer;
  SinglyLinkedList* oldClones;
};

#endif // SHARED_MANAGER_H
