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
#include "ChangeEffectCreate.h"


ChangeEffectCreate::ChangeEffectCreate(Instrument* inst, char timeline, char type)
  : Change(type != -1)
{
  this->inst = inst;
  this->timeline = timeline;
  
  bool destroy;
  
  if (type == -1) {
    Effect* effect = inst->createEffect(timeline);
    type = effect->type;
    destroy = false;
  } else {
    destroy = true;
  }
  
  this->type = type;
  depth = inst->original->depths[timeline][type];
  if ((int)timeline != CONSTANT_TIMELINE) {
    buffer = (unsigned char*) malloc(sizeof(unsigned char) * EFFECT_LENGTH);
    memcpy(buffer, inst->original->buffers[timeline][type], EFFECT_LENGTH * sizeof(unsigned char));
  } else {
    buffer = 0;
  }
  
  noop = false;
  
  if (destroy) {
    doBackwards();
  }
}

ChangeEffectCreate::~ChangeEffectCreate() {
  if (buffer != 0) {
    free(buffer);
  }
}

void ChangeEffectCreate::doForwards() {
  inst->createEffect(timeline);
  unsigned char* newBuffer = (unsigned char*) Pool_draw(Instrument::effectPool());
  memcpy(newBuffer, buffer, EFFECT_LENGTH * sizeof(unsigned char));
  inst->replaceEffectBuffer(type, timeline, newBuffer);
  inst->setDepth(type, timeline, depth);
}

void ChangeEffectCreate::doBackwards() {
  Effect* effect;
  for (int i = 0; i < inst->timelines[timeline].size(); i++) {
    Effect* iter = inst->timelines[timeline][i];
    if (iter->type == type) {
      effect = iter;
      break;
    }
  }
  inst->destroyEffect(effect);
}
