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
#include "ChangeSwitchEffect.h"


ChangeSwitchEffect::ChangeSwitchEffect(Instrument* inst, char timeline, char before, char after)
  : Change(false)
{
  this->inst = inst;
  this->timeline = timeline;
  this->before = before;
  this->after = after;
  didAnything = true;
  doForwards();
}

ChangeSwitchEffect::~ChangeSwitchEffect() {}

void ChangeSwitchEffect::doForwards() {
  inst->switchEffectType(timeline, before, after);
}

void ChangeSwitchEffect::doBackwards() {
  inst->switchEffectType(timeline, after, before);
}
