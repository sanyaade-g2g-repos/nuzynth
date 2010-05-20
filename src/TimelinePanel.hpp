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

#ifndef TIMELINE_PANEL_H
#define TIMELINE_PANEL_H

#include <stdlib.h>
#include <wx/wx.h>
#include "WheelCatcher.hpp"
#include "Instrument.hpp"
#include "ChangeTimelineSpeed.hpp"

class EffectPanel;

// Define a new frame type: this is going to be our main frame
class TimelinePanel : public wxPanel {
public:
  TimelinePanel(wxScrolledWindow* scrollMe, int timeline, bool isConstant, 
                wxString timelineTitle, wxString addLabel,
                wxWindow *parent, wxWindowID id);
  
  void setInstrument(Instrument* inst);
  
private:
  int timeline;
  bool isConstant;
  Instrument* inst;
  wxScrolledWindow* scrollMe;
  
  void onEffectCountChanged( int* val );
  void addChild(Effect* effect);
  void destroyChild(EffectPanel* child);
  void OnAddPressed( wxCommandEvent &event );
  void OnSliderUpdate( wxScrollEvent &event );
  void OnSliderFinish( wxScrollEvent &event );
  void speedChangedCallback(unsigned char* val);
  
  wxButton         *addButton;
  wxSlider         *speedSlider;
  wxBoxSizer       *speedBox;
  wxBoxSizer       *effectBox;
  wxStaticBoxSizer *topBox;
  std::vector<EffectPanel*> children;
  ChangeTimelineSpeed *speedChange;
  
  // any class wishing to process wxWidgets events must use this macro
  DECLARE_EVENT_TABLE()
};

enum {
  ADD_BUTTON = wxID_HIGHEST,
  SPEED_SLIDER,
};

#endif // TIMELINE_PANEL_H
