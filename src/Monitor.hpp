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

#ifndef MONITOR_H 
#define MONITOR_H 

#include "Callback.hpp"
#include <map>
#include <list>

class Monitor {
private:
  static std::map<void*, Monitor*> monitors;
  std::list<CallbackBase*> callbacks;
  
  //static std::list<Monitor*> dirtyList;
  //bool dirty;
  
  static void addCallbackInternal(void* prop, CallbackBase* callback);
  static void setPropertyInternal(void* prop);
  
public:
  
  template<class Value, class Listener>
  static void addCallback(Value* prop, Callback<Value, Listener>* callback) {
    addCallbackInternal((void*) prop, callback);
  }
  
  static void removeCallback(void* prop, void* thisptr);
  
  template <class Value>
  static void setProperty(Value* prop, const Value& val) {
    *prop = val;
    setPropertyInternal( (void*)prop );
  }
};

#endif // MONITOR_H 
