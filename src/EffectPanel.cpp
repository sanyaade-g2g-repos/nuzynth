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

#include "EffectPanel.hpp"
#include "Monitor.hpp"
#include "SharedManagerBase.hpp"

BEGIN_EVENT_TABLE(EffectPanel, wxPanel)
  EVT_COMMAND_SCROLL_THUMBTRACK (DEPTH_SLIDER, EffectPanel::OnSliderUpdate)
  EVT_COMMAND_SCROLL_THUMBRELEASE (DEPTH_SLIDER, EffectPanel::OnSliderFinish)
  EVT_CHOICE    (EFFECT_TYPE_CHOICE, EffectPanel::OnChoiceUpdate)
END_EVENT_TABLE()

EffectPanel::EffectPanel(wxScrolledWindow* scrollMe, Effect* effect, bool isConstant, TimelinePanel *parent, wxWindowID id)
              : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize) {
  timelineParent = parent;
  this->isConstant = isConstant;
  this->effect = effect;
  this->scrollMe = scrollMe;
  depthChange = 0;
  PushEventHandler(new WheelCatcher(scrollMe));
  
  wxBoxSizer *vBox;
  wxStaticText *text;
  vBox = new wxBoxSizer(wxVERTICAL);
  SetSizer( vBox );
  SetAutoLayout( true );
  
  effectTypeChoice = new wxChoice(this, EFFECT_TYPE_CHOICE, wxDefaultPosition, wxDefaultSize, NUM_EFFECT_TYPES + 1, effectChoices);
  effectTypeChoice->PushEventHandler(new WheelCatcher(scrollMe));
  
  vBox->Add(effectTypeChoice, wxSizerFlags(0).Center().Expand());
  effectTypeChoice->Select(effect->type);
  
  sliderBox = new wxBoxSizer(wxHORIZONTAL);
  vBox->Add(sliderBox, wxSizerFlags(0).Expand().Border(wxBOTTOM, 2));
  
  if (isConstant) {
    if (isFinite()) {
      text = new wxStaticText( this, wxID_ANY, _T("Offset: "), wxDefaultPosition, wxDefaultSize);
    } else {
      text = new wxStaticText( this, wxID_ANY, _T("Range: "), wxDefaultPosition, wxDefaultSize);
    }
  } else {
    text = new wxStaticText( this, wxID_ANY, _T("Range: "), wxDefaultPosition, wxDefaultSize);
  }
  text->PushEventHandler(new WheelCatcher(scrollMe));
  sliderBox->Add(text, wxSizerFlags(0));
  
  depthSlider = new wxSlider( this, DEPTH_SLIDER, 0, 0, 254,
                              wxDefaultPosition, wxSize(125,wxDefaultCoord), wxSL_AUTOTICKS);
  sliderBox->Add(depthSlider, wxSizerFlags(1));
  depthSlider->PushEventHandler(new WheelCatcher(scrollMe));
  depthSlider->SetValue(effect->inst->getDepth(effect->type, effect->timeline));
  if (isConstant == false) {
    if (!isFinite()) {
      sliderBox->Show(false);
    }
    
    effectCanvas = new EffectCanvas(effect, this, EFFECT_CANVAS, wxDefaultPosition, wxSize(192,80));
    vBox->Add(effectCanvas, wxSizerFlags(0).Border(wxBOTTOM, 10));
    effectCanvas->PushEventHandler(new WheelCatcher(scrollMe));
  } else {
    effectCanvas = 0;
  }
  
  vBox->Layout();
  
  
  
  Monitor::addCallback( &effect->type, 
                        new Callback<char, EffectPanel>
                          (this, &EffectPanel::typeChangedCallback) );
  oldType = effect->type;
  Monitor::addCallback( &effect->inst->original->depths[effect->timeline][effect->type], 
                        new Callback<unsigned char, EffectPanel>
                          (this, &EffectPanel::depthChangedCallback) );
}

EffectPanel::~EffectPanel() {
  Monitor::removeCallback( (void*)(&effect->type), this );
  Monitor::removeCallback( (void*)(&effect->inst->original->depths[effect->timeline][effect->type]), this );
  if (effectCanvas != 0) effectCanvas->setEffect(0); // detach callbacks
}

void EffectPanel::OnChoiceUpdate( wxCommandEvent &event ) {
  //if (m_inst == 0) return;
  int val = event.GetSelection();
  Instrument* inst = effect->inst;
  switch (event.GetId()) {
    case EFFECT_TYPE_CHOICE:
      if (val != NUM_EFFECT_TYPES) {
        inst->song->history.record(new ChangeSwitchEffect(inst, effect->timeline, effect->type, val));
      } else {
        // Destroy!
        if (effect != 0) {
          inst->song->history.record(new ChangeEffectCreate(inst, effect->timeline, effect->type));
          // DO NOT ACCESS MEMBER VARIABLES AFTER DESTRUCTION!!!
        }
        return;
      }
      break;
  }
}

void EffectPanel::OnSliderUpdate( wxScrollEvent& event ) {
  int val = event.GetSelection();
  switch (event.GetId()) {
    case DEPTH_SLIDER:
      if (depthChange != 0) {
        depthChange->undo();
        delete depthChange;
        depthChange = 0;
      }
      depthChange = new ChangeEffectDepth(effect->inst, effect->timeline, effect->type, val);
      break;
  }
  SharedManagerBase::share();
}

void EffectPanel::OnSliderFinish( wxScrollEvent& event ) {
  switch (event.GetId()) {
    case DEPTH_SLIDER:
      effect->inst->song->history.record(depthChange);
      depthChange = 0;
      break;
  }
}

void EffectPanel::typeChangedCallback(char* type) {
  
  Monitor::removeCallback( (void*)(&effect->inst->original->depths[effect->timeline][oldType]), this );
  Monitor::addCallback( &effect->inst->original->depths[effect->timeline][effect->type], 
                        new Callback<unsigned char, EffectPanel>
                          (this, &EffectPanel::depthChangedCallback) );
  oldType = effect->type;
  effectTypeChoice->Select(effect->type);
  if (isConstant == false) {
    if (isFinite()) {
      sliderBox->Show(true);
    } else {
      sliderBox->Show(false);
      depthSlider->SetValue(effect->inst->getDepth(effect->type, effect->timeline));
    }
  }
  wxSizer* parentSizer = GetParent()->GetContainingSizer();
  wxWindow* parentWindow = GetParent()->GetParent();
  parentWindow->SetVirtualSizeHints(0,0);
  parentSizer->SetVirtualSizeHints(parentWindow);
}

void EffectPanel::depthChangedCallback(unsigned char* val) {
  depthSlider->SetValue(effect->inst->getDepth(effect->type, effect->timeline));
}

bool EffectPanel::isFinite() {
  if (!effectScaleType[effect->type]) {
    return false;
  } else {
    return true;
  }
}
