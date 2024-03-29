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

#ifndef TRACK_H
#define TRACK_H

#include <stdio.h>
#include <vector>
#include "CloneManager.hpp"
#include "TrackData.hpp"
#include "Note.hpp"

class Instrument;

class Track : public CloneManager<TrackData> {
public:
  
  Track(Instrument* instrument);
  ~Track();
  
  int nextID;
  
  std::vector<Note> notes;
  std::vector<int> bars;
  std::vector<int> emptyRuns;
  
  int allocateNote(int id);
  int allocateNotePair(int start, int end, int pitch);
  
  void freeNote(int index);
  void freeNoteLine(int beginning);
  
  void sortNote(int index, bool spliceOut = true);
  
  int compareNotes(int index1, int index2);
  
  void setInstrument(Instrument* inst);
  
protected:
  
  virtual void updateClone();
  virtual void destroyOldClone(TrackData* newClone, TrackData* oldClone);
};

#endif // TRACK_H
