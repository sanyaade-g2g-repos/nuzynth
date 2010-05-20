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

#include "MyFrame.hpp"
#include "Modulator.h"
#include "Monitor.hpp"
#include "audiocallback.hpp"
#include "midi.hpp"
#include "Tone.h"
#include "WheelCatcher.hpp"
#include "Clip.hpp"
#include "Track.hpp"




class blah : public wxPanel {
public:
  blah(wxWindow *parent, wxWindowID id);
};
blah::blah(wxWindow *parent, wxWindowID id)
             : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize){}






// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU  (wxID_EXIT, MyFrame::OnQuit)
  EVT_MENU (wxID_ABOUT, MyFrame::OnAbout)
  EVT_MENU  (wxID_OPEN, MyFrame::OnOpen)
  EVT_MENU (wxID_SAVEAS, MyFrame::OnSave)
  EVT_MENU (FILE_EXPORT, MyFrame::OnExport)
  EVT_MENU (wxID_UNDO, MyFrame::OnUndo)
  EVT_MENU (wxID_REDO, MyFrame::OnRedo)
  EVT_BUTTON    (RECORD_BUTTON, MyFrame::OnRecordPressed)
  EVT_BUTTON    (STOP_BUTTON, MyFrame::OnStopPressed)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(830, 720))
{
  // set the frame icon
  //SetIcon(wxICON(sample));
  
  #if wxUSE_MENUS
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(wxID_EXIT, _T("&Quit\tCTRL-Q"), _T("Quit this program"));
    fileMenu->Append(wxID_OPEN, _T("&Open Instrument\tCTRL-O"), _T("Open an Instrument"));
    fileMenu->Append(wxID_SAVEAS, _T("&Save Instrument\tCTRL-S"), _T("Save an Instrument"));
    fileMenu->Append(FILE_EXPORT, _T("&Export to WAV\tCTRL-E"), _T("Export a WAV sample of a tone playing with the current instrument."));
    
    wxMenu *editMenu = new wxMenu;
    editMenu->Append(wxID_UNDO, _T("&Undo\tCTRL-Z"), _T("Undo previous action"));
    editMenu->Append(wxID_REDO, _T("&Redo\tCTRL-SHIFT-Z"), _T("Redo next action"));
    
    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, _T("&About...\tF1"), _T("Show about dialog"));
    
    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(editMenu, _T("&Edit"));
    menuBar->Append(helpMenu, _T("&Help"));
    
    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
  #endif // wxUSE_MENUS
  
  
  m_inst = 0;
  
  
  wxBoxSizer *frameSizer = new wxBoxSizer(wxHORIZONTAL);
  SetSizer( frameSizer );
  SetAutoLayout( true );
  
  wxPanel* panel;
  wxStaticBoxSizer *panelSizer;
  wxBoxSizer *hBox, *vBox;
  
  
  oscillatorPanel = new OscillatorPanel(this, wxID_ANY);
  frameSizer->Add(oscillatorPanel, wxSizerFlags(0).Border().Expand());
  
  panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  frameSizer->Add(panel, wxSizerFlags(0).Border().Expand()); // flags: proportion=1, several pixel border on all sides, autoexpand to fill space available
  panelSizer = new wxStaticBoxSizer (new wxStaticBox (panel, wxID_ANY, _T("Modulators")), wxVERTICAL );
  panel->SetSizer( panelSizer );
  panel->SetAutoLayout( true );
  panel->SetMinSize(wxSize(260,100));
  
  wxScrolledWindow* scrolled = new wxScrolledWindow(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxTAB_TRAVERSAL | wxFULL_REPAINT_ON_RESIZE | wxALWAYS_SHOW_SB);
  panelSizer->Add(scrolled, wxSizerFlags(1).Expand());
  vBox = new wxBoxSizer(wxVERTICAL);
  scrolled->SetSizer( vBox );
  scrolled->SetAutoLayout( true );
  scrolled->SetScrollRate(0,5);
  
  timelinePanels[0] = new TimelinePanel(scrolled, ATTACK_TIMELINE, false, _T("Startup"), _T("Add Startup Effect"), scrolled, wxID_ANY), wxSizerFlags(0).Expand();
  timelinePanels[1] = new TimelinePanel(scrolled, RELEASE_TIMELINE, false, _T("Release"), _T("Add Release Effect"), scrolled, wxID_ANY), wxSizerFlags(0).Expand();
  timelinePanels[2] = new TimelinePanel(scrolled, LOOP_1_TIMELINE, false, _T("Loop 1"), _T("Add Loop 1 Effect"), scrolled, wxID_ANY), wxSizerFlags(0).Expand();
  timelinePanels[3] = new TimelinePanel(scrolled, LOOP_2_TIMELINE, false, _T("Loop 2"), _T("Add Loop 2 Effect"), scrolled, wxID_ANY), wxSizerFlags(0).Expand();
  timelinePanels[4] = new TimelinePanel(scrolled, CONSTANT_TIMELINE, true, _T("Constant"), _T("Add Constant Effect"), scrolled, wxID_ANY), wxSizerFlags(0).Expand();
  
  for (int i = 0; i < NUM_TIMELINES; i++) {
    vBox->Add(timelinePanels[i], wxSizerFlags(0).Expand().Border(wxRIGHT|wxBOTTOM,10));
  }
  
  vBox->AddSpacer(150);
  
  vBox->SetVirtualSizeHints(scrolled); // needed to update parent size?
  vBox->Layout();
  panelSizer->Layout();
  
  
  vBox = new wxBoxSizer(wxVERTICAL);
  frameSizer->Add(vBox);
  
  panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  vBox->Add(panel); // flags: proportion=1, several pixel border on all sides, autoexpand to fill space available
  //panelSizer = new wxStaticBoxSizer (new wxStaticBox (panel, wxID_ANY, _T("Placeholder Tone Matrix")), wxHORIZONTAL );
  //panel->SetSizer( panelSizer );
  panel->SetAutoLayout( true );
  /*panelSizer->Add(*/ new SongCanvas(panel, wxID_ANY, wxDefaultPosition, wxSize(250,250)) ;//, wxSizerFlags(0).Center());
  //panelSizer->Layout();
  
  hBox = new wxBoxSizer(wxHORIZONTAL);
  vBox->Add(hBox);
  recordButton = new wxButton(this, RECORD_BUTTON, _T("Record"), wxDefaultPosition, wxDefaultSize);
  hBox->Add(recordButton);
  stopButton = new wxButton(this, STOP_BUTTON, _T("Stop"), wxDefaultPosition, wxDefaultSize);
  hBox->Add(stopButton);
  stopButton->Enable(false);
  
  
  // For some reason the GLCanvases don't get positioned correctly unless I do this:
  frameSizer->Layout();
  
  
}


// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
                    _T("Welcome to %s!\n")
                    _T("\n")
                    _T("This is the minimal wxWidgets sample\n")
                    _T("running under %s."),
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _T("About wxWidgets minimal sample"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::OnOpen(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, _T("Choose an instrument"),
                     wxEmptyString, wxEmptyString,
                     _T("XML files (*.xml)|*.xml"), wxFD_OPEN);
    if ( dlg.ShowModal() == wxID_OK ) {
      const char* filepath = dlg.GetPath().mb_str(wxConvUTF8);
      FILE* file = fopen(filepath, "r");
      Instrument* inst = new Instrument(_song, file);
      fclose(file);
      Instrument* temp = _song->clips[0]->tracks[0]->original->instrument;
      
      _song->clips[0]->tracks[0]->setInstrument(inst);
      
      setInstrument(inst);
      temp->prepareToDie();
    }
}

void MyFrame::OnSave(wxCommandEvent& WXUNUSED(event)) {
    wxFileDialog dlg(this, _T("Save an instrument"),
                     wxEmptyString, _T("instrument.xml"),
                     _T("XML file (*.xml)|*.xml"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if ( dlg.ShowModal() == wxID_OK ) {
      const char* filepath = dlg.GetPath().mb_str(wxConvUTF8);
      FILE* file = fopen(filepath, "w");
      m_inst->save(file);
      fclose(file);
    }
}
/*
void printLittleEndian(int val, int size, FILE* file) {
  unsigned char bytes[4];
  for (int i = 0; i < size; i++) {
    bytes[i] = (val & (0xff << (i * 8))) >> (i * 8);
  }
  fwrite(bytes, 1, size, file);
}
*/
void MyFrame::OnExport(wxCommandEvent& WXUNUSED(event)) {
  /*
  wxFileDialog dlg(this, _T("Save a sample"),
                   wxEmptyString, _T("sample.wav"),
                   _T("WAV file (*.wav)|*.wav"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
  if ( dlg.ShowModal() == wxID_OK ) {
    const char* filepath = dlg.GetPath().mb_str(wxConvUTF8);
    FILE* file = fopen(filepath, "w");
    
    
    Tone tone;
    Tone_create(&tone, 60, &m_inst->sharer, midi_frequencyFromPitchIndex(60));
    
    float buffer[256 * 2];
    std::vector<float> sample(0);
    while (tone.alive) {
      memset (buffer, 0, 256 * 2 * (sizeof(float)));
      m_inst->tran.loopFunction(&tone, &m_inst->tran, buffer, 256);
      int oldSize = sample.size();
      sample.resize(oldSize + 256 * 2);
      memcpy(&sample[oldSize], buffer, sizeof(float) * 256 * 2);
      
      if (oldSize > 2048) tone.released = 1;
    }
    
    int rawChannelCount = 2;
    int wavChannelCount = 1;
    int sampleRate = 44100;
    int bytesPerSample = 2;
    int bitsPerSample = 8*bytesPerSample;
    int sampleCount = wavChannelCount * (sample.size() / rawChannelCount);
    
    fprintf(file, "RIFF");
    printLittleEndian(36 + sampleCount * bytesPerSample, 4, file);
    fprintf(file, "WAVEfmt ");
    printLittleEndian(0x00000010, 4, file);
    printLittleEndian(0x0001, 2, file);
    printLittleEndian(wavChannelCount, 2, file); // channels
    printLittleEndian(sampleRate, 4, file); // sample rate
    printLittleEndian(sampleRate * bytesPerSample * wavChannelCount, 4, file); // bytes per second
    printLittleEndian(bytesPerSample, 2, file); // sample rate
    printLittleEndian(bitsPerSample, 2, file); // sample rate
    fprintf(file, "data");
    printLittleEndian(sampleCount * bytesPerSample, 4, file);
    
    int stride;
    int repeat;
    if (rawChannelCount == wavChannelCount) {
      stride = 1;
      repeat = 1;
    } else {
      stride = rawChannelCount;
      repeat = wavChannelCount;
    }
    if (bytesPerSample > 1) {
      // usually samples are signed. 
      for (int i = 0; i < sample.size(); i += stride) {
        signed short val = sample[i] * 256 * 256;
        for (int k = 0; k < repeat; k++) {
          for (int j = 0; j < bitsPerSample; j += 8) {
            fputc(val >> j, file);
          }
        }
      }
    } else {
      // 8 bit samples are a special case: they are unsigned.
      for (int i = 0; i < sample.size(); i += stride) {
        unsigned char val = sample[i] * 256 + 128;
        for (int k = 0; k < repeat; k++) {
          fputc(val, file);
        }
      }
    }
    
    fclose(file);
  }
  */
}

void MyFrame::OnUndo(wxCommandEvent& WXUNUSED(event)) {
  _song->history.undo();
}

void MyFrame::OnRedo(wxCommandEvent& WXUNUSED(event)) {
  _song->history.redo();
}

void MyFrame::setInstrument(Instrument* inst) {
  m_inst = inst;
  printf("MyFrame::setInstrument\n");
  oscillatorPanel->setInstrument(m_inst);
  
  for (int i = 0; i < NUM_TIMELINES; i++) {
    timelinePanels[i]->setInstrument(inst);
  }
}

void MyFrame::OnRecordPressed( wxCommandEvent &event ) {
  recordButton->Enable(false);
  stopButton->Enable(true);
  startRecording = 1;
}

void MyFrame::OnStopPressed( wxCommandEvent &event ) {
  recordButton->Enable(true);
  stopButton->Enable(false);
  stopRecording = 1;
}
