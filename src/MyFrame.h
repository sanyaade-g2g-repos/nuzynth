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

#ifndef MY_FRAME_H
#define MY_FRAME_H

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/bookctrl.h>
#include <wx/choice.h>
#include "OscillatorPanel.h"
#include "SongCanvas.h"
#include "TimelinePanel.h"

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame {
public:
  MyFrame(const wxString& title);
  
  void setInstrument(Instrument* inst);
  
private:
  
  // event handlers (these functions should _not_ be virtual)
  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnExport(wxCommandEvent& event);
  void OnUndo( wxCommandEvent &event );
  void OnRedo( wxCommandEvent &event );
  void OnRecordPressed( wxCommandEvent &event );
  void OnStopPressed( wxCommandEvent &event );
  
  Instrument* m_inst;
  OscillatorPanel    *oscillatorPanel;
  
  wxButton* recordButton;
  wxButton* stopButton;
  
  TimelinePanel   *timelinePanels[NUM_TIMELINES];
  
  // any class wishing to process wxWidgets events must use this macro
  DECLARE_EVENT_TABLE()
};


// IDs for the controls and the menu commands
enum {
    FILE_EXPORT = wxID_HIGHEST,
    RECORD_BUTTON,
    STOP_BUTTON,
};

#endif // MY_FRAME_H
