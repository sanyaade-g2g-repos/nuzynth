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

#ifndef CHANGE_AGGREGATE_H 
#define CHANGE_AGGREGATE_H 

#include "Change.hpp"

class ChangeAggregate {
public:
  ChangeAggregate();
  ~ChangeAggregate();
  void appendChange(Change* change);
protected:
  virtual void doForwards();
  virtual void doBackwards();
  std::vector<Change*> changes;
};

#endif // CHANGE_AGGREGATE_H 




package {
	public class AggregateChange extends Change {
		private const changes: Vector.<Change> = new Vector.<Change>();
		public function AggregateChange() {
			super(false);
			didAnything = false;
		}
		
		public final function append(change: Change): void {
			if (change.didNothing) return;
			changes[changes.length] = change;
			didAnything = true;
		}
		
		// WARNING: prepend is almost always a bad idea. Know what you're doing.
		protected final function prepend(change: Change): void {
			if (change.didNothing) return;
			changes.splice(0,0,change);
			didAnything = true;
		}
		
		protected override final function doForwards(): void {
			for (var i: int = 0; i < changes.length; i++) {
				changes[i].redo();
			}
		}
		
		protected override final function doBackwards(): void {
			for (var i: int = changes.length-1; i >= 0 ; i--) {
				changes[i].undo();
			}
		}
	}
}
