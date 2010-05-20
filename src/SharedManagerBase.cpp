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

#include "SharedManagerBase.hpp"
#include "memoryBarrier.h"

std::vector<SharedManagerBase*> SharedManagerBase::dirtyManagers;
std::vector<SharedManagerBase*> SharedManagerBase::bloatedManagers;
std::vector<SharedManagerBase*> SharedManagerBase::condemnedManagers;
Sharer<unsigned int, true> SharedManagerBase::writeSharedSyncIndex;
Sharer<unsigned int, false> SharedManagerBase::readSharedSyncIndex;

SharedManagerBase::SharedManagerBase() : dirty(false), clonedIndex(0), condemnedIndex(0) {
  markDirty();
}

SharedManagerBase::~SharedManagerBase() {}

void SharedManagerBase::markDirty() {
  printf("SharedManagerBase::markDirty()\n");
  if (!dirty) {
    dirty = true;
    SharedManagerBase::dirtyManagers.push_back(this);
  }
}

void SharedManagerBase::condemn() {
  printf("SharedManagerBase::condemn()\n");
  
  /// TODO: Update the index in the share() function instead of in condemn()
  ///       by adding the manager to a list like when we mark it as dirty.
  ///       This will likely delay the overflow of writeSharedSyncIndex by
  ///       batching changes into a single increment.
  unsigned int temp = SharedManagerBase::writeSharedSyncIndex.read() + 1;
  SharedManagerBase::writeSharedSyncIndex.write(temp);
  condemnedIndex = temp;
  SharedManagerBase::condemnedManagers.push_back(this);
}

bool SharedManagerBase::isCondemned() {
  return condemnedIndex > 0;
}

void SharedManagerBase::markOldStuffAsUnused() {
  SharedManagerBase::readSharedSyncIndex.write(SharedManagerBase::writeSharedSyncIndex.read());
}

void SharedManagerBase::share() {
  if (SharedManagerBase::dirtyManagers.size() > 0) {
    unsigned int writeSharedSyncIndex = SharedManagerBase::writeSharedSyncIndex.read() + 1;
    while (SharedManagerBase::dirtyManagers.size() > 0) {
      printf("updating dirty managers\n");
      SharedManagerBase* manager = SharedManagerBase::dirtyManagers.back();
      SharedManagerBase::dirtyManagers.pop_back();
      manager->update();
      manager->dirty = false;
      
      manager->clonedIndex = writeSharedSyncIndex;
      
      if (manager->isCondemned()) {
        printf("harvesting condemned manager immediately! %p\n", manager);
        manager->harvest();
      } else {
        printf("pushing extra clones %p\n", manager);
        SharedManagerBase::bloatedManagers.push_back(manager);
      }
    }
    SharedManagerBase::writeSharedSyncIndex.write(writeSharedSyncIndex);
  }
  
  unsigned int readSharedSyncIndex = SharedManagerBase::readSharedSyncIndex.read();
  while (SharedManagerBase::bloatedManagers.size() > 0) {
    printf("inspecting item with extra clones\n");
    SharedManagerBase* manager = SharedManagerBase::bloatedManagers.back();
    if (readSharedSyncIndex >= manager->clonedIndex) {
      printf("harvesting extra clones %p\n", manager);
      manager->harvest();
      SharedManagerBase::bloatedManagers.pop_back();
    } else {
      break; /// TODO: skip and try the next one.
    }
  }
  
  while (SharedManagerBase::condemnedManagers.size() > 0) {
    printf("inspecting condemned item\n");
    SharedManagerBase* manager = SharedManagerBase::condemnedManagers.back();
    if (readSharedSyncIndex >= manager->condemnedIndex) {
      printf("killing abandoned item %p\n", manager);
      SharedManagerBase::condemnedManagers.pop_back();
      manager->abandon();
      delete manager;
    } else {
      break; /// TODO: skip and try the next one.
    }
  }
}
