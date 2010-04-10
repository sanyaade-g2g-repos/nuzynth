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
#include "EffectPanel.h"
#include "Monitor.h"
#include "ChangeEffectCreate.h"

BEGIN_EVENT_TABLE(TimelinePanel, wxPanel)
  EVT_COMMAND_SCROLL_THUMBTRACK (SPEED_SLIDER, TimelinePanel::OnSliderUpdate)
  EVT_COMMAND_SCROLL_THUMBRELEASE (SPEED_SLIDER, TimelinePanel::OnSliderFinish)
  EVT_BUTTON    (ADD_BUTTON, TimelinePanel::OnAddPressed)
END_EVENT_TABLE()

TimelinePanel::TimelinePanel(wxScrolledWindow* scrollMe, int timeline, bool isConstant, 
                             wxString timelineTitle, wxString addLabel,
                             wxWindow *parent, wxWindowID id)
             : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize)
{
  this->timeline = timeline;
  this->isConstant = isConstant;
  this->scrollMe = scrollMe;
  inst = 0;
  speedChange = 0;
  PushEventHandler(new WheelCatcher(scrollMe));
  
  wxStaticText *text;
  wxStaticBox *staticBox = new wxStaticBox (this, wxID_ANY, timelineTitle);
  topBox = new wxStaticBoxSizer (staticBox, wxVERTICAL );
  staticBox->PushEventHandler(new WheelCatcher(scrollMe));
  SetSizer( topBox );
  SetAutoLayout( true );
  
  if (isConstant == false) {
    speedBox = new wxBoxSizer(wxHORIZONTAL);
    topBox->Add(speedBox, wxSizerFlags(0).Expand().Border(wxBOTTOM, 5));
    
    text = new wxStaticText( this, wxID_ANY, _T("Slow "), wxDefaultPosition, wxDefaultSize);
    speedBox->Add(text, wxSizerFlags(0));
    text->PushEventHandler(new WheelCatcher(scrollMe));
    
    speedSlider = new wxSlider( this, SPEED_SLIDER, 0, 0, 254,
                                wxDefaultPosition, wxSize(125,wxDefaultCoord), wxSL_AUTOTICKS);
    speedBox->Add(speedSlider, wxSizerFlags(1));
    speedSlider->PushEventHandler(new WheelCatcher(scrollMe));
    
    text = new wxStaticText( this, wxID_ANY, _T(" Fast"), wxDefaultPosition, wxDefaultSize);
    speedBox->Add(text, wxSizerFlags(0));
    text->PushEventHandler(new WheelCatcher(scrollMe));
    
    speedBox->Show(false);
  } else {
    speedBox = 0;
    speedSlider = 0;
  }
  
  effectBox = new wxBoxSizer(wxVERTICAL);
  topBox->Add(effectBox, wxSizerFlags(0).Expand());
  
  addButton = new wxButton(this, ADD_BUTTON, addLabel, wxDefaultPosition, wxDefaultSize);
  topBox->Add(addButton, wxSizerFlags(0).Center());
  addButton->PushEventHandler(new WheelCatcher(scrollMe));
}

void TimelinePanel::OnAddPressed( wxCommandEvent &event ) {
  if (inst == 0) return;
  inst->song->history.record(new ChangeEffectCreate(inst, timeline));
}

void TimelinePanel::OnSliderUpdate( wxScrollEvent &event ) {
  //int val = m_slider->GetValue();
  int val = event.GetSelection();
  switch (event.GetId()) {
    case SPEED_SLIDER:
      printf("SPEED_SLIDER\n");
      if (speedChange != 0) {
        speedChange->undo();
        delete speedChange;
        speedChange = 0;
      }
      speedChange = new ChangeTimelineSpeed(inst, timeline, val);
      break;
  }
  SharedManagerBase::share();
  printf("slider updated, index: %d\n", val);
}

