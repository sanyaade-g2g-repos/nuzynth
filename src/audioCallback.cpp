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

#include <stdlib.h>
#include <math.h>
#include <memory.h>
//#include <assert.h>

#include "audio.hpp"
#include "audioCallback.hpp"
#include "midi.hpp"
#include "Tone.h"
#include "keyboard.hpp"
#include "Clip.hpp"
#include "Track.hpp"
#include "Instrument.hpp"







// Formant filter stolen from:
// http://www.musicdsp.org/showArchiveComment.php?ArchiveID=110

/*
Public source code by alex@smartelectronix.com
Simple example of implementation of formant filter
Vowelnum can be 0,1,2,3,4 <=> A,E,I,O,U
Good for spectral rich input like saw or square 
*/
//-------------------------------------------------------------VOWEL COEFFICIENTS
const double coeff[5][11]= {
{ 8.11044e-06,
8.943665402, -36.83889529, 92.01697887, -154.337906, 181.6233289,
-151.8651235, 89.09614114, -35.10298511, 8.388101016, -0.923313471 ///A
},
{4.36215e-06,
8.90438318, -36.55179099, 91.05750846, -152.422234, 179.1170248, ///E
-149.6496211,87.78352223, -34.60687431, 8.282228154, -0.914150747
},
{ 3.33819e-06,
8.893102966, -36.49532826, 90.96543286, -152.4545478, 179.4835618,
-150.315433, 88.43409371, -34.98612086, 8.407803364, -0.932568035 ///I
},
{1.13572e-06,
8.994734087, -37.2084849, 93.22900521, -156.6929844, 184.596544, ///O
-154.3755513, 90.49663749, -35.58964535, 8.478996281, -0.929252233
},
{4.09431e-07,
8.997322763, -37.20218544, 93.11385476, -156.2530937, 183.7080141, ///U
-153.2631681, 89.59539726, -35.12454591, 8.338655623, -0.910251753
}
}; 
//---------------------------------------------------------------------------------
static double memory[10]={0,0,0,0,0,0,0,0,0,0};
//---------------------------------------------------------------------------------
float formant_filter(float *in, int vowelnum)
{
float res= (float) ( coeff[vowelnum][0]* *in +
coeff[vowelnum][1] *memory[0] + 
coeff[vowelnum][2] *memory[1] +
coeff[vowelnum][3] *memory[2] +
coeff[vowelnum][4] *memory[3] +
coeff[vowelnum][5] *memory[4] +
coeff[vowelnum][6] *memory[5] +
coeff[vowelnum][7] *memory[6] +
coeff[vowelnum][8] *memory[7] +
coeff[vowelnum][9] *memory[8] +
coeff[vowelnum][10] *memory[9] );

memory[9]= memory[8];
memory[8]= memory[7];
memory[7]= memory[6];
memory[6]= memory[5];
memory[5]= memory[4];
memory[4]= memory[3];
memory[3]= memory[2];
memory[2]= memory[1]; 
memory[1]= memory[0];
memory[0]=(double) res;
return res;
}







// TODO: remove this stuff:
Song* _song;

#define BEATS_PER_MINUTE    (200)
#define BEATS_PER_SECOND    ((double)BEATS_PER_MINUTE / 60.0)
#define SAMPLES_PER_BEAT    ((double)SAMPLE_RATE / (double)BEATS_PER_SECOND)
#define MAX_TONES (50)

Tone tones[MAX_TONES + 1];
int oldestTone;
int newestTone;
int nextTone;
int totalTones;
double nextBeatSample;
int nextBeat;
char startRecording = 0;
char stopRecording = 0;
char recordingStopped = 0;
std::vector<float> recordedSamples;


int count;
int beat;

//int pitches[] = {42,44,46,49,51,54,56,58,61,63,66,68,70,73,75,78,};

void audioCallback_init() {
  memset(tones, 0, sizeof(Tone) * MAX_TONES);
  //memset(tonematrix, 0, sizeof(bool) * 256);
  count = 0;
  beat = 0;
  oldestTone = 0;
  newestTone = 0;
  nextTone = 1;
  totalTones = 0;
  nextBeatSample = 0.0;
  nextBeat = 0;
}

int createTone (Track* track, int note) {
  int id = track->currentClone->notes[note].id;
  float hertz = midi_frequencyFromPitchIndex(track->currentClone->notes[note].pitch);
  
  
  if (totalTones == MAX_TONES) {
    nextTone = oldestTone;
    oldestTone = tones[oldestTone].next;
  } else if (totalTones == 0) {
    oldestTone = 1;
    nextTone = 1;
    totalTones = 1;
  } else {
    while (tones[nextTone].alive) {
      nextTone++;
      if (nextTone > MAX_TONES) {
        nextTone = 1;
      }
    }
    totalTones++;
  }
  
  
  int tone = nextTone;
  tones[newestTone].next = tone;
  newestTone = tone;
  nextTone++;
  if (nextTone > MAX_TONES) nextTone = 1;
  
  Tone_create(&tones[tone], track, id, note, hertz);
  tones[tone].next = 0;
  
  return tone;
}

