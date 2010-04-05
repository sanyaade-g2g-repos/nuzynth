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

#include "SharedManagerBase.h"
#include "memoryBarrier.h"

std::vector<SharedManagerBase*> SharedManagerBase::managersWithOutdatedClones;
std::vector<SharedManagerBase*> SharedManagerBase::managersWithExtraClones;
std::vector<SharedManagerBase*> SharedManagerBase::condemnedManagers;
Sharer<unsigned int, true> SharedManagerBase::condemnedCount;
Sharer<unsigned int, false> SharedManagerBase::abandonedCount;

SharedManagerBase::SharedManagerBase() : dirty(false), condemnedIndex(0) {
  markSharedDataDirty();
}

SharedManagerBase::~SharedManagerBase() {}

void SharedManagerBase::markSharedDataDirty() {
  printf("SharedManagerBase::markSharedDataDirty()\n");
  if (!dirty) {
    dirty = true;
    SharedManagerBase::managersWithOutdatedClones.push_back(this);
  }
}

void SharedManagerBase::condemn() {
  printf("SharedManagerBase::condemn()\n");
  unsigned int temp = SharedManagerBase::condemnedCount.read() + 1;
  SharedManagerBase::condemnedCount.write(temp);
  condemnedIndex = temp;
  SharedManagerBase::condemnedManagers.push_back(this);
}

bool SharedManagerBase::isCondemned() {
  return condemnedIndex > 0;
}

void SharedManagerBase::updateClones() {
  while (SharedManagerBase::managersWithOutdatedClones.size() > 0) {
    printf("updating outdated clones\n");
    SharedManagerBase* manager = SharedManagerBase::managersWithOutdatedClones.back();
    SharedManagerBase::managersWithOutdatedClones.pop_back();
    SharedManagerBase::managersWithExtraClones.push_back(manager);
    manager->updateClone();
    manager->dirty = false;
  }
  
  while (SharedManagerBase::managersWithExtraClones.size() > 0) {
    printf("inspecting item with extra clones\n");
    SharedManagerBase* manager = SharedManagerBase::managersWithExtraClones.back();
    if (manager->tryHarvestingExtraClones()) {
      printf("all extra clones harvested\n");
      SharedManagerBase::managersWithExtraClones.pop_back();
    } else {
      break;
    }
  }
  SharedManagerBase::abandonedCount.write(SharedManagerBase::condemnedCount.read());
}

void SharedManagerBase::abandonCondemnedManagers() {
  SharedManagerBase::abandonedCount.write(SharedManagerBase::condemnedCount.read());
}

void SharedManagerBase::deleteAbandonedManagers() {
  while (SharedManagerBase::condemnedManagers.size() > 0) {
    printf("inspecting condemned item\n");
    SharedManagerBase* manager = SharedManagerBase::condemnedManagers.back();
    if (manager->isAbandoned()) {
      printf("killing abandoned item\n");
      SharedManagerBase::condemnedManagers.pop_back();
      delete manager;
    } else {
      break; /// TODO: skip and try the next one.
    }
  }
}

bool SharedManagerBase::isAbandoned() {
  return (SharedManagerBase::abandonedCount.read() >= condemnedIndex);
}
