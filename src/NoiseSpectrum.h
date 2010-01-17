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

#ifndef __NOISE_SPECTRUM_H__
#define __NOISE_SPECTRUM_H__

#include "Modulator.h"
#include <fftw3.h>

#define NOISE_SPECTRUM_OCTAVES   (SUB_OCTAVES + OCTAVES_IN_PERIOD - 1)
#define NOISE_SAMPLES_PER_OCTAVE (24)
#define NOISE_SPECTRUM_LENGTH    (NOISE_SAMPLES_PER_OCTAVE * NOISE_SPECTRUM_OCTAVES)

struct NoiseSpectrum {
  float linearSpectrum[SAMPLES_IN_WAVE];
  unsigned char logSpectrum[NOISE_SPECTRUM_LENGTH + 1];
};

void NoiseSpectrum_init(NoiseSpectrum* noise);
void NoiseSpectrum_updateAll(NoiseSpectrum* noise);
void NoiseSpectrum_updateRange(NoiseSpectrum* noise, float lowerOctave, float upperOctave);

#endif // __NOISE_SPECTRUM_H__