int destroyTone (int tone, int prevTone) {
  if (prevTone != 0) {
    tones[prevTone].next = tones[tone].next;
  }
  
  if (tone == oldestTone) {
    oldestTone = tones[tone].next;
  }
  if (tone == newestTone) {
    newestTone = prevTone;
    if (tone == nextTone -1) {
      nextTone = tone;
    }
  }
  
  memset(&tones[tone], 0, sizeof(Tone));
  
  totalTones--;
}



int audioCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
  // Prevent unused variable warnings:
  (void) inputBuffer;
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;
  
  memset(outputBuffer, 0, (sizeof(float) * 2) * framesPerBuffer);
  float *out = (float*)outputBuffer;
  
  /*
  // port midi:
  bool wasANote;
  bool wasANoteStart;
  unsigned char pitchIndex;
  
  while (midi_poll(&wasANote, &wasANoteStart, &pitchIndex)) {
    if (wasANote) {
      if (wasANoteStart) {
        for (int i = 0; i < MAX_TONES; i++) {
          if (tones[i].alive == false) {
            Tone_create(&tones[i], pitchIndex, inst, midi_frequencyFromPitchIndex(pitchIndex));
            break;
          }
        }
      } else {
        for (int i = 0; i < MAX_TONES; i++) {
          if (tones[i].id == pitchIndex) {
            tones[i].released = 1;
            //break;
          }
        }
      }
    }
  }
  
  // TODO: This iterating over key-press/release vectors is probably not thread safe!
  std::vector<unsigned char>::iterator iter = pressedKeys.begin();
  while (iter < pressedKeys.end()) {
    int pitch = *iter;
    if (pitch == 0) {
      iter++;
      continue;
    }
    bool tonematrixWasPlaying = false;
    for (int i = 0; i < 16; i++) for (int j = 0; j < 16; j++) {
      tonematrixWasPlaying = tonematrixWasPlaying || tonematrix[i][j];
      tonematrix[i][j] = false;
    }
    if (tonematrixWasPlaying) {
      for (int i = 0; i < MAX_TONES; i++) {
        tones[i].released = 1;
      }
    }
    for (int i = 0; i < MAX_TONES; i++) {
      if (tones[i].alive == false) {
        Tone_create(&tones[i], *iter, inst, midi_frequencyFromPitchIndex(*iter));
        break;
      }
    }
    iter++;
  }
  pressedKeys.erase(pressedKeys.begin(), pressedKeys.end());
  
  iter = releasedKeys.begin();
  while (iter < releasedKeys.end()) {
    if (*iter == 0) continue;
    for (int i = 0; i < MAX_TONES; i++) {
      if (tones[i].id == *iter) {
        tones[i].released = 1;
        //break;
      }
    }
    iter++;
  }
  releasedKeys.erase(releasedKeys.begin(), releasedKeys.end());
  */
  
  
  // pick a clone for each CloneManager to use for the duration of the 
  // callback so that I don't have to worry about the structure of the song
  // changing while I'm iterating over stuff. 
  for (int i = 0; i < _song->clips.size(); i++) {
    Clip* clip = _song->clips[i];
    for (int j = 0; j < clip->tracks.size(); j++) {
      Track* track = clip->tracks[j];
      track->updateCurrentCloneToLatest();
      if (track->currentClone != 0) {
        Instrument* instrument = track->currentClone->instrument;
        if (instrument != 0) {
          instrument->updateCurrentCloneToLatest();
        }
      }
    }
  }
  
  
  int prevTone;
  int curTone;
  
  prevTone = 0;
  curTone = oldestTone;
  while (curTone != 0) {
    bool valid = true;
    
    Tone* tone = &tones[curTone];
    
    if (tone->alive == false ||
        tone->track->currentClone == 0 || 
        tone->track->currentClone->instrument == 0 || 
        tone->track->currentClone->instrument->currentClone == 0)
    {
      valid = false;
    } else {
      Note* notes = tone->track->currentClone->notes;
      if (notes[tone->note].on == false || 
          notes[tone->note].id != tone->id)
      {
        valid = false;
      } else if (tone->released == 0) {
        /// TODO: Check if the note has been moved away from this callback
        //        period. If so, mark it as invalid. Or maybe just release it?
      }
    }
    
    
    if (valid) {
      prevTone = curTone;
      curTone = tone->next;
    } else {
      int temp = curTone;
      curTone = tone->next;
      destroyTone(temp, prevTone);
    }
  }
  
  
  int sampleIndex = nextBeatSample;
  while (sampleIndex < framesPerBuffer) {
    // nextBeat occurs inside this callback duration!
    
    // Iterate over unreleased tones and look for NOTE_ENDS at this beat.
    // If found, render the tone up to its end, then release the tone
    // but don't destroy it. 
    prevTone = 0;
    curTone = oldestTone;
    while (curTone != 0) {
      Tone* tone = &tones[curTone];
      if (tone->released == false) {
        Note* notes = tone->track->currentClone->notes;
        int nextJoined = notes[tone->note].nextJoined;
        if (notes[nextJoined].beat == nextBeat && 
            notes[nextJoined].type == NOTE_END)
        {
          Modulator* mod = tone->track->currentClone->instrument->currentClone;
          mod->synthFunction(tone, mod, 
                             &out[tone->firstSampleIndex * 2], 
                             sampleIndex - tone->firstSampleIndex);
          tone->firstSampleIndex = sampleIndex;
          tone->released = 1;
        }
      }
      
      prevTone = curTone;
      curTone = tone->next;
    }
    
    /// TODO: Get the total number of beats from the Clip:
    if (nextBeat >= 16) {
      // We want to get the NOTE_ENDs from the end of the track, but 
      // we want to get the NOTE_STARTs from the beginning of the track. 
      nextBeat = 0;
    }
    
    
    /// TODO: Iterate over the tracks and look for NOTE_STARTs.
    {
      Track* track = _song->clips[0]->tracks[0];
      if (track->currentClone == 0 || 
          track->currentClone->instrument->currentClone == 0)
      {
        //continue;
      } else {
        /// TODO: Don't check every single note every single time. 
        Note* notes = track->currentClone->notes;
        int prevNote = 0;
        int curNote = track->currentClone->head;
        while (curNote != 0) {
          if (notes[curNote].beat == nextBeat &&
              notes[curNote].type == NOTE_START)
          {
            int tone = createTone(track, curNote);
            tones[tone].firstSampleIndex = sampleIndex;
          }
          curNote = notes[curNote].next;
        }
      }
    }
    
    nextBeatSample += SAMPLES_PER_BEAT;
    sampleIndex = nextBeatSample;
    nextBeat++;
  }
  nextBeatSample -= (double)framesPerBuffer;
  
  // Finally, render all tones that are still alive at the end of the callback:
  prevTone = 0;
  curTone = oldestTone;
  while (curTone != 0) {
    Tone* tone = &tones[curTone];
    
    Modulator* mod = tone->track->currentClone->instrument->currentClone;
    mod->synthFunction(tone, mod, 
                       &out[tone->firstSampleIndex * 2], 
                       framesPerBuffer - tone->firstSampleIndex);
    
    tone->firstSampleIndex = 0;
    
    prevTone = curTone;
    curTone = tone->next;
  }
  
  SharedManagerBase::markOldStuffAsUnused();
  
  float *out2 = (float*)outputBuffer;
  
  /*
  for (int i = 0; i < framesPerBuffer * 2; i+=2) {
    out2[i] = formant_filter(&out2[i], 0);
    out2[i+1] = out2[i];
  }
  */
  
  for (int i = 0; i < framesPerBuffer * 2; i++) {
    
    // envelope limiter adapted from:
    // http://www.musicdsp.org/showone.php?id=265
    static float releaseMs = 40000;
    static float sampleRate = 44100 * 2; // 2 channels
    static float envelope = 0;
    static float r = pow( 0.01, 1.0 / ( releaseMs * sampleRate * 0.001 ) );
    float v = fabs( *out2 );
    if ( v > envelope ) envelope = v;
    else                envelope = r * ( envelope - v ) + v;
    if (envelope > 0.0f) {
      *out2 = *out2 * (1.0f - pow(2.0f, -envelope)) / envelope;
    }
    
    // Clip distortion:
    /*
    if (*out2 > 0.5f) *out2 = 0.5f;
    else if (*out2 < -0.5f) *out2 = -0.5f;
    */
    
    // tube distortion:
    //out2[i] = tanh(out2[i]);
    
    out2++;
  }
  
  if (startRecording) {
    if (stopRecording) {
      if (!recordingStopped) {
        recordingStopped = 1;
      }
    } else {
      float *out = (float*)outputBuffer;
      int oldSize = recordedSamples.size();
      recordedSamples.resize(oldSize + framesPerBuffer * 2);
      memcpy(&recordedSamples[oldSize], out, sizeof(float) * framesPerBuffer * 2);
    }
  }
  
  return paContinue;
}
