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

#ifndef SHARED_MANAGER_BASE_H
#define SHARED_MANAGER_BASE_H

#include <stdio.h>
#include <vector>
#include "Sharer.h"

class SharedManagerBase {
public:
  SharedManagerBase();
  ~SharedManagerBase();
  
  // Indicate that the manager's data should be cloned and shared
  // next time updateClones() is called. 
  void markSharedDataDirty();
  
  // Indicate that the manager is scheduled to be deleted as soon as all
  // threads stop using it:
  void condemn();
  
  bool isCondemned();
  
  // To be called periodically to update the clones of the shared data:
  static void updateClones();
  
  // To be called to indicate that no threads are using any of the 
  // outdated clones anymore. 
  static void updateReceivedCloneCount();
  
  // To be called to indicate that no threads are using any of the 
  // condemned managers anymore. 
  static void abandonCondemnedManagers();
  
  // To be called periodically to deallocate abandoned managers. 
  static void deleteAbandonedManagers();
  
protected:
  virtual void updateClone() = 0;
  virtual void harvestExtraClones() = 0;
  
private:
  bool dirty;
  unsigned int clonedIndex;
  unsigned int condemnedIndex;
  
  bool isAbandoned();
  
  static std::vector<SharedManagerBase*> managersWithOutdatedClones;
  static std::vector<SharedManagerBase*> managersWithExtraClones;
  static std::vector<SharedManagerBase*> condemnedManagers;
  static Sharer<unsigned int, true> cloneCount;
  static Sharer<unsigned int, false> receivedCloneCount;
  static Sharer<unsigned int, true> condemnedCount;
  static Sharer<unsigned int, false> abandonedCount;
};

#endif // SHARED_MANAGER_BASE_H
