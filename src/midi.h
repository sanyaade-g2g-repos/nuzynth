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

#ifndef MIDI_H 
#define MIDI_H 

#include <stdio.h>
//#include <portmidi.h>

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define DRIVER_INFO NULL

// we're not using the porttime utilities. 
//#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */
//#define TIME_PROC ((long (*)(void *)) Pt_Time)
#define TIME_PROC NULL
#define TIME_INFO NULL


void midi_setup();

void midi_teardown();

// Returns true if a MIDI event was found. Keep calling this function until no more events are 
// found, or the event buffer will overflow. 
// The parameters are pointers to data that will be overwritten by the event.
// -wasANote will be true if a Note-On event or a Note-Off event was processed. These events
//   are the only events that we actually care about. 
// -If wasANote is true, wasANoteStart will indicate whether it was a Note-On (true) or a Note-Off (false)
// -pitchIndex will be a number from 0 to 127 representing the key that was played or released. 
bool midi_poll(bool* wasANote, bool* wasANoteStart, unsigned char* pitchIndex);


// A utility function to convert from MIDI pitch index into hertz, assuming no pitch bend:
float midi_frequencyFromPitchIndex(unsigned char pitchIndex);

#endif // MIDI_H 
