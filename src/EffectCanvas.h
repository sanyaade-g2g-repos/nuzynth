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

#ifndef _EFFECT_CANVAS_H_ 
#define _EFFECT_CANVAS_H_ 

#include <wx/wx.h>

#include "Instrument.h"

class EffectCanvas: public wxWindow
{
public:
  EffectCanvas( Effect* effect,
                wxWindow *parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0, const wxString& name = _T("EffectCanvas") );

  ~EffectCanvas();
  
  void setEffect(Effect* effect);
  
private:
  void OnPaint(wxPaintEvent& event);
  void OnIdle( wxIdleEvent& event );
  void OnKeyDown(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);
  void OnMouseEnterWindow(wxMouseEvent& event);
  void OnMouseExitWindow(wxMouseEvent& event);
  void OnLeftMouseDown(wxMouseEvent& event);
  void OnLeftMouseUp(wxMouseEvent& event);
  void OnRightMouseDown(wxMouseEvent& event);
  void OnRightMouseUp(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnMouseWheel(wxMouseEvent& event);
  void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
  
  Effect* effect;
  int prevIndex;
  int margin;
  bool mouseInside;
  void bufferUpdated(unsigned char** buffer);
  void detachOldCallback();
  void getIndexValue(int& index, float& value, wxMouseEvent& event);
  
DECLARE_EVENT_TABLE()
};

#endif // _EFFECT_CANVAS_H_ 
