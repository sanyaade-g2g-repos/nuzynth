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

#include <math.h>
//#include <assert.h>

#include "midi.hpp"
#include "keyboard.hpp"

/*
PmStream* midi;
PmError status, length;
PmEvent buffer[1];
bool success;
*/
void midi_setup() 
{/*
  int default_in;
  //int default_out;
  //int i = 0, n = 0;
  //int stream_test = 0;
  
  //latency = 100; // latency is only used for midi output?

  // list device information
  default_in = Pm_GetDefaultInputDeviceID();
  //default_out = Pm_GetDefaultOutputDeviceID();
  
  if (default_in == -1) {
    success = false;
    return;
  }
  
  //TIME_START;
  Pm_OpenInput(&midi, 
               default_in,
               DRIVER_INFO, 
               INPUT_BUFFER_SIZE, 
               TIME_PROC, 
               TIME_INFO);
  Pm_SetFilter(midi, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);
  // empty the buffer after setting filter, just in case anything
  // got through 
  while (Pm_Poll(midi)) {
    Pm_Read(midi, buffer, 1);
  }
  
  success = true;*/
}

void midi_teardown()
{
  //if (success)
    //Pm_Close(midi);
}

bool midi_poll(bool* wasANote, bool* wasANoteStart, unsigned char* pitchIndex)
{/*
  if (!success) return false;
  status = Pm_Poll(midi);
  if (status != true) return false;
  
  length = (PmError) Pm_Read(midi, buffer, 1);
  if (length <= 0) return false; // raise an exception? assert?
  
  if (Pm_MessageStatus(buffer[0].message) == 0x90) {
    if (Pm_MessageData2(buffer[0].message) == 0) {
      *wasANoteStart = false;
    } else {
      *wasANoteStart = true;
    }
  } else if (Pm_MessageStatus(buffer[0].message) == 0x90) {
    *wasANoteStart = false;
  } else {
    *wasANote = false;
    return true;
  }
  *wasANote = true;
  
  *pitchIndex = Pm_MessageData1(buffer[0].message);
  return true;*/
  return false;
}


float midi_frequencyFromPitchIndex(unsigned char pitchIndex)
{
  return 440.0f * pow(2.0f, (float) (pitchIndex - 69) / 12.0f);
}

