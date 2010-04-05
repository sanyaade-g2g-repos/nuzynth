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

#ifndef SHARER_H
#define SHARER_H

#include "memoryBarrier.h"

template <class Type, bool fastRead>
class Sharer {
public:
  Sharer(): val1(0), val2(0), readVal1(0), lock(0) {}
  
  void write(Type val) {
    if (fastRead) { // slow write
      while(lock) {
        /// TODO: investigate yield instead of busy wait
      }
      
      if (readVal1) {
        val2 = val;
        // ---------------------------------------------------------------------
        PaUtil_WriteMemoryBarrier();
        // ---------------------------------------------------------------------
        readVal1 = 0;
      } else {
        val1 = val;
        // ---------------------------------------------------------------------
        PaUtil_WriteMemoryBarrier();
        // ---------------------------------------------------------------------
        readVal1 = 1;
      }
      
    } else { // fast write
      /*
      lock = true;
      // -----------------------------------------------------------------------
      PaUtil_WriteMemoryBarrier();
      // -----------------------------------------------------------------------
      */
      if (readVal1) {
        val2 = val;
        // ---------------------------------------------------------------------
        PaUtil_WriteMemoryBarrier();
        // ---------------------------------------------------------------------
        readVal1 = 0;
      } else {
        val1 = val;
        // ---------------------------------------------------------------------
        PaUtil_WriteMemoryBarrier();
        // ---------------------------------------------------------------------
        readVal1 = 1;
      }
      /*
      // -----------------------------------------------------------------------
      PaUtil_WriteMemoryBarrier();
      // -----------------------------------------------------------------------
      lock = false;
      */
    }
  }
  
  Type read() {
    if (fastRead) {
      lock = true;
      // -----------------------------------------------------------------------
      PaUtil_FullMemoryBarrier();
      // -----------------------------------------------------------------------
      Type result = readVal1 ? val1 : val2;
      // -----------------------------------------------------------------------
      PaUtil_FullMemoryBarrier();
      // -----------------------------------------------------------------------
      lock = false;
      return result;
    } else { // slow read
      Type attempt1;
      Type attempt2;
      
      // Read the data twice in a row. If it's the same both times, 
      // it's good. If not, try until you get a repeat. 
      
      attempt2 = readVal1 ? val1 : val2;
      do {
        
        ///TODO: yield?
        /*
        while (lock) {
          yield?
        }
        */
        
        attempt1 = attempt2;
        // ---------------------------------------------------------------------
        PaUtil_ReadMemoryBarrier();
        // ---------------------------------------------------------------------
        attempt2 = readVal1 ? val1 : val2;
      } while (attempt1 != attempt2);
      
      return attempt1;
    }
  }
  
private:
  Type val1;
  Type val2;
  bool readVal1;
  bool lock;
};

#endif // SHARER_H
