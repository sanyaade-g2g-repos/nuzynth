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

#ifndef __OSCILLATOR_PANEL_H__ 
#define __OSCILLATOR_PANEL_H__ 

#include <wx/wx.h>
#include "Instrument.h"
#include "OscillatorCanvas.h"
#include "ChangeOscillatorSlider.h"

class OscillatorPanel : public wxPanel {
public:
  OscillatorPanel(wxWindow *parent, wxWindowID id);
  
  void setInstrument(Instrument* inst);
  
private:
  Instrument       *inst;
  int               selectedVoice;
  wxSpinCtrl       *numerator;
  wxSpinCtrl       *denominator;
  wxBoxSizer       *voiceBox;
  wxChoice         *voiceChoice;
  wxSlider         *blurSlider;
  wxSlider         *stretchSlider;
  OscillatorCanvas *oscillatorCanvas;
  ChangeOscillatorSlider *sliderChange;
  bool              callbacksAssigned;
  
  void setVoice(int voice);
  void removeCalbacks();
  void stretchCallback(unsigned char*);
  void blurCallback(unsigned char*);
  void numeratorCallback(unsigned char*);
  void denominatorCallback(unsigned char*);
  void spinUpdated(wxCommandEvent& event);
  
  void OnChoiceUpdate( wxCommandEvent &event );
  void OnSliderMove(wxScrollEvent& event);
  void OnSliderRelease(wxScrollEvent& event);
  void OnSpinCtrl(wxSpinEvent& event);
  void OnSpinCtrlText(wxCommandEvent& event);
  
  // any class wishing to process wxWidgets events must use this macro
  DECLARE_EVENT_TABLE()
};

enum {
  VOICE_CHOICE = wxID_HIGHEST,
  OSC_SCROLL_BAR,
  NUMERATOR_SPINNER,
  DENOMINATOR_SPINNER,
  BLUR_SLIDER,
  STRETCH_SLIDER,
};

#endif // __OSCILLATOR_PANEL_H__ 
