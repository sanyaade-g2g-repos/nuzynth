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

#include "Tone.h"
#include "audio.h"

#include <stdlib.h>
#include <stdio.h>
#include "random.h"

float randomZeroCrossing(float* wave) {
  if (wave[0] == 0.0f) return 0.0f;
  int left, right;
  bool leftSign, rightSign;
  left = fastRandom(0.0f, (float) SAMPLES_IN_WAVE);
  right = (left + 1) & WAVE_MASK;
  leftSign = (wave[left] < 0);
  rightSign = (wave[right] < 0);
  while (leftSign == rightSign) {
    left = right;
    right = (right + 1) & WAVE_MASK;
    leftSign = rightSign;
    rightSign = (wave[right] < 0);
  }
  float zeroDist = -wave[left];
  float totalDist = wave[right] - wave[left];
  float result = (float) left + (zeroDist / totalDist);
  return result;
}

void Tone_create(Tone* tone, unsigned int id, SharedManager<Modulator>* instrument, float hertz) {
  tone->id = id;
  tone->instrument = instrument;
  //instrument->numReferences++;
  
  Modulator* mod = instrument->getSharedDataClone();
  tone->wavePhase = randomZeroCrossing(mod->wave);
  tone->wavePhase2 = randomZeroCrossing(mod->wave);
  //tone->wavePhase2 = fastRandom(0.0f, (float) SAMPLES_IN_WAVE);
  
  tone->waveSpeed = hertz * ((float) SAMPLES_IN_PERIOD / (float) SAMPLE_RATE);
  tone->timelinePhases[ATTACK_TIMELINE] = 0.0f;
  tone->timelinePhases[RELEASE_TIMELINE] = 0.0f;
  tone->timelinePhases[LOOP_1_TIMELINE] = 0.0f;
  tone->timelinePhases[LOOP_2_TIMELINE] = fastRandom(0.0f, (float) EFFECT_LENGTH);
  tone->age = 0;
  tone->released = false;
  tone->alive = true;
  tone->prevLowPassOutput = 0.0f;
  tone->prevLowPassDelta = 0.0f;
  tone->prevHighPassOutput = 0.0f;
  tone->prevHighPassInput = 0.0f;
  tone->prevBrownOutput = 0.0f;
}
