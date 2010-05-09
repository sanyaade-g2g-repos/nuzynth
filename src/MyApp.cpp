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

#include "MyApp.h"
/*
#include "EnvelopeCanvas.h"
#include "HarmonicCanvas.h"
#include "IntervalCanvas.h"
#include "WaveCanvas.h"
*/
#include "midi.h"
#include "audio.h"
#include "audioCallback.h"
#include "Modulator.h"
#include "Instrument.h"
#include "keyboard.h"
#include "Clip.h"
#include "Track.h"
#include "SharedManagerBase.h"

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

BEGIN_EVENT_TABLE(MyApp, wxApp)
  EVT_KEY_DOWN( MyApp::OnKeyDown )
  EVT_KEY_UP( MyApp::OnKeyUp )
  EVT_IDLE( MyApp::OnIdle )
END_EVENT_TABLE()


// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;
    
    /// TODO: clean this up:
    _song = new Song();
    _song->clips.push_back(new Clip());
    _song->clips[0]->tracks.push_back(new Track(new Instrument(_song)));
    
    keyboard_init();
    audioCallback_init();
    
    // Do port midi before port audio, because the audio callback depends on midi being set up:
    midi_setup();
    
    // Now set up the port audio callback:
    audio_setup(audioCallback);
    
    // create the main application window
    frame = new MyFrame(_T("Nuzynth"));
    
    frame->setInstrument(_song->clips[0]->tracks[0]->original->instrument);

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);
    
    
    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}


int MyApp::OnExit()
{
  audio_tearDown();    
  midi_teardown();
  return wxApp::OnExit();
}

void MyApp::OnKeyDown( wxKeyEvent& event ) {
  
  long keyCode = event.GetKeyCode();
  char note = pitchIndexFromKeyCode(keyCode);
  if (!allKeys[note]) {
    pressedKeys.push_back(note);
    allKeys[note] = true;
  }
  
  event.Skip();
}
void MyApp::OnKeyUp( wxKeyEvent& event ) {
  long keyCode = event.GetKeyCode();
  char note = pitchIndexFromKeyCode(keyCode);
  releasedKeys.push_back(note);
  allKeys[note] = false;
  event.Skip();
}




void printLittleEndian(int val, int size, FILE* file) {
  unsigned char bytes[4];
  for (int i = 0; i < size; i++) {
    bytes[i] = (val & (0xff << (i * 8))) >> (i * 8);
  }
  fwrite(bytes, 1, size, file);
}


void MyApp::OnIdle( wxIdleEvent& event ) {
  SharedManagerBase::share();
  
  if (stopRecording) {
    if (!recordingStopped) {
      event.RequestMore();
    } else {
      startRecording = 0;
      stopRecording = 0;
      recordingStopped = 0;
      
      
      wxFileDialog dlg(frame, _T("Save a sample"),
                       wxEmptyString, _T("sample.wav"),
                       _T("WAV file (*.wav)|*.wav"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
      if ( dlg.ShowModal() == wxID_OK ) {
        const char* filepath = dlg.GetPath().mb_str(wxConvUTF8);
        FILE* file = fopen(filepath, "w");
        
        int rawChannelCount = 2;
        int wavChannelCount = 1;
        int sampleRate = 44100;
        int bytesPerSample = 2;
        int bitsPerSample = 8*bytesPerSample;
        int sampleCount = wavChannelCount * (recordedSamples.size() / rawChannelCount);
        
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
          for (int i = 0; i < recordedSamples.size(); i += stride) {
            signed short val = recordedSamples[i] * (1 << (bitsPerSample - 1));
            for (int k = 0; k < repeat; k++) {
              for (int j = 0; j < bitsPerSample; j += 8) {
                fputc(val >> j, file);
              }
            }
          }
        } else {
          // 8 bit samples are a special case: they are unsigned.
          for (int i = 0; i < recordedSamples.size(); i += stride) {
            unsigned char val = recordedSamples[i] * 256 + 128;
            for (int k = 0; k < repeat; k++) {
              fputc(val, file);
            }
          }
        }
        
        fclose(file);
      }
      recordedSamples.clear();
    }
  }
}
