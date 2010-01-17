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

#include <memory.h>
#include "Monitor.h"
#include "ChangeOscillatorDraw.h"


ChangeOscillatorDraw::ChangeOscillatorDraw(Instrument* inst, unsigned char* address, int size, bool noise)
  : Change(false)
{
  this->inst = inst;
  this->address = address;
  this->size = size;
  this->noise = noise;
  /// TODO: make a destructor and free this:
  this->buffer = (unsigned char*) malloc(sizeof(unsigned char) * size);
  
  lowestIndex = size;
  highestIndex = -1;
}

void ChangeOscillatorDraw::update(int prevIndex, int nextIndex, float value) {
  int bottomIndex, topIndex;
  float bottomValue, topValue, delta;
  if (prevIndex == -1) {
    bottomIndex = topIndex = nextIndex;
    bottomValue = topValue = value;
    delta = 0;
  } else if (prevIndex < nextIndex) {
    bottomIndex = prevIndex;
    topIndex = nextIndex;
    bottomValue = address[bottomIndex];
    topValue = value;
  } else {
    bottomIndex = nextIndex;
    topIndex = prevIndex;
    bottomValue = value;
    topValue = address[topIndex];
  }
  delta = (topValue - bottomValue) / ((float) topIndex - (float) bottomIndex);
  for (int i = bottomIndex; i <= topIndex; i++) {
    if (i < lowestIndex || i > highestIndex) buffer[i] = address[i];
    address[i] = bottomValue;
    bottomValue += delta;
  }
  
  if (noise) {
    NoiseSpectrum_updateRange(&inst->oscillator.noise, 
                              (float) (bottomIndex-1) / (float) NOISE_SAMPLES_PER_OCTAVE - SUB_OCTAVES,
                              (float) (topIndex + 1)  / (float) NOISE_SAMPLES_PER_OCTAVE - SUB_OCTAVES);
  }
  inst->oscillator.dirty = true;
  inst->markDirty();
  
  if (bottomIndex < lowestIndex) lowestIndex = bottomIndex;
  if (topIndex > highestIndex) highestIndex = topIndex;
  
  didAnything = true;
}

void ChangeOscillatorDraw::swap() {
  for (int i = lowestIndex; i <= highestIndex; i++) {
    unsigned char temp = address[i];
    address[i] = buffer[i];
    buffer[i] = temp;
  }
  if (noise) {
    NoiseSpectrum_updateRange(&inst->oscillator.noise, 
                              (float) (lowestIndex - 1) / (float) NOISE_SAMPLES_PER_OCTAVE - SUB_OCTAVES,
                              (float) (highestIndex +1) / (float) NOISE_SAMPLES_PER_OCTAVE - SUB_OCTAVES);
  }
  inst->oscillator.dirty = true;
  inst->markDirty();
}

void ChangeOscillatorDraw::doForwards() {
  swap();
}

void ChangeOscillatorDraw::doBackwards() {
  swap();
}
