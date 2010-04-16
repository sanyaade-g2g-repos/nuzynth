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

#ifndef TONE_H
#define TONE_H

#include "Modulator.h"
#include "CloneManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Tone {
  unsigned int id;
  CloneManager<Modulator>* instrument;
  float wavePhase;
  float wavePhase2;
  float waveSpeed;
  float timelinePhases[NUM_TIMELINES-1];
  int age;
  float prevLowPassOutput;
  float prevLowPassDelta;
  float prevHighPassInput;
  float prevHighPassOutput;
  float prevBrownOutput;
  char released;
  char alive;
} Tone;

void Tone_create(Tone* tone, unsigned int id, CloneManager<Modulator>* instrument, float hertz);

#ifdef __cplusplus
}
#endif

#endif // TONE_H
