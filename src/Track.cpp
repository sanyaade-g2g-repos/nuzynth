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

#include "Track.hpp"
#include "Instrument.hpp"

Track::Track(Instrument* instrument) {
  original = (TrackData*) Pool_draw(clonePool());
  memset(original, 0, sizeof(TrackData));
  
  setInstrument(instrument);
  
  notes.resize(16);
  memset (&notes[0], 0, notes.size() * (sizeof(Note)));
  original->beatsPerBar = 4;
  original->head = 1;
  original->tail = 2;
  emptyRuns.push_back(3);
  nextID = 1;
  
  notes[original->head].prev = 0;
  notes[original->head].next = original->tail;
  notes[original->head].id = INT_MAX;
  notes[original->head].beat = INT_MIN;
  notes[original->head].on = true;
  notes[original->head].type = NOTE_DUMMY;
  
  notes[original->tail].prev = original->head;
  notes[original->tail].next = 0;
  notes[original->tail].id = INT_MAX;
  notes[original->tail].beat = INT_MAX;
  notes[original->tail].on = true;
  notes[original->tail].type = NOTE_DUMMY;
  
  /*
  allocateNotePair(0, 5, 56);
  allocateNotePair(8, 13, 58);
  allocateNotePair(6, 7, 61);
  allocateNotePair(14, 15, 63);
  */
  
  allocateNotePair(0, 13, 56);
  allocateNotePair(3, 14, 58);
  allocateNotePair(6, 15, 61);
  allocateNotePair(9, 16, 63);
  
  for (int i = 0; i < 12; i++) {
    printf("(NOTE) index: %d, neighbors: (%d, %d), joined: (%d, %d), type: %d, id: %d, beat: %d, pitch: %d, %s\n",
           i, 
           notes[i].prev, 
           notes[i].next, 
           notes[i].prevJoined, 
           notes[i].nextJoined, 
           notes[i].type, 
           notes[i].id, 
           notes[i].beat, 
           notes[i].pitch, 
           notes[i].on ? "on" : "off");
  }
  
  for (int i = 0; i < emptyRuns.size(); i++) {
    printf("EMPTY: %d\n", emptyRuns[i]);
  }
  
  printf("notecount %d\n", notes.size());
  
  update();
}

Track::~Track() {
  Pool_return(clonePool(), original);
}

int Track::allocateNote(int id) {
  markDirty();
  
  int index;
  int emptySize = emptyRuns.size();
  if (emptySize > 0) {
    index = emptyRuns[emptySize - 1];
    int nextIndex = index + 1;
    if (nextIndex < notes.size() && notes[nextIndex].on == false) {
      emptyRuns[emptySize - 1] = nextIndex;
    } else {
      emptyRuns.pop_back();
    }
  } else {
    int oldSize = notes.size();
    notes.resize(oldSize * 2);
    memset (&notes[oldSize], 0, oldSize * (sizeof(Note)));
    emptyRuns.push_back(oldSize+1);
    
    index = oldSize;
  }
  
  notes[index].on = true;
  notes[index].id = id;
  
  return index;
}

void Track::freeNote(int index) {
  markDirty();
  
  // Close the gap:
  notes[notes[index].next].prev = notes[index].prev;
  notes[notes[index].prev].next = notes[index].next;
  
  
  memset (&notes[index], 0, sizeof(Note));
  
  
  bool prevOn = (index == 0 || notes[index-1].on);
  bool nextOn = (index == notes.size()-1 || notes[index+1].on);
  
  if (prevOn && nextOn) {
    // we're creating a new empty run.
    emptyRuns.push_back(index);
  } else if (prevOn) {
    // we're at the start of an empty run. 
    for (int i = 0; i < emptyRuns.size(); i++) {
      if (emptyRuns[i] == index + 1) {
        emptyRuns[i] = index;
        break;
      }
    }
  } else if (nextOn) {
    // we're at the end of an empty run, we don't have to do anything.
  } else {
    // we're joining a couple empty runs. remove the head of the second run.
    std::vector<int>::iterator iter = emptyRuns.begin();
    for (; iter != emptyRuns.end(); iter++) {
      if (*iter == index + 1) {
        emptyRuns.erase(iter);
        return;
      }
    }
  }
  
}

