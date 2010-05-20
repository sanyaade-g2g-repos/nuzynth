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

#include "History.hpp"

History::History() {
  index = 0;
}

History::~History() {
  for (int i = 0; i < changes.size(); i++) {
    delete changes[i];
  }
}

void History::record(Change* change) {
  if (change->isNoOp()) {
    delete change;
    return;
  }
  if (index < changes.size()) {
    for (int i = index; i < changes.size(); i++) {
      delete changes[i];
    }
    changes.resize(index);
  }
  changes.push_back(change);
  index++;
}

void History::undo() {
  if (index <= 0) return;
  index--;
  changes[index]->undo();
}

void History::redo() {
  if (index >= changes.size()) return;
  changes[index]->redo();
  index++;
}
