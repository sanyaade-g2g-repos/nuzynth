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

#ifndef CALLBACK_H 
#define CALLBACK_H 

class CallbackBase {
public:
  virtual void operator()(void* prop) = 0;
  virtual void* getThisPtr() = 0;
  //virtual void* getFuncPtr() = 0;
};

template <class Value, class Listener> class Callback : public CallbackBase {
public:
  Listener* _thisptr;
  void (Listener::*_funcptr) (Value*);
  Callback(Listener* thisptr, void (Listener::*funcptr) (Value*)) : _thisptr(thisptr), _funcptr(funcptr) {}
  void operator()(void* prop) { (_thisptr->*_funcptr)( (Value*) prop ); }
  virtual void* getThisPtr() { return (void*) _thisptr; }
  //virtual void* getFuncPtr() { return reinterpret_cast<void*>(_funcptr); }
};


#endif // CALLBACK_H 
