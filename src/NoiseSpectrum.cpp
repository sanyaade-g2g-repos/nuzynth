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
#include <memory.h>
#include <math.h>
#include "NoiseSpectrum.hpp"
#include "random.h"

void NoiseSpectrum_init(NoiseSpectrum* noise) {
  memset (noise->linearSpectrum, 0, (SAMPLES_IN_WAVE) * (sizeof(float)));
  memset (noise->logSpectrum, 0, (NOISE_SPECTRUM_LENGTH + 1) * (sizeof(unsigned char)));
}

void NoiseSpectrum_updateAll(NoiseSpectrum* noise) {
  NoiseSpectrum_updateRange(noise, -EXTRA_OCTAVES_IN_WAVE - 1, OCTAVES_IN_PERIOD + 1);
}

void NoiseSpectrum_updateRange(NoiseSpectrum* noise, float lowerOctave, float upperOctave) {
  int lower = pow(2.0f, lowerOctave + EXTRA_OCTAVES_IN_WAVE);
  int upper = pow(2.0f, upperOctave + EXTRA_OCTAVES_IN_WAVE);
  if (upper > (SAMPLES_IN_WAVE >> 1)) upper = (SAMPLES_IN_WAVE >> 1);
  int lowestMatch = 1 << (EXTRA_OCTAVES_IN_WAVE - SUB_OCTAVES);
  float value;
  float pinkScale;
  float arbitraryScale = 10.0f;
  if (lower < lowestMatch) {
    // Updated lowest logarithmic sample, update low part of linear spectrum. 
    value = noise->logSpectrum[0] * (1.0f/255.0f);
    if (value > 0) {
      value -= 1.0f;
      value *= 16.0f;
      value = pow(2.0f, value);
    }
    // Technically the pink scale should vary with the frequency, but I don't want it to explode:
    pinkScale = 1.0f / sqrt( (float) lowestMatch );
    
    for (int i = 1; i < lowestMatch; i++) {
      //pinkScale = 1.0f / sqrt( (float) i );
      noise->linearSpectrum[i] = value * pinkScale * arbitraryScale;
    }
    
    lower = lowestMatch;
  }
  for (int i = lower; i < upper; i++) {
    float octave = log2((float) i) - EXTRA_OCTAVES_IN_WAVE + SUB_OCTAVES;
    octave *= NOISE_SAMPLES_PER_OCTAVE;
    
    int left = (int) octave;
    int right = (left + 1);
    float rightScale = octave - left;
    float leftScale = 1.0f - rightScale;
    value = (noise->logSpectrum[left] * leftScale) + (noise->logSpectrum[right] * rightScale);
    value *= (1.0f/255.0f);
    if (value > 0) {
      value -= 1.0f;
      value *= 16.0f;
      value = pow(2.0f, value);
    }
    
    pinkScale = 1.0f / sqrt( (float) i );
    noise->linearSpectrum[i] = value * pinkScale * arbitraryScale;
  }
}
