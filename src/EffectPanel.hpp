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

#ifndef EFFECT_PANEL_H
#define EFFECT_PANEL_H

#include <wx/wx.h>
#include <wx/choice.h>
#include "EffectCanvas.hpp"
#include "Instrument.hpp"
#include "TimelinePanel.hpp"
#include "ChangeEffectCreate.hpp"
#include "ChangeSwitchEffect.hpp"
#include "ChangeEffectDepth.hpp"

// Define a new frame type: this is going to be our main frame
class EffectPanel : public wxPanel {
public:
  EffectPanel(wxScrolledWindow* scrollMe, Effect* effect, bool isConstant, TimelinePanel *parent, wxWindowID id);
  ~EffectPanel();
  Effect* effect;
  
private:
  
  void OnSliderUpdate( wxScrollEvent &event );
  void OnSliderFinish( wxScrollEvent &event );
  
  void OnChoiceUpdate( wxCommandEvent &event );
  void typeChangedCallback(char* effect);
  void depthChangedCallback(unsigned char* val);
  bool isFinite();
  
  //void selectModifier(int modifier);
  
  char               oldType;
  TimelinePanel*     timelineParent;
  wxScrolledWindow*  scrollMe;
  bool               isConstant;
  ChangeEffectDepth *depthChange;
  wxChoice          *effectTypeChoice;
  wxSlider          *depthSlider;
  EffectCanvas      *effectCanvas;
  wxBoxSizer        *sliderBox;
  
  // any class wishing to process wxWidgets events must use this macro
  DECLARE_EVENT_TABLE()
};


enum {
  EFFECT_TYPE_CHOICE = wxID_HIGHEST,
  EFFECT_CANVAS,
  DEPTH_SLIDER,
};

#endif // EFFECT_PANEL_H
