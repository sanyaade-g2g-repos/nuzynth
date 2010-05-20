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

#include "OscillatorCanvas.hpp"
#include "Monitor.hpp"

BEGIN_EVENT_TABLE(OscillatorCanvas, wxWindow)
  EVT_PAINT(OscillatorCanvas::OnPaint)
  /*
  EVT_IDLE( OscillatorCanvas::OnIdle )
  EVT_SIZE(OscillatorCanvas::OnSize)
  EVT_ERASE_BACKGROUND(OscillatorCanvas::OnEraseBackground)
  EVT_KEY_DOWN( OscillatorCanvas::OnKeyDown )
  EVT_KEY_UP( OscillatorCanvas::OnKeyUp )
  */
  EVT_ENTER_WINDOW( OscillatorCanvas::OnMouseEnterWindow )
  EVT_LEAVE_WINDOW(OscillatorCanvas::OnMouseExitWindow)
  EVT_LEFT_DOWN(OscillatorCanvas::OnLeftMouseDown)
  EVT_LEFT_UP(OscillatorCanvas::OnLeftMouseUp)
  /*
  EVT_RIGHT_DOWN(OscillatorCanvas::OnRightMouseDown)
  EVT_RIGHT_UP(OscillatorCanvas::OnRightMouseUp)
  */
  EVT_MOTION(OscillatorCanvas::OnMouseMove)
  //EVT_MOUSEWHEEL(OscillatorCanvas::OnMouseWheel)
  //EVT_MOUSE_CAPTURE_LOST(OscillatorCanvas::OnMouseCaptureLost)
END_EVENT_TABLE()


OscillatorCanvas::OscillatorCanvas(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxWindow(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE , name )
{
  prevIndex = 0;
  inst = 0;
  voice = 0;
  drawChange = 0;
  selectedNoise = false;
  
  bottomOctave = log2(0.25f);
  topOctave = log2(64.0f);
  numOctaves = topOctave - bottomOctave;
  margin = 12;
  mouseInside = false;
}

OscillatorCanvas::~OscillatorCanvas() {}

void OscillatorCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) ) {
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
  
  // Regardless of update regions, update the lines only once, 'cus I'm too lazy to optimize this. :P
  vX = 0;
  vY = 0;
  GetClientSize(&vW, &vH);
  
  if (mouseInside) {
    float halfMargin = margin / 2;
    dc.SetBrush(wxBrush(wxColour(0xdd, 0xdd, 0xff)));
    dc.DrawRectangle(0, 0, halfMargin, vH);
    dc.DrawRectangle(vW-halfMargin, 0, halfMargin, vH);
    dc.DrawRectangle(0, 0, vW, halfMargin);
    dc.DrawRectangle(0, vH-halfMargin, vW, halfMargin);
  }
  
  if (inst == 0) return;
  
  float inner_width = vW - margin * 2 - 1;
  float x, y;
  dc.SetBrush(wxBrush(wxColour(), wxTRANSPARENT));
  
  dc.SetPen(wxPen(wxColour(0xD9,0xD9,0xD9), 1, wxSOLID));
  for (int i = 1; i < MAX_HARMONICS + 1; i++) {
    y = vH - (log2(i) - bottomOctave) * (1.0f / (numOctaves)) * vH;
    dc.DrawLine(margin, y, vW - margin, y);
  }
  
  if (selectedNoise) {
    dc.SetPen(wxPen(wxColour(0x00,0x00,0x00), 2, wxSOLID));
    
    unsigned char* spectrum = inst->oscillator.noise.logSpectrum;
    float x1, x2, y1, y2;
    x2 = margin + spectrum[0] * (1.0f/255.0f) * inner_width;
    y2 = vH * (1.0f - (-SUB_OCTAVES - bottomOctave) * (1.0f / (numOctaves)));
    
    for (int i = 2; i < NOISE_SPECTRUM_LENGTH; i++) {
      x1 = x2;
      y1 = y2;
      x2 = margin + spectrum[i] * (1.0f/255.0f) * inner_width;
      y2 = vH * (1.0f - (-SUB_OCTAVES - bottomOctave + 
                           (float) i / (float) NOISE_SAMPLES_PER_OCTAVE
                        ) * (1.0f / (numOctaves))
                );
      dc.DrawLine(x1, y1, x2, y2);
    }
  } else {
    
    dc.SetPen(wxPen(wxColour(0x00,0x00,0x00), 2, wxSOLID));
    
    HarmonicSet* set = &inst->oscillator.harmonicSet[voice];
    for (int i = 0; i < MAX_HARMONICS; i++) {
      if (set->amplitudes[i] == 0) continue;
      if (set->enabled[i] == false) continue;
      
      y = vH - (log2(set->freqs[i]) - bottomOctave) * (1.0f / (numOctaves)) * vH;
      x = margin + set->amplitudes[i] * (1.0f/255.0f) * inner_width;
      dc.DrawLine(margin, y, x, y);
      
      dc.DrawCircle(x, y, 3);
    }
  }
  
}

