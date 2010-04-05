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

#include "MyFrame.h"
#include "Monitor.h"
#include "OscillatorPanel.h"
#include "WheelCatcher.h"

BEGIN_EVENT_TABLE(OscillatorPanel, wxPanel)
  EVT_CHOICE         (VOICE_CHOICE,   OscillatorPanel::OnChoiceUpdate)
  EVT_SPINCTRL       (wxID_ANY,       OscillatorPanel::OnSpinCtrl)
  EVT_TEXT           (wxID_ANY,       OscillatorPanel::OnSpinCtrlText)
  EVT_COMMAND_SCROLL_THUMBTRACK (wxID_ANY, OscillatorPanel::OnSliderMove)
  EVT_COMMAND_SCROLL_THUMBRELEASE (wxID_ANY, OscillatorPanel::OnSliderRelease)
END_EVENT_TABLE()

OscillatorPanel::OscillatorPanel(wxWindow *parent, wxWindowID id)
             : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize)
{
  selectedVoice = 0;
  sliderChange = 0;
  callbacksAssigned = false;
  
  wxStaticBoxSizer *panelSizer = new wxStaticBoxSizer (new wxStaticBox (this, wxID_ANY, _T("Oscillator")), wxVERTICAL );
  SetSizer( panelSizer );
  SetAutoLayout( true );
  
  wxStaticText *text;
  wxBoxSizer *vBox = new wxBoxSizer(wxVERTICAL);
  panelSizer->Add(vBox);
  
  voiceChoice = new wxChoice(this, VOICE_CHOICE, wxDefaultPosition, wxDefaultSize, NUM_VOICES + 1, voiceChoices);
  vBox->Add(voiceChoice, wxSizerFlags(0).Center().Expand());
  voiceChoice->Select(0);
  
  oscillatorCanvas = new OscillatorCanvas(this, wxID_ANY, wxDefaultPosition, wxSize(200, 550));
  vBox->Add(oscillatorCanvas);
  
  voiceBox = new wxBoxSizer(wxVERTICAL);
  vBox->Add(voiceBox, wxSizerFlags(0).Expand());
  wxBoxSizer *hBox = new wxBoxSizer(wxHORIZONTAL);
  voiceBox->Add(hBox, wxSizerFlags(0).Expand());
  
  text = new wxStaticText( this, wxID_ANY, _T("Frequency: "), wxDefaultPosition, wxDefaultSize);
  hBox->Add(text, wxSizerFlags(0).Center());
  
  numerator = new wxSpinCtrl( this, NUMERATOR_SPINNER, wxEmptyString, wxDefaultPosition, wxSize(60, wxDefaultCoord) );
  hBox->Add(numerator);
  
  text = new wxStaticText( this, wxID_ANY, _T("/"), wxDefaultPosition, wxDefaultSize);
  hBox->Add(text, wxSizerFlags(0).Center());
  
  denominator = new wxSpinCtrl( this, DENOMINATOR_SPINNER, wxEmptyString, wxDefaultPosition, wxSize(60, wxDefaultCoord) );
  hBox->Add(denominator);
  
  numerator->SetRange(1, MAX_HARMONICS);
  denominator->SetRange(1, MAX_HARMONICS);
  
  
  hBox = new wxBoxSizer(wxHORIZONTAL);
  voiceBox->Add(hBox, wxSizerFlags(0).Expand());
  text = new wxStaticText( this, wxID_ANY, _T("Blur:"), wxDefaultPosition, wxDefaultSize);
  hBox->Add(text);
  blurSlider = new wxSlider( this, BLUR_SLIDER, 0, 0, 254,
                              wxDefaultPosition, wxSize(125,wxDefaultCoord), wxSL_AUTOTICKS);
  hBox->Add(blurSlider, wxSizerFlags(1));
  
  hBox = new wxBoxSizer(wxHORIZONTAL);
  voiceBox->Add(hBox, wxSizerFlags(0).Expand());
  text = new wxStaticText( this, wxID_ANY, _T("Stretch:"), wxDefaultPosition, wxDefaultSize);
  hBox->Add(text);
  stretchSlider = new wxSlider( this, STRETCH_SLIDER, 0, 0, 254,
                              wxDefaultPosition, wxSize(125,wxDefaultCoord), wxSL_AUTOTICKS);
  hBox->Add(stretchSlider, wxSizerFlags(1));
  
  
  inst = 0;
  setInstrument(0);
  
  //PushEventHandler(new WheelCatcher(scrollMe));
  panelSizer->Layout();
}

void OscillatorPanel::setInstrument(Instrument* inst) {
  if (this->inst != 0) {
    removeCalbacks();
  }
  
  this->inst = inst;
  oscillatorCanvas->setInstrument(inst);
  
  if (this->inst != 0) {
    voiceChoice->Enable(true);
    setVoice(selectedVoice);
  } else {
    voiceChoice->Enable(false);
    voiceBox->Show(false);
  }
}

