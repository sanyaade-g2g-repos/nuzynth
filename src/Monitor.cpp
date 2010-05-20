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

#include <stdlib.h>
#include "Monitor.hpp"

std::map<void*, Monitor*> Monitor::monitors;

void Monitor::addCallbackInternal(void* prop, CallbackBase* callback) {
  Monitor* monitor;
  if (monitors.find(prop) != monitors.end()) {
    monitor = monitors[prop];
  } else {
    monitor = new Monitor();
    monitors[prop] = monitor;
    //monitor->dirty = false;
  }
  // TODO: check for duplicate.
  monitor->callbacks.push_front(callback);
}
  
void Monitor::removeCallback(void* prop, void* thisptr) {
  Monitor* monitor;
  if (monitors.find(prop) != monitors.end()) {
    monitor = monitors[prop];
  } else {
    printf("Monitor::removeCallback() - Attempting to remove nonexistent callback.\n");
  }
  std::list<CallbackBase*>::iterator iter = monitor->callbacks.begin();
  while (iter != monitor->callbacks.end()) {
    CallbackBase* callback2 = *iter;
    //if (callback->getThisPtr() == callback2->getThisPtr() && callback->getFuncPtr() == callback2->getFuncPtr()) {
    if (callback2->getThisPtr() == thisptr) {
      delete *iter;
      monitor->callbacks.erase(iter++);
    } else {
      iter++;
    }
  }
  // TODO: consider deleting monitor if empty... repeated new() and delete() too expensive? nah, go for it.
}
  
void Monitor::setPropertyInternal(void* prop) {
  Monitor* monitor;
  if (monitors.find(prop) != monitors.end()) {
    monitor = monitors[prop];
    std::list<CallbackBase*>::iterator iter;
    for (iter = monitor->callbacks.begin(); iter != monitor->callbacks.end(); iter++) {
      CallbackBase* callback = *iter;
      (*callback)( (void*) prop );
    }
  }
}