void TimelinePanel::OnSliderFinish( wxScrollEvent& event ) {
  switch (event.GetId()) {
    case SPEED_SLIDER:
      inst->song->history.record(speedChange);
      speedChange = 0;
      break;
  }
}





void TimelinePanel::setInstrument(Instrument* inst) {
  printf("TimelinePanel::setInstrument from %s to %s\n", this->inst == 0 ? "null" : "valid", inst == 0 ? "null" : "valid");
  if (this->inst != 0) {
    while (children.size() > 0) {
      destroyChild(children.front());
    }
    Monitor::removeCallback(&this->inst->timelineEffectCount[timeline], this);
    
    if (speedBox != 0) { /// TODO: && numchilden != 0 ???
      Monitor::removeCallback( (void*)(&this->inst->sharedData->speeds[timeline]), this );
    }
  }
  
  this->inst = inst;
  
  if (this->inst != 0) {
    std::vector<Effect*> vec = this->inst->timelines[timeline];
    std::vector<Effect*>::iterator iter = vec.begin();
    for (; iter != vec.end(); iter++) {
      addChild(*iter);
    }
    Monitor::addCallback( &this->inst->timelineEffectCount[timeline], 
                          new Callback<int, TimelinePanel>
                            (this, &TimelinePanel::onEffectCountChanged) );
    
    addButton->Enable(true);
    if (speedBox != 0) { /// TODO: && numchilden != 0 ???
      speedBox->Show(children.size() != 0);
      speedSlider->SetValue(this->inst->getSpeed(timeline));
      
      Monitor::addCallback( &inst->sharedData->speeds[timeline], 
                            new Callback<unsigned char, TimelinePanel>
                              (this, &TimelinePanel::speedChangedCallback) );
    }
  } else {
    if (speedBox != 0) speedBox->Show(false);
    addButton->Enable(false);
  }
  effectBox->Layout();
  topBox->Layout();
}

void TimelinePanel::onEffectCountChanged( int* val ) {
  while (inst->timelineEffectCount[timeline] > children.size()) {
    addChild(inst->timelines[timeline][children.size()]);
  }
  printf("count changed, count: %d, children: %d\n", inst->timelineEffectCount[timeline], children.size());
  while (inst->timelineEffectCount[timeline] < children.size()) {
    printf("children: %d\n", children.size());
    for (int i = 0; i < children.size(); i++) {
      if (i < inst->timelineEffectCount[timeline]) {
        printf("%d, %d, %d, %s\n", 
          i, 
          inst->timelines[timeline][i]->type, 
          children[i]->effect->type, 
          (children[i]->effect == inst->timelines[timeline][i]) ?"same":"different");
      }
      if (i >= inst->timelineEffectCount[timeline] || 
          children[i]->effect != inst->timelines[timeline][i])
      {
        printf("destroying %d\n", i);
        destroyChild(children[i]);
        break;
      }
    }
  }
}

void TimelinePanel::addChild(Effect* effect) {
  EffectPanel* child = new EffectPanel(scrollMe, effect, isConstant, this, wxID_ANY);
  effectBox->Add(child, wxSizerFlags(0).Expand().Center());
  children.push_back(child);
  
  if (speedBox != 0) speedBox->Show(true);
  if (children.size() >= NUM_EFFECT_TYPES) addButton->Show(false);
  GetContainingSizer()->SetVirtualSizeHints(GetParent());
}

void TimelinePanel::speedChangedCallback(unsigned char* val) {
  speedSlider->SetValue(inst->getSpeed(timeline));
}

void TimelinePanel::destroyChild(EffectPanel* child) {
  std::vector<EffectPanel*>::iterator iter = children.begin();
  for (; iter != children.end(); iter++) {
    if (*iter == child) {
      children.erase(iter);
      break;
    }
  }
  child->Destroy();
  
  if (children.size() <= 0 && speedBox != 0) speedBox->Show(false);
  addButton->Show(true);
  GetContainingSizer()->SetVirtualSizeHints(GetParent());
}