void OscillatorPanel::setVoice(int voice) {
  removeCalbacks();
  selectedVoice = voice;
  if (voice >= NUM_VOICES) {
    voiceBox->Show(false);
  } else {
    voiceBox->Show(true);
    HarmonicSet* set = &inst->oscillator.harmonicSet[selectedVoice];
    blurSlider->SetValue(set->blur);
    stretchSlider->SetValue(set->stretch);
    numerator->SetValue(set->numerator);
    denominator->SetValue(set->denominator);
    
    if (callbacksAssigned == false) {
      Monitor::addCallback( &set->stretch, 
                            new Callback<unsigned char, OscillatorPanel>
                              (this, &OscillatorPanel::stretchCallback) );
      Monitor::addCallback( &set->blur, 
                            new Callback<unsigned char, OscillatorPanel>
                              (this, &OscillatorPanel::blurCallback) );
      Monitor::addCallback( &set->numerator, 
                            new Callback<unsigned char, OscillatorPanel>
                              (this, &OscillatorPanel::numeratorCallback) );
      Monitor::addCallback( &set->denominator, 
                            new Callback<unsigned char, OscillatorPanel>
                              (this, &OscillatorPanel::denominatorCallback) );
      callbacksAssigned = true;
    }
  }
  oscillatorCanvas->setVoice(voice);
}

void OscillatorPanel::removeCalbacks() {
  if (callbacksAssigned == false) return;
  HarmonicSet* set = &inst->oscillator.harmonicSet[selectedVoice];
  Monitor::removeCallback( (void*)(&set->stretch), this );
  Monitor::removeCallback( (void*)(&set->blur), this );
  Monitor::removeCallback( (void*)(&set->numerator), this );
  Monitor::removeCallback( (void*)(&set->denominator), this );
  callbacksAssigned = false;
}

void OscillatorPanel::stretchCallback(unsigned char*) {
  HarmonicSet* set = &inst->oscillator.harmonicSet[selectedVoice];
  stretchSlider->SetValue(set->stretch);
}

void OscillatorPanel::blurCallback(unsigned char*) {
  HarmonicSet* set = &inst->oscillator.harmonicSet[selectedVoice];
  blurSlider->SetValue(set->blur);
}

void OscillatorPanel::numeratorCallback(unsigned char*) {
  HarmonicSet* set = &inst->oscillator.harmonicSet[selectedVoice];
  numerator->SetValue(set->numerator);
}

void OscillatorPanel::denominatorCallback(unsigned char*) {
  HarmonicSet* set = &inst->oscillator.harmonicSet[selectedVoice];
  denominator->SetValue(set->denominator);
}

void OscillatorPanel::OnChoiceUpdate( wxCommandEvent &event ) {
  int val = event.GetSelection();
  switch (event.GetId()) {
    case VOICE_CHOICE:
      setVoice(val);
      break;
  }
}

void OscillatorPanel::OnSpinCtrl(wxSpinEvent& event) {
  spinUpdated(event);
}

void OscillatorPanel::OnSpinCtrlText(wxCommandEvent& event) {
  spinUpdated(event);
}

void OscillatorPanel::spinUpdated(wxCommandEvent& event) {
  if (inst == 0) return;
  
  HarmonicSet* set = &inst->oscillator.harmonicSet[selectedVoice];
  
  ChangeOscillatorSlider *change = 0;
  
  switch (event.GetId()) {
    case NUMERATOR_SPINNER:
      if (numerator->GetValue() == set->numerator) return;
      change = new ChangeOscillatorSlider(inst, &set->numerator, numerator->GetValue());
      break;
    case DENOMINATOR_SPINNER:
      if (denominator->GetValue() == set->denominator) return;
      change = new ChangeOscillatorSlider(inst, &set->denominator, denominator->GetValue());
      break;
  }
  
  if (change != 0) {
    inst->song->history.record(change);
  }
  
  // Do whatever you can to remove keyboard focus from the spinners!
  oscillatorCanvas->SetFocus();
}

void OscillatorPanel::OnSliderMove(wxScrollEvent& event) {
  int val = event.GetSelection();
  HarmonicSet* set = &inst->oscillator.harmonicSet[selectedVoice];
  if (sliderChange == 0) {
    switch (event.GetId()) {
      case BLUR_SLIDER:
        sliderChange = new ChangeOscillatorSlider(inst, &set->blur, val);
        break;
      case STRETCH_SLIDER:
        sliderChange = new ChangeOscillatorSlider(inst, &set->stretch, val);
        break;
    }
  } else {
    sliderChange->update(val);
  }
  SharedManagerBase::updateClones();
}
void OscillatorPanel::OnSliderRelease(wxScrollEvent& event) {
  if (sliderChange != 0) {
    inst->song->history.record(sliderChange);
    sliderChange = 0;
  }
}