int Track::allocateNotePair(int start, int end, int pitch) {
  markDirty();
  
  int id = nextID;
  nextID++;
  int first = allocateNote(id);
  int second = allocateNote(id);
  
  /*
  notes[second].prev = first;
  notes[second].next = notes[head].next;
  notes[notes[second].next].prev = second;
  notes[first].prev = head;
  notes[first].next = second;
  notes[head].next = first;
  */
  
  notes[first].nextJoined = second;
  notes[first].beat = start;
  notes[first].pitch = pitch;
  notes[first].type = NOTE_START;
  
  notes[second].prevJoined = first;
  notes[second].beat = end;
  notes[second].pitch = pitch;
  notes[second].type = NOTE_END;
  
  sortNote(second, false);
  sortNote(first, false);
  
  return first;
}

void Track::freeNoteLine(int beginning) {
  int iter = beginning;
  while (true) {
    if (notes[iter].type & NOTE_END) {
      freeNote(iter);
      break;
    } else {
      int next = notes[iter].nextJoined;
      freeNote(iter);
      iter = next;
    }
  }
}

/// TODO: Pick nearest note from bar array before sorting. 
// if Notes have a valid position in the linked list they need to be spliced out. 
void Track::sortNote(int index, bool spliceOut) {
  markDirty();
  
  int iter;
  
  if (spliceOut) {
    notes[notes[index].prev].next = notes[index].next;
    notes[notes[index].next].prev = notes[index].prev;
    iter = notes[index].prev;
  } else {
    iter = original->head;
  }
  
  if (compareNotes(index, iter) < 0) {
    do {
      iter = notes[iter].next;
    } while (compareNotes(index, iter) < 0);
    
    notes[index].prev = notes[iter].prev;
    notes[notes[index].prev].next = index;
    notes[index].next = iter;
    notes[iter].prev = index;
    
  } else if (compareNotes(index, iter) > 0) {
    do {
      iter = notes[iter].prev;
    } while (compareNotes(index, iter) > 0);
    
    notes[index].next = notes[iter].next;
    notes[notes[index].next].prev = index;
    notes[index].prev = iter;
    notes[iter].next = index;
  }
}

// Returns  1 if index2 comes after index1. 
// Returns -1 if index2 comes first. 
// Returns  0 if they're equivalent
int Track::compareNotes(int index1, int index2) {
  if (notes[index1].beat < notes[index2].beat) {
    return 1;
  } else if (notes[index1].beat > notes[index2].beat) {
    return -1;
  } else {
    if (notes[index1].type & NOTE_START) {
      if (notes[index2].type & NOTE_START) {
        return 0;
      } else {
        return -1;
      }
    } else if (notes[index1].type & NOTE_END) {
      if (notes[index2].type & NOTE_END) {
        return 0;
      } else {
        return 1;
      }
    } else {
      if (notes[index2].type & NOTE_START) {
        return 1;
      } else if (notes[index2].type & NOTE_END) {
        return -1;
      } else {
        return 0;
      }
    }
  }
}

void Track::setInstrument(Instrument* inst) {
  markDirty();
  original->instrument = inst;
}

void Track::updateClone() {
  //beatsPerBarSharer.write();
  
  // allocate new buffers!
  original->notes = (Note*) malloc(sizeof(Note) * notes.size());
  memcpy(original->notes, &notes[0], sizeof(Note) * notes.size());
  original->bars = (int*) malloc(sizeof(int) * bars.size());
  memcpy(original->bars, &bars[0], sizeof(int) * bars.size());
  original->noteCount = notes.size();
  original->barCount = bars.size();
}

void Track::destroyOldClone(TrackData* newClone, TrackData* oldClone) {
  free(oldClone->notes);
  free(oldClone->bars);
}
