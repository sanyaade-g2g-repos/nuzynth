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

#include "Oscillator.h"
#include "math.h"
#include "random.h"
#include "Instrument.h"
#include "Pool.h"

fftwf_complex angles[SAMPLES_IN_WAVE >> 1];
fftwf_complex* spectrum;
//float* tempWave;
fftwf_plan plan;
bool spectrumInitialized = false;
bool planInitialized = false;

void initializeSpectrumAndAngles() {
  spectrum = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * SAMPLES_IN_WAVE);
  spectrumInitialized = true;
  
  //tempWave = (float*) Pool_draw(Instrument::wavePool());
  
  for (int i = 0; i < (SAMPLES_IN_WAVE >> 1); i++) {
    float angle = fastRandom(-M_PI, M_PI);
    angles[i][0] = sin(angle);
    angles[i][1] = cos(angle);
  }
}

void Oscillator_init(Oscillator* oscillator) {
  if (spectrumInitialized == false) initializeSpectrumAndAngles();
  
  HarmonicSet_init(&oscillator->harmonicSet[0], false);
  for (int i = 1; i < NUM_VOICES; i++) {
    HarmonicSet_init(&oscillator->harmonicSet[i], true);
  }
  
  NoiseSpectrum_init(&oscillator->noise);
  
  oscillator->dirty = true;
}

void Oscillator_renderWave(Oscillator* oscillator, float* wave) {
  
  if (planInitialized == false) {
    plan = fftwf_plan_dft_c2r_1d(SAMPLES_IN_WAVE, spectrum, wave, FFTW_MEASURE);
    planInitialized = true;
  }
  
  spectrum[                   0][0] = 0;
  spectrum[                   0][1] = 0;
  spectrum[SAMPLES_IN_WAVE >> 1][0] = 0;
  spectrum[SAMPLES_IN_WAVE >> 1][1] = 0;
  for (int i = 1; i < (SAMPLES_IN_WAVE >> 1); i++) {
    float value = oscillator->noise.linearSpectrum[i];
    spectrum[                  i][0] = value * angles[i][0];
    spectrum[                  i][1] = value * angles[i][1];
    spectrum[SAMPLES_IN_WAVE - i][0] = value * angles[i][0];
    spectrum[SAMPLES_IN_WAVE - i][1] = -value * angles[i][1];
  }
  
  for (int i = 0; i < NUM_VOICES; i++) {
    HarmonicSet *set = &oscillator->harmonicSet[i];
    HarmonicSet_updateFreqs(set);
    for (int j = 0; j < MAX_HARMONICS; j++) {
      if (set->amplitudes[j] == 0) continue;
      if (set->enabled[j] == false) continue;
      
      int middle = set->freqs[j] * PERIODS_IN_WAVE;
      float power = (float) set->amplitudes[j] / 255.0f;
      power -= 1.0f;
      power *= 13.0f;
      power = pow(2.0f, power);
      power *= 0.5; // use a range from -0.5 to 0.5, then add to both + and - frequencies:
      
      float width = ((float) set->blur / 254.0f);
      width *= width * width * width;
      width *= set->freqs[j] * (float) SAMPLES_IN_WAVE / 500.0f;
      width += 1.0f;
      float peak = power / sqrt(width);
      
      spectrum[                  middle][0] += peak * angles[middle][0];
      spectrum[                  middle][1] += peak * angles[middle][1];
      spectrum[SAMPLES_IN_WAVE - middle][0] += peak * angles[middle][0];
      spectrum[SAMPLES_IN_WAVE - middle][1] += -peak * angles[middle][1];
      
      for (int k = 1; k < width; k++) {
        float value = peak;
        if (middle + k < (SAMPLES_IN_WAVE >> 1)) {
          spectrum[                  (middle + k)][0] += value * angles[middle + k][0];
          spectrum[                  (middle + k)][1] += value * angles[middle + k][1];
          spectrum[SAMPLES_IN_WAVE - (middle + k)][0] += value * angles[middle + k][0];
          spectrum[SAMPLES_IN_WAVE - (middle + k)][1] += -value * angles[middle + k][1];
        }
        if (middle - k > 1) {
          spectrum[                  (middle - k)][0] += value * angles[middle - k][0];
          spectrum[                  (middle - k)][1] += value * angles[middle - k][1];
          spectrum[SAMPLES_IN_WAVE - (middle - k)][0] += value * angles[middle - k][0];
          spectrum[SAMPLES_IN_WAVE - (middle - k)][1] += -value * angles[middle - k][1];
        }
      }
    }
  }
  
  fftwf_execute_dft_c2r(plan, spectrum, wave);
  
  
  float max = 0.0f;
  for (int i = 0; i < SAMPLES_IN_WAVE; i++) {
    float value = fabs(wave[i]);
    if (value > max) max = value;
  }
  
  if (max > 1.0f) {
    float scale = 1/max;
    for (int i = 0; i < SAMPLES_IN_WAVE; i++) {
      wave[i] *= scale;
    }
  }
  
  oscillator->dirty = false;
}
