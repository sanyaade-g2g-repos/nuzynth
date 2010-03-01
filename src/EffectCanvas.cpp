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

#include "EffectCanvas.h"
#include "Monitor.h"

BEGIN_EVENT_TABLE(EffectCanvas, wxWindow)
  EVT_PAINT(EffectCanvas::OnPaint)
  /*
  EVT_IDLE( EffectCanvas::OnIdle )
  EVT_KEY_DOWN( EffectCanvas::OnKeyDown )
  EVT_KEY_UP( EffectCanvas::OnKeyUp )
  */
  EVT_ENTER_WINDOW( EffectCanvas::OnMouseEnterWindow )
  EVT_LEAVE_WINDOW(EffectCanvas::OnMouseExitWindow)
  EVT_LEFT_DOWN(EffectCanvas::OnLeftMouseDown)
  EVT_LEFT_UP(EffectCanvas::OnLeftMouseUp)
  /*
  EVT_RIGHT_DOWN(EffectCanvas::OnRightMouseDown)
  EVT_RIGHT_UP(EffectCanvas::OnRightMouseUp)
  */
  EVT_MOTION(EffectCanvas::OnMouseMove)
  //EVT_MOUSEWHEEL(EffectCanvas::OnMouseWheel)
  //EVT_MOUSE_CAPTURE_LOST(EffectCanvas::OnMouseCaptureLost)
END_EVENT_TABLE()


EffectCanvas::EffectCanvas(Effect* effect,
                           wxWindow *parent, wxWindowID id,
                           const wxPoint& pos, const wxSize& size, long style, const wxString& name)
            : wxWindow(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
{
  prevIndex = 0;
  this->effect = 0;
  change = 0;
  effectType = -1;
  setEffect(effect);
  mouseInside = false;
  margin = 2;
}

EffectCanvas::~EffectCanvas() {
  //detachOldCallback();
}

void EffectCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) ) {
  wxPaintDC dc(this);
  
  int vX, vY, vW, vH;                 // Dimensions of client area in pixels
  wxRegionIterator upd(GetUpdateRegion()); // get the update rect list
  
  while (upd)
  {
    vX = upd.GetX();
    vY = upd.GetY();
    vW = upd.GetW();
    vH = upd.GetH();
    
    dc.SetPen(wxPen(wxColour(), 1, wxTRANSPARENT));
    dc.SetBrush(wxBrush(wxColour(0xff,0xff,0xff)));
    dc.DrawRectangle(vX, vY, vW, vH);
    
    upd ++ ;
  }
  
  // Regardless of update regions, update the curve only once, 'cus I'm too lazy to optimize this. :P
  vX = 0;
  vY = 0;
  GetClientSize(&vW, &vH);
  float inner_height = vH - margin * 2 - 1;
  
  if (mouseInside) {
    dc.SetBrush(wxBrush(wxColour(0xdd, 0xdd, 0xff)));
    dc.DrawRectangle(0, 0, margin*2, vH);
    dc.DrawRectangle(vW-margin*2, 0, margin*2, vH);
    dc.DrawRectangle(0, 0, vW, margin*2);
    dc.DrawRectangle(0, vH-margin*2, vW, margin*2);
  }
  
  dc.SetPen(wxPen(wxColour(0x00,0x00,0x00), 2, wxSOLID));
  
  unsigned char* buffer = effect->inst->mod.buffers[effect->timeline][effect->type];
  float x1, x2, y1, y2;
  float dx = (float) vW / (float) EFFECT_LENGTH;
  x2 = (float) dx / 2.0f;
  y2 = margin + (1.0f - (float) buffer[0] * (1.0f / 254.0f)) * inner_height;
  for (int i = 1; i < EFFECT_LENGTH; i++) {
    x1 = x2;
    y1 = y2;
    x2 += dx;
    y2 = margin + (1.0f - (float) buffer[i] * (1.0f / 254.0f)) * inner_height;
    dc.DrawLine(x1, y1, x2, y2);
  }
}

void EffectCanvas::OnIdle( wxIdleEvent& event ) {
    //Render();
    //event.RequestMore();
}
void EffectCanvas::OnKeyDown( wxKeyEvent& event ) {
  //long keyCode = event.GetKeyCode();
  event.Skip();
}
void EffectCanvas::OnKeyUp( wxKeyEvent& event ) {event.Skip();}

