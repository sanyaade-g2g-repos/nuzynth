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

#ifndef _SONG_CANVAS_H_ 
#define _SONG_CANVAS_H_ 

#include "wx/wx.h"
#if !wxUSE_GLCANVAS
  #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif
#include "wx/glcanvas.h"

#include "audioCallback.h"

class SongCanvas: public wxGLCanvas
{
public:
  SongCanvas( wxWindow *parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0, const wxString& name = _T("SongCanvas") );

  ~SongCanvas();
  
private:
  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnLeftMouseDown(wxMouseEvent& event);
  void OnLeftMouseUp(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  
  bool fill;
  
DECLARE_EVENT_TABLE()
};

#endif // _SONG_CANVAS_H_ 
