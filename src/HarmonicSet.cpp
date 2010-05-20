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

#include <memory.h>
#include <math.h>
#include "Modulator.h"
#include "HarmonicSet.h"

void HarmonicSet_init(HarmonicSet* set, bool silent)
{
  // Some of these are initialized by the update function instead, so don't bother zeroing those...?
  memset (&set->freqs, 0, (MAX_HARMONICS) * (sizeof(float)));
  memset (&set->amplitudes, 0, (MAX_HARMONICS) * (sizeof(unsigned char)));
  
  if (!silent) {
    set->amplitudes[0] = 255;
    set->amplitudes[1] = 230;
    set->amplitudes[2] = 180;
    set->amplitudes[3] = 70;
    set->amplitudes[4] = 20;
    set->amplitudes[5] = 7;
  }
  
  set->blur = 0;
  set->stretch = 127;
  
  set->numerator = 1;
  set->denominator = 1;
}

// Find the nearest fraction with a denominator of 
// PERIODS_IN_WAVE (128?) 
// and an integer numerator.
static inline float roundToRational(float in)
{
  return (float) round(in * PERIODS_IN_WAVE) * PERIODS_IN_WAVE_INVERSE;
}

void HarmonicSet_updateFreqs(HarmonicSet* set)
{
  
  float freq = 0.0f;
  float interval = (float) set->numerator / (float) set->denominator;
  float stretch = 2.0f * (float) set->stretch / 254.0f - 1.0f;
  float delta;
  float multiply;
  if (stretch > 0.0f) {
    delta = stretch * stretch * stretch * (1.0f / 3.0f);
    multiply = 1.0f;
  } else {
    delta = 0.0f;
    multiply = 1.0f - stretch * stretch * 0.5f;
  }
  
  freq += interval;
  set->freqs[0] = roundToRational(freq);
  set->enabled[0] = (set->freqs[0] >= pow(2.0f, -SUB_OCTAVES) && set->freqs[0] < pow(2.0f, OCTAVES_IN_PERIOD - 1));
  
  for (int i = 1; i < MAX_HARMONICS; i++) {
    interval += delta;
    freq += interval * multiply;
    set->freqs[i] = roundToRational(freq);
    set->enabled[i] = (set->freqs[i] >= pow(2.0f, -SUB_OCTAVES) && set->freqs[i] < pow(2.0f, OCTAVES_IN_PERIOD - 1));
  }
}
