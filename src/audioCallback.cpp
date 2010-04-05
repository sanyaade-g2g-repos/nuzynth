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

#include "audio.h"
#include "audioCallback.h"
#include "midi.h"
#include "Tone.h"
#include "keyboard.h"
#include "Clip.h"
#include "Track.h"







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

#define MAX_MIDI_TONES (32)
Tone tones[MAX_MIDI_TONES];
char startRecording = 0;
char stopRecording = 0;
char recordingStopped = 0;
std::vector<float> recordedSamples;


bool tonematrix[16][16];

int count;
int beat;

int pitches[] = {42,44,46,49,51,54,56,58,61,63,66,68,70,73,75,78,};

void audioCallback_init() {
  memset(tones, 0, sizeof(Tone) * MAX_MIDI_TONES);
  memset(tonematrix, 0, sizeof(bool) * 256);
  count = 0;
  beat = 15;
}

int audioCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    (void) inputBuffer;
    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) userData;
    
    /// TODO: Make sure this is thread safe:
    Instrument* inst = _song->clips[0]->tracks[0]->instrument;
    
    memset(outputBuffer, 0, (sizeof(float) * 2) * framesPerBuffer);
    
    // port midi:
    bool wasANote;
    bool wasANoteStart;
    unsigned char pitchIndex;
    
    while (midi_poll(&wasANote, &wasANoteStart, &pitchIndex)) {
      if (wasANote) {
        if (wasANoteStart) {
          for (int i = 0; i < MAX_MIDI_TONES; i++) {
            if (tones[i].alive == 0) {
              Tone_create(&tones[i], pitchIndex, inst, midi_frequencyFromPitchIndex(pitchIndex));
              break;
            }
          }
        } else {
          for (int i = 0; i < MAX_MIDI_TONES; i++) {
            if (tones[i].id == pitchIndex) {
              tones[i].released = 1;
              //break;
            }
          }
        }
      }
    }
    
    count++;
    if (count == 40) {
      count = 0;
      
      for (int i = 0; i < 16; i++) {
        pitchIndex = pitches[i];
        if (tonematrix[beat][i]) {
          for (int j = 0; j < MAX_MIDI_TONES; j++) {
            if (tones[j].id == pitchIndex) {
              tones[j].released = 1;
            }
          }
        }
      }
      
      beat++;
      if (beat == 16) beat = 0;
      
      for (int i = 0; i < 16; i++) {
        pitchIndex = pitches[i];
        if (tonematrix[beat][i]) {
          for (int j = 0; j < MAX_MIDI_TONES; j++) {
            if (tones[j].alive == 0) {
              Tone_create(&tones[j], pitchIndex, inst, midi_frequencyFromPitchIndex(pitchIndex));
              break;
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
        for (int i = 0; i < MAX_MIDI_TONES; i++) {
          tones[i].released = 1;
        }
      }
      for (int i = 0; i < MAX_MIDI_TONES; i++) {
        if (tones[i].alive == 0) {
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
      for (int i = 0; i < MAX_MIDI_TONES; i++) {
        if (tones[i].id == *iter) {
          tones[i].released = 1;
          //break;
        }
      }
      iter++;
    }
    releasedKeys.erase(releasedKeys.begin(), releasedKeys.end());
    
    //std::vector<Sharer<Modulator>*> localDeathRow;
    
    for (int i = 0; i < MAX_MIDI_TONES; i++) {
      Tone* tone = &tones[i];
      if (tone->alive == 0) continue;
      
      if (tone->instrument->isCondemned()) {
        //localDeathRow.push_back(tone->instrument);
        tone->alive = 0;
        //tone->instrument->abandon();
        continue;
      }
      
      float *out = (float*)outputBuffer;
      
      // loop!
      Modulator* mod = tone->instrument->getSharedDataClone();
      mod->used = 1;
      
      if (mod != 0) {
        mod->loopFunction(tone, mod, out, framesPerBuffer);
      }
      
      /*
      if (tone->alive == 0) {
        tone->instrument->numReferences--;
      }*/
    }
    
    /// TODO: Mark all condemned things as abandoned. 
    SharedManagerBase::abandonCondemnedManagers();
    
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
      
      /// old envelope limiter
      /*
      static float attackMs = 5;
      static float releaseMs = 2000;
      static float sampleRate = 44100 * 2; // 2 channels
      static float envelope = 0;
      static float a = pow( 0.01, 1.0 / ( attackMs * sampleRate * 0.001 ) );
      static float r = pow( 0.01, 1.0 / ( releaseMs * sampleRate * 0.001 ) );
      float v = fabs( *out2 ) * 1.3;
      if ( v > envelope ) envelope = a * ( envelope - v ) + v;
      else                envelope = r * ( envelope - v ) + v;
      
      //if ( envelope > 1.0f ) *out2 = *out2 / envelope;
      if ( envelope > 0.0f ) *out2 = *out2 * 0.9f * tanh(envelope) / envelope;
      
      if (*out2 > 0.99999f) *out2 = 0.99999f;
      else if (*out2 < -0.99999f) *out2 = -0.99999f;
      */
      
      // new envelope limiter
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
