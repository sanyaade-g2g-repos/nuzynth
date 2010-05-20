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
#include "ChangeEffectDraw.h"


ChangeEffectDraw::ChangeEffectDraw(Instrument* inst, char timeline, char type)
  : Change(false)
{
  this->inst = inst;
  this->timeline = timeline;
  this->type = type;
  this->before = (unsigned char*) malloc(sizeof(unsigned char) * EFFECT_LENGTH);
  this->after = (unsigned char*) malloc(sizeof(unsigned char) * EFFECT_LENGTH);
  
  lowestIndex = EFFECT_LENGTH;
  highestIndex = -1;
}

ChangeEffectDraw::~ChangeEffectDraw() {
  free(before);
  free(after);
}

void ChangeEffectDraw::update(int prevIndex, int nextIndex, float value) {
  unsigned char* buffer = inst->copyEffectBuffer(type, timeline);
  
  int bottomIndex, topIndex;
  float bottomValue, topValue, delta;
  if (prevIndex == -1) {
    bottomIndex = topIndex = nextIndex;
    bottomValue = topValue = value;
    delta = 0;
  } else if (prevIndex < nextIndex) {
    bottomIndex = prevIndex;
    topIndex = nextIndex;
    bottomValue = buffer[bottomIndex];
    topValue = value;
  } else {
    bottomIndex = nextIndex;
    topIndex = prevIndex;
    bottomValue = value;
    topValue = buffer[topIndex];
  }
  delta = (topValue - bottomValue) / ((float) topIndex - (float) bottomIndex);
  for (int i = bottomIndex; i <= topIndex; i++) {
    if (i < lowestIndex || i > highestIndex) before[i] = buffer[i];
    buffer[i] = bottomValue;
    after[i] = bottomValue;
    bottomValue += delta;
  }
  
  inst->replaceEffectBuffer(type, timeline, buffer);
  
  if (bottomIndex < lowestIndex) lowestIndex = bottomIndex;
  if (topIndex > highestIndex) highestIndex = topIndex;
  
  noop = false;
}

void ChangeEffectDraw::doForwards() {
  unsigned char* buffer = inst->copyEffectBuffer(type, timeline);
  memcpy(&buffer[lowestIndex], &after[lowestIndex], (highestIndex - lowestIndex + 1) * sizeof(unsigned char));
  inst->replaceEffectBuffer(type, timeline, buffer);
}

void ChangeEffectDraw::doBackwards() {
  unsigned char* buffer = inst->copyEffectBuffer(type, timeline);
  memcpy(&buffer[lowestIndex], &before[lowestIndex], (highestIndex - lowestIndex + 1) * sizeof(unsigned char));
  inst->replaceEffectBuffer(type, timeline, buffer);
}
