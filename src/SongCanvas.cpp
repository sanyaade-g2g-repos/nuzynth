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

#include <stdio.h>
#include <math.h>
#include <limits>

#include "SongCanvas.h"

BEGIN_EVENT_TABLE(SongCanvas, wxGLCanvas)
  EVT_SIZE(SongCanvas::OnSize)
  EVT_PAINT(SongCanvas::OnPaint)
  EVT_ERASE_BACKGROUND(SongCanvas::OnEraseBackground)
  EVT_LEFT_DOWN(SongCanvas::OnLeftMouseDown)
  EVT_LEFT_UP(SongCanvas::OnLeftMouseUp)
  EVT_MOTION(SongCanvas::OnMouseMove)
END_EVENT_TABLE()


SongCanvas::SongCanvas(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
{
  wxSizeEvent blah;
  OnSize(blah);
  return;
}

SongCanvas::~SongCanvas() {}

void SongCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) ) {
  //printf("render start\n");
  wxPaintDC dc(this);

#ifndef __WXMOTIF__
  if (!GetContext()) return;
#endif

  SetCurrent();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  // clear color and depth buffers 
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  
  // Enable the vertex array and specify the data.
  glEnableClientState( GL_VERTEX_ARRAY );
  GLfloat data[8];
  glVertexPointer( 2, GL_FLOAT, 0, data );
  
  /*
  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      if (tonematrix[x][y]) {
        glColor3f(1.0f,1.0f,1.0f);
      } else {
        glColor3f(0.25f,0.25f,0.25f);
      }
      
      float left = (x) / 16.0f + 0.01;
      float right = (x+1) / 16.0f - 0.01;
      float top = (y) / 16.0f + 0.01;
      float bottom = (y+1) / 16.0f - 0.01;
      
      data[0] = left;
      data[1] = top;
      data[2] = right;
      data[3] = top;
      data[4] = right;
      data[5] = bottom;
      data[6] = left;
      data[7] = bottom;
      
      glDrawArrays(GL_QUADS, 0, 8);
    }
  }
  */
  
  glFlush();
  SwapBuffers();
  //printf("render end\n");
}

void SongCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);

    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    int w, h;
    GetClientSize(&w, &h);
#ifndef __WXMOTIF__
    if (GetContext())
#endif
    {
        SetCurrent();
        glViewport(0, 0, (GLint) w, (GLint) h);
    }
    Refresh();
}
void SongCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event)) {
  // Do nothing, to avoid flashing.
}

void SongCanvas::OnLeftMouseDown(wxMouseEvent& event) {
  if (!HasCapture()) {
    CaptureMouse();
  }
  
  /*
  int w, h;
  GetClientSize(&w, &h);
  
  float x1 = (float) event.m_x / (float) w;
  float y1 = (float) event.m_y / (float) h;
  int x = x1 * 16;
  int y = 15 - (int)(y1 * 16);
  if (x < 0) x = 0;
  if (x > 15) x = 15;
  if (y < 0) y = 0;
  if (y > 15) y = 15;
  
  fill = !tonematrix[x][y];
  
  OnMouseMove(event);
  */
  
  //Refresh();
  //event.Skip();
}
void SongCanvas::OnLeftMouseUp(wxMouseEvent& event) {
  if (HasCapture()) {
    ReleaseMouse();
  }
  //event.Skip();
}
void SongCanvas::OnMouseMove(wxMouseEvent& event) {
  if (!event.m_leftDown || !HasCapture()) {
    event.Skip();
    return;
  }
  
  /*
  int w, h;
  GetClientSize(&w, &h);
  
  float x1 = (float) event.m_x / (float) w;
  float y1 = (float) event.m_y / (float) h;
  int x = x1 * 16;
  int y = 15 - (int)(y1 * 16);
  if (x < 0) x = 0;
  if (x > 15) x = 15;
  if (y < 0) y = 0;
  if (y > 15) y = 15;
  
  tonematrix[x][y] = fill;
  
  Refresh();
  */
  
  //event.Skip();
}
