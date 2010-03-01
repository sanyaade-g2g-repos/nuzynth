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
#include "ChangeOscillatorSlider.h"

ChangeOscillatorSlider::ChangeOscillatorSlider(Instrument* inst, unsigned char* address, unsigned char newValue) 
  : Change(false)
{
  this->inst = inst;
  this->address = address;
  before = *address;
  after = newValue;
  doForwards();
}

ChangeOscillatorSlider::~ChangeOscillatorSlider() {}

void ChangeOscillatorSlider::update(unsigned char newValue) {
  after = newValue;
  doForwards();
}

void ChangeOscillatorSlider::doForwards() {
  if (*address == after) return;
  Monitor::setProperty(address, after);
  inst->oscillator.dirty = true;
  inst->markDirty();
  didAnything = true;
}

void ChangeOscillatorSlider::doBackwards() {
  if (*address == before) return;
  Monitor::setProperty(address, before);
  inst->oscillator.dirty = true;
  inst->markDirty();
}