void EffectCanvas::OnMouseEnterWindow( wxMouseEvent& event ) {
  mouseInside = true;
  Refresh();
  event.Skip();
}
void EffectCanvas::OnMouseExitWindow(wxMouseEvent& event) {
  mouseInside = false;
  Refresh();
  event.Skip();
}


void EffectCanvas::getIndexValue(int& index, float& value, wxMouseEvent& event) {
  int w, h;
  GetClientSize(&w, &h);
  index = (int) ((float) EFFECT_LENGTH * (float) event.m_x / (float) w);
  value = 1.0f - (float) (event.m_y - margin) / (float) (h - margin * 2);
  
  if (index < 0) index = 0;
  else if (index >= EFFECT_LENGTH) index = EFFECT_LENGTH-1;
  if (value < 0.0f) value = 0.0f;
  else if (value > 1.0f) value = 1.0f;
}

void EffectCanvas::OnLeftMouseDown(wxMouseEvent& event) {
  int index;
  float value;
  getIndexValue(index, value, event);
  prevIndex = index;
  
  change = new ChangeEffectDraw(effect->inst, effect->timeline, effect->type);
  change->update(prevIndex, index, value * 254.0f);
  
  Refresh();
  
  if (!HasCapture()) {
    CaptureMouse();
  }
  //event.Skip();
}
void EffectCanvas::OnLeftMouseUp(wxMouseEvent& event) {
  if (HasCapture()) {
    effect->inst->song->history.record(change);
    change = 0;
    ReleaseMouse();
  }
  //event.Skip();
}
void EffectCanvas::OnRightMouseDown(wxMouseEvent& event) {event.Skip();}
void EffectCanvas::OnRightMouseUp(wxMouseEvent& event) {event.Skip();}

void EffectCanvas::OnMouseMove(wxMouseEvent& event) {
  if (!event.m_leftDown || !HasCapture()) {
    event.Skip();
    return;
  }
  int index;
  float value;
  getIndexValue(index, value, event);
  
  change->update(prevIndex, index, value * 254.0f);
  
  prevIndex = index;
  
  Refresh();
  //event.Skip();
}

void EffectCanvas::OnMouseWheel(wxMouseEvent& event) {
  event.Skip();
  //printf("mousewheel\n");
  //GetParent()->GetParent()->GetEventHandler()->ProcessEvent( event );
}

/*void EffectCanvas::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
  mouseCaptured = false;
}*/

/*
event.m_altDown
event.m_controlDown
event.m_leftDown
event.m_middleDown
event.m_rightDown
event.m_metaDown
event.m_shiftDown
event.m_x
event.m_y
event.m_wheelRotation
event.m_wheelDelta
event.m_linesPerAction
event.LeftDClick() // returns true if was a double click event
event.MiddleDClick() // returns true if was a double click event
event.RightDClick() // returns true if was a double click event
*/

void EffectCanvas::setEffect(Effect* effect) {
  detachEffectBufferCallback();
  detachEffectTypeCallback();
  
  this->effect = effect;
  
  if (effect != 0) {
    Monitor::addCallback( &effect->type, 
                          new Callback<char, EffectCanvas>
                            (this, &EffectCanvas::onEffectTypeChanged) );
    onEffectTypeChanged((char*) 0);
  }
  Refresh();
}

void EffectCanvas::detachEffectTypeCallback() {
  if (effect != 0) {
    Monitor::removeCallback( (void*)(&effect->type), this );
  }
  effect = 0;
}

void EffectCanvas::onEffectTypeChanged(char* unused) {
  detachEffectBufferCallback();
  
  effectType = effect->type;
  
  if (effectType != -1) {
    Monitor::addCallback( &effect->inst->mod.buffers[effect->timeline][effectType], 
                          new Callback<unsigned char*, EffectCanvas>
                            (this, &EffectCanvas::onBufferChanged) );
  }
  Refresh();
}

void EffectCanvas::detachEffectBufferCallback() {
  if (effectType != -1) {
    Monitor::removeCallback( (void*)(&effect->inst->mod.buffers[effect->timeline][effectType]), 
                             this );
  }
  effectType = -1;
}

void EffectCanvas::onBufferChanged(unsigned char** buffer) {
  Refresh();
}