void OscillatorCanvas::OnSize(wxSizeEvent& event) {}





void OscillatorCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event)) {
  // Do nothing, to avoid flashing.
}
void OscillatorCanvas::OnIdle( wxIdleEvent& event ) {
    //Render();
    //event.RequestMore();
}
void OscillatorCanvas::OnKeyDown( wxKeyEvent& event ) {
  //long keyCode = event.GetKeyCode();
  event.Skip();
}
void OscillatorCanvas::OnKeyUp( wxKeyEvent& event ) {event.Skip();}
void OscillatorCanvas::OnMouseEnterWindow( wxMouseEvent& event ) {
  mouseInside = true;
  Refresh();
  event.Skip();
}
void OscillatorCanvas::OnMouseExitWindow(wxMouseEvent& event) {
  mouseInside = false;
  Refresh();
  event.Skip();
}

void OscillatorCanvas::OnLeftMouseDown(wxMouseEvent& event) {
  if (inst == 0) {
    event.Skip();
    return;
  }
  if (!HasCapture()) {
    CaptureMouse();
  }
  
  prevIndex = -1;
  prevOctave = -1;
  OnMouseMove(event);
  
  //Refresh(); // Rely on the callback for refresh. If that fails, something's wrong.
}
void OscillatorCanvas::OnLeftMouseUp(wxMouseEvent& event) {
  if (HasCapture()) {
    ReleaseMouse();
    if (drawChange != 0) {
      inst->song->history.record(drawChange);
      drawChange = 0;
    }
  }
  //event.Skip();
}
void OscillatorCanvas::OnRightMouseDown(wxMouseEvent& event) {event.Skip();}
void OscillatorCanvas::OnRightMouseUp(wxMouseEvent& event) {event.Skip();}

void OscillatorCanvas::OnMouseMove(wxMouseEvent& event) {
  if (inst == 0) {
    event.Skip();
    return;
  }
  if (!event.m_leftDown || !HasCapture()) {
    event.Skip();
    return;
  }
  int w, h;
  GetClientSize(&w, &h);
  
  float inner_width = w - margin * 2 - 1;
  float value = ((float) event.m_x - margin) / inner_width;
  if (value > 1.0f) value = 1.0f;
  if (value < 0.0f) value = 0.0f;
  float octave = (1.0f - (float) event.m_y / (float) h) * numOctaves + bottomOctave;
  
  if (selectedNoise) {
    int nextIndex = (octave + (float) SUB_OCTAVES) * NOISE_SAMPLES_PER_OCTAVE;
    if (nextIndex < 0) nextIndex = 0;
    if (nextIndex >= NOISE_SPECTRUM_LENGTH) nextIndex = NOISE_SPECTRUM_LENGTH - 1;
    
    if (drawChange == 0) {
      drawChange = new ChangeOscillatorDraw(inst, inst->oscillator.noise.logSpectrum, NOISE_SPECTRUM_LENGTH, true);
    }
    drawChange->update(prevIndex, nextIndex, value * 255.0f);
    prevIndex = nextIndex;
  } else {
    int nextIndex = 0;
    float closestDistance = std::numeric_limits<float>::max();
    
    for (int i = 0; i < MAX_HARMONICS; i++) {
      float distance = log2(inst->oscillator.harmonicSet[voice].freqs[i]) - octave;
      if (distance < 0) distance = -distance;
      if (distance < closestDistance) {
        nextIndex = i;
        closestDistance = distance;
      }
    }
    
    if (drawChange == 0) {
      drawChange = new ChangeOscillatorDraw(inst, inst->oscillator.harmonicSet[voice].amplitudes, MAX_HARMONICS, false);
    }
    drawChange->update(prevIndex, nextIndex, value * 255.0f);
    prevIndex = nextIndex;
  }
  
  SharedManagerBase::share();
  
  //Refresh(); // Rely on the callback for refresh. If that fails, something's wrong.
}

void OscillatorCanvas::OnMouseWheel(wxMouseEvent& event) {event.Skip();}

/*void OscillatorCanvas::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
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

void OscillatorCanvas::setInstrument(Instrument* inst) {
  detachOldCallback();
  this->inst = inst;
  if (inst != 0) {
    Monitor::addCallback( &inst->original->wave, 
                          new Callback<float*, OscillatorCanvas>
                            (this, &OscillatorCanvas::waveUpdated) );
  }
  Refresh();
}

void OscillatorCanvas::detachOldCallback() {
  if (inst != 0) {
    Monitor::removeCallback( (void*)(&inst->original->wave), this);
  }
  inst = 0;
}

void OscillatorCanvas::waveUpdated(float** buffer) {
  Refresh();
}


void OscillatorCanvas::setVoice(int voice) {
  this->voice = voice;
  selectedNoise = (voice == NUM_VOICES);
  Refresh();
}
