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

#include "../src/Tone.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include "../src/random.h"

#define RANGE_INFINITE     (0)
#define RANGE_FINITE       (1)

#define DO_NOT_SCALE_LOOP  (0)
#define DO_SCALE_LOOP      (1)

#define FIND_EFFECT_VAL_RANGE_SCALE(rangeType, range) \
        range *= (1.0f / 254.0f); \
        if (rangeType == RANGE_INFINITE) range *= range;

#define FIND_EFFECT_VAL_SCALE(rangeType, bufferVal) \
        if (rangeType == RANGE_INFINITE) bufferVal = bufferVal * (2.0f / 254.0f) - (1.0f); \
        if (rangeType == RANGE_FINITE)   bufferVal *= (1.0f / 254.0f); 

#define FIND_EFFECT_VAL_RANGE(rangeType, bufferVal, range) \
        if (rangeType == RANGE_INFINITE) bufferVal *= range; \
        /*if (rangeType == RANGE_FINITE)   bufferVal = (bufferVal - 1.0f) * range + 1.0f;*/ 

#define FIND_EFFECT_VAL_COMBINE(rangeType, bufferVal, effectVal) \
        if (rangeType == RANGE_INFINITE) effectVal += bufferVal; \
        if (rangeType == RANGE_FINITE)   effectVal *= bufferVal; 

#define FIND_EFFECT_VAL_LOOP_RANGE(rangeType, bufferVal, range, scaleLoop, loopScale) \
        if (scaleLoop == DO_NOT_SCALE_LOOP) FIND_EFFECT_VAL_RANGE(rangeType, bufferVal, range) \
        if (scaleLoop == DO_SCALE_LOOP) { \
          if (rangeType == RANGE_INFINITE) bufferVal *= range * loopScale; \
          if (rangeType == RANGE_FINITE)   bufferVal = (bufferVal - 1.0f) /* * range */ * loopScale + 1.0f; \
        } 

#define FIND_EFFECT_VAL(effect, rangeType, scaleLoop) \
      unsigned char* buffer; \
      float bufferVal, range; \
      if (GET_OPTION(options[effect], OPTION_CONSTANT)) { \
        if (rangeType == RANGE_INFINITE) { \
          float temp = mod->depths[CONSTANT_TIMELINE][effect] * (2.0f / 254.0f) - 1.0f; \
          effectVal = temp * temp; \
          /* This is a fast way to copy the sign bit of a float32: */ \
          (int&)effectVal |= (int&)temp & 0x80000000; \
        } \
        if (rangeType == RANGE_FINITE)   effectVal = mod->depths[CONSTANT_TIMELINE][effect] * (1.0f / 254.0f); \
      } \
      if (GET_OPTION(options[effect], OPTION_ATTACK)) { \
        buffer = mod->buffers[ATTACK_TIMELINE][effect]; \
        range = mod->depths[ATTACK_TIMELINE][effect]; \
        FIND_EFFECT_VAL_RANGE_SCALE(rangeType, range) \
        bufferVal = (buffer[buffer1Start] * buffer1StartScale) + (buffer[buffer1End] * buffer1EndScale); \
        FIND_EFFECT_VAL_SCALE(rangeType, bufferVal) \
        FIND_EFFECT_VAL_RANGE(rangeType, bufferVal, range) \
        FIND_EFFECT_VAL_COMBINE(rangeType, bufferVal, effectVal) \
      } \
      if (GET_OPTION(options[effect], OPTION_RELEASE)) { \
        buffer = mod->buffers[RELEASE_TIMELINE][effect]; \
        range = mod->depths[RELEASE_TIMELINE][effect]; \
        FIND_EFFECT_VAL_RANGE_SCALE(rangeType, range) \
        bufferVal = (buffer[buffer2Start] * buffer2StartScale) + (buffer[buffer2End] * buffer2EndScale); \
        FIND_EFFECT_VAL_SCALE(rangeType, bufferVal) \
        FIND_EFFECT_VAL_RANGE(rangeType, bufferVal, range) \
        FIND_EFFECT_VAL_COMBINE(rangeType, bufferVal, effectVal) \
      } \
      if (GET_OPTION(options[effect], OPTION_LOOP_1)) { \
        buffer = mod->buffers[LOOP_1_TIMELINE][effect]; \
        range = mod->depths[LOOP_1_TIMELINE][effect]; \
        FIND_EFFECT_VAL_RANGE_SCALE(rangeType, range) \
        bufferVal = (buffer[buffer3Start] * buffer3StartScale) + (buffer[buffer3End] * buffer3EndScale); \
        FIND_EFFECT_VAL_SCALE(rangeType, bufferVal) \
        FIND_EFFECT_VAL_LOOP_RANGE(rangeType, bufferVal, range, scaleLoop, loopScale) \
        FIND_EFFECT_VAL_COMBINE(rangeType, bufferVal, effectVal) \
      } \
      if (GET_OPTION(options[effect], OPTION_LOOP_2)) { \
        buffer = mod->buffers[LOOP_2_TIMELINE][effect]; \
        range = mod->depths[LOOP_2_TIMELINE][effect]; \
        FIND_EFFECT_VAL_RANGE_SCALE(rangeType, range) \
        bufferVal = (buffer[buffer4Start] * buffer4StartScale) + (buffer[buffer4End] * buffer4EndScale); \
        FIND_EFFECT_VAL_SCALE(rangeType, bufferVal) \
        FIND_EFFECT_VAL_RANGE(rangeType, bufferVal, range) \
        FIND_EFFECT_VAL_COMBINE(rangeType, bufferVal, effectVal) \
      }










void synth(char* options, Tone* tone, Modulator* mod, float* out, int frames) {
  int i;
  
  char hasAttackEffect = 0;
  char hasReleaseEffect = 0;
  char hasLoop1Effect = 0;
  char hasLoop2Effect = 0;
  
  hasAttackEffect |= GET_OPTION(options[VOLUME],       OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[HIGHPASS],     OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[LOWPASS],      OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[LOWPASS_PEAK], OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[PITCH],        OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[PITCH_2],      OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[DISTORTION],   OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[PAN],          OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[LOOP_SPEED],   OPTION_ATTACK);
  hasAttackEffect |= GET_OPTION(options[LOOP_DEPTH],   OPTION_ATTACK);
  
  hasReleaseEffect |= GET_OPTION(options[VOLUME],       OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[HIGHPASS],     OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[LOWPASS],      OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[LOWPASS_PEAK], OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[PITCH],        OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[PITCH_2],      OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[DISTORTION],   OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[PAN],          OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[LOOP_SPEED],   OPTION_RELEASE);
  hasReleaseEffect |= GET_OPTION(options[LOOP_DEPTH],   OPTION_RELEASE);
  
  hasLoop1Effect |= GET_OPTION(options[VOLUME],       OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[HIGHPASS],     OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[LOWPASS],      OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[LOWPASS_PEAK], OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[PITCH],        OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[PITCH_2],      OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[DISTORTION],   OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[PAN],          OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[LOOP_SPEED],   OPTION_LOOP_1);
  hasLoop1Effect |= GET_OPTION(options[LOOP_DEPTH],   OPTION_LOOP_1);
  
  hasLoop2Effect |= GET_OPTION(options[VOLUME],       OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[HIGHPASS],     OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[LOWPASS],      OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[LOWPASS_PEAK], OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[PITCH],        OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[PITCH_2],      OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[DISTORTION],   OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[PAN],          OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[LOOP_SPEED],   OPTION_LOOP_2);
  hasLoop2Effect |= GET_OPTION(options[LOOP_DEPTH],   OPTION_LOOP_2);
  
  float sliders[NUM_EFFECT_TYPES];
  
  float loopSpeed;
  float effectVal;
  
  float prevLowPassOutput;
  float prevLowPassDelta;
  float prevHighPassOutput;
  float prevHighPassInput;
  if (options[LOWPASS] != 0)  prevLowPassOutput = tone->prevLowPassOutput;
  if (options[LOWPASS] != 0)  prevLowPassDelta = tone->prevLowPassDelta;
  if (options[HIGHPASS] != 0) prevHighPassOutput = tone->prevHighPassOutput;
  if (options[HIGHPASS] != 0) prevHighPassInput = tone->prevHighPassInput;
  
  float* wave = mod->wave;
  float waveBaseSpeed = tone->waveSpeed;
  
  // I encountered numerical errors when wavePhase was a float and the chorus pitch
  // was just barely different from the primary pitch. 
  double wavePhase = tone->wavePhase;
  double waveSpeed;
  
  double wavePhase2;
  double waveSpeed2;
  if (options[PITCH_2] != 0) wavePhase2 = tone->wavePhase2;
  
  float buffer1Phase = tone->timelinePhases[ATTACK_TIMELINE];
  float buffer1Speed = ((float)mod->speeds[ATTACK_TIMELINE] + 5) * 0.00005;
  buffer1Speed *= buffer1Speed;
  
  float buffer2Phase = tone->timelinePhases[RELEASE_TIMELINE];
  float buffer2Speed = ((float)mod->speeds[RELEASE_TIMELINE] + 5) * 0.00005;
  buffer2Speed *= buffer2Speed;
  
  float buffer3Phase = tone->timelinePhases[LOOP_1_TIMELINE];
  float buffer3Speed = ((float)mod->speeds[LOOP_1_TIMELINE] + 5) * 0.001;
  buffer3Speed *= buffer3Speed;
  
  float buffer4Phase = tone->timelinePhases[LOOP_2_TIMELINE];
  float buffer4Speed = ((float)mod->speeds[LOOP_2_TIMELINE] + 5) * 0.001;
  buffer4Speed *= buffer4Speed;
  
  float volume;
  
  for( i = 0; i < frames; i++ ) {
    
    int waveStart = (int) wavePhase;
    int waveEnd = (waveStart + 1) & WAVE_MASK;
    float waveEndScale = wavePhase - waveStart;
    float waveStartScale = 1.0f - waveEndScale;
    
    int waveStart2;
    int waveEnd2;
    float waveEndScale2;
    float waveStartScale2;
    if (options[PITCH_2] != 0) {
      waveStart2 = (int) wavePhase2;
      waveEnd2 = (waveStart2 + 1) & WAVE_MASK;
      waveEndScale2 = wavePhase2 - waveStart2;
      waveStartScale2 = 1.0f - waveEndScale2;
    }
    
    int buffer1Start;
    int buffer1End;
    float buffer1EndScale;
    float buffer1StartScale;
    if (hasAttackEffect) {
      float phase = sqrt(buffer1Phase) * (EFFECT_LENGTH - 1.0f);
      buffer1Start = (int) phase;
      buffer1End = (buffer1Start + 1); // note the lack of a mask... this buffer doesn't loop.
      buffer1EndScale = phase - buffer1Start;
      buffer1StartScale = 1.0f - buffer1EndScale;
    }
    
    int buffer2Start;
    int buffer2End;
    float buffer2EndScale;
    float buffer2StartScale;
    if (hasReleaseEffect) {
      float phase = sqrt(buffer2Phase) * (EFFECT_LENGTH - 1.0f);
      buffer2Start = (int) phase;
      buffer2End = (buffer2Start + 1); // again, no mask
      buffer2EndScale = phase - buffer2Start;
      buffer2StartScale = 1.0f - buffer2EndScale;
    }
    
    int buffer3Start;
    int buffer3End;
    float buffer3EndScale;
    float buffer3StartScale;
    if (hasLoop1Effect) {
      buffer3Start = (int) buffer3Phase;
      buffer3End = (buffer3Start + 1) & EFFECT_MASK;
      buffer3EndScale = buffer3Phase - buffer3Start;
      buffer3StartScale = 1.0f - buffer3EndScale;
    }
    
    int buffer4Start;
    int buffer4End;
    float buffer4EndScale;
    float buffer4StartScale;
    if (hasLoop2Effect) {
      buffer4Start = (int) buffer4Phase;
      buffer4End = (buffer4Start + 1) & EFFECT_MASK;
      buffer4EndScale = buffer4Phase - buffer4Start;
      buffer4StartScale = 1.0f - buffer4EndScale;
    }
    
    float loopScale = 1.0f;
    
    if (hasLoop1Effect) {
      loopSpeed = buffer3Speed;
      if (options[LOOP_SPEED] != 0) {
        effectVal = 0.0f;
        FIND_EFFECT_VAL(LOOP_SPEED, RANGE_INFINITE, DO_NOT_SCALE_LOOP);
        loopSpeed *= pow(2.0f, effectVal * 2.0f);
      }
      
      if (options[LOOP_DEPTH] != 0) {
        effectVal = 1.0f;
        FIND_EFFECT_VAL(LOOP_DEPTH, RANGE_FINITE, DO_NOT_SCALE_LOOP);
        loopScale = effectVal;
      }
    }
    
    waveSpeed = waveBaseSpeed;
    if (options[PITCH] != 0) {
      effectVal = 0.0f;
      FIND_EFFECT_VAL(PITCH, RANGE_INFINITE, DO_SCALE_LOOP);
      waveSpeed *= pow(2.0f, effectVal);
    }
    
    volume = 0.25f;
    if (options[VOLUME] != 0) {
      effectVal = 0.5f;
      FIND_EFFECT_VAL(VOLUME, RANGE_FINITE, DO_SCALE_LOOP);
      volume = effectVal * effectVal;
    }
    float result = ((wave[waveStart] * waveStartScale) + (wave[waveEnd] * waveEndScale)) * volume;
    
    if (options[PITCH_2] != 0) {
      waveSpeed2 = waveBaseSpeed;
      effectVal = 0.0f;
      FIND_EFFECT_VAL(PITCH_2, RANGE_INFINITE, DO_SCALE_LOOP);
      waveSpeed2 *= pow(2.0f, effectVal);
      result += ((wave[waveStart2] * waveStartScale2) + (wave[waveEnd2] * waveEndScale2)) * volume;
    }
    
    float res_ctrl = 0.0f;
    if (options[LOWPASS] != 0) {
      if (options[LOWPASS_PEAK] != 0) {
        effectVal = 1.0f;
        FIND_EFFECT_VAL(LOWPASS_PEAK, RANGE_FINITE, DO_SCALE_LOOP);
        res_ctrl = effectVal * effectVal;
      }
    
      effectVal = 1.0f;
      FIND_EFFECT_VAL(LOWPASS, RANGE_FINITE, DO_SCALE_LOOP);
      
      float lowpass = effectVal * effectVal * effectVal * 0.1f;
      float resonance = 1.0f - (0.05f + lowpass) / (1.0f + res_ctrl * 20.0f);
      prevLowPassDelta = (prevLowPassDelta + (result - prevLowPassOutput) * lowpass) * resonance;
      prevLowPassOutput += prevLowPassDelta;
      result = prevLowPassOutput;
      
      if (options[LOWPASS_PEAK] != 0) {
        // Under EXTREME conditions, the lowpass resonance can freak the fuck out, exploding until
        // it hits NAN and then the tone goes silent. These conditions are:
        // - constant lowpass at least higher than halfway.
        // - constant lowpass resonance at least higher than halfway.
        // - loop 1 lowpass set to a square wave.
        // - loop 1 base speed maxed
        // - constant loop 1 maxed.
        // This results in a very rapidly changing lowpass value and a constant resonance, which,
        // for whatever reason, is fatal. However, we can perform a safety check here to avoid the
        // awful silence effect:
        if (prevLowPassDelta > 1.0f) prevLowPassDelta = 1.0f;
      }
    }
    
    if (options[HIGHPASS] != 0) {
      effectVal = 1.0f;
      FIND_EFFECT_VAL(HIGHPASS, RANGE_FINITE, DO_SCALE_LOOP);
      effectVal = 1.0f - effectVal;
      effectVal *= effectVal;
      float temp = result;
      result = (1.0f - effectVal) * (prevHighPassOutput + result - prevHighPassInput);
      prevHighPassInput = temp;
      prevHighPassOutput = result;
      result *= 1.0f + effectVal;
    }
    
    if (options[DISTORTION] != 0) {
      effectVal = 1.0f;
      FIND_EFFECT_VAL(DISTORTION, RANGE_FINITE, DO_SCALE_LOOP);
      effectVal = effectVal * effectVal * volume;
      if (result > effectVal) result = effectVal;
      else if (-result > effectVal) result = -effectVal;
      /*
      result = tanh(result / (effectVal + 0.001f) ) * effectVal;
      */
    }
    
    
    if (options[PAN] != 0) {
      effectVal = 0.0f;
      FIND_EFFECT_VAL(PAN, RANGE_INFINITE, DO_SCALE_LOOP);
      if (effectVal >= 0) {
        *out++ += result;
        *out++ += result * (1 - effectVal);
      } else {
        *out++ += result * (1 + effectVal);
        *out++ += result;
      }
    } else {
      *out++ += result;
      *out++ += result;
    }
    
    
    
    if (hasAttackEffect) {
      buffer1Phase += buffer1Speed;
    }
    if (hasReleaseEffect && tone->released) {
      buffer2Phase += buffer2Speed;
    }
    
    if (hasReleaseEffect) {
      if (buffer2Phase >= 1.0f) {
        tone->alive = 0;
        break;
      }
      if (hasAttackEffect) {
        if (buffer1Phase >= 1.0f) {
          buffer1Phase = 1.0f;
        }
      }
    } else if (hasAttackEffect) {
      if (buffer1Phase >= 1.0f) {
        buffer1Phase = 1.0f;
        if (tone->released) {
          tone->alive = 0;
          break;
        }
      }
    } else {
      if (tone->released) {
        tone->alive = 0;
        break;
      }
    }
    
    
    
    if (hasLoop1Effect) {
      // Loop 1 might be controlled by a modulator...
      //buffer3Phase += buffer3Speed;
      buffer3Phase += loopSpeed;
      // This was previously an "if" but now it's a "while" in case the combination of 
      // the base loop speed and the modulator cause it to wrap around twice.
      while (buffer3Phase >= EFFECT_LENGTH) buffer3Phase -= EFFECT_LENGTH;
    }
    
    if (hasLoop2Effect) {
      buffer4Phase += buffer4Speed;
      if (buffer4Phase >= EFFECT_LENGTH) buffer4Phase -= EFFECT_LENGTH;
    }
    
    wavePhase += waveSpeed;
    if (wavePhase >= (float) SAMPLES_IN_WAVE) wavePhase -= (float) SAMPLES_IN_WAVE;
    
    if (options[PITCH_2] != 0) {
      wavePhase2 += waveSpeed2;
      if (wavePhase2 >= (float) SAMPLES_IN_WAVE) wavePhase2 -= (float) SAMPLES_IN_WAVE;
    }
  }
  
  tone->age += frames;
  tone->wavePhase = wavePhase;
  if (options[PITCH_2] != 0) tone->wavePhase2 = wavePhase2;
  if (hasAttackEffect) tone->timelinePhases[ATTACK_TIMELINE] = buffer1Phase;
  if (hasReleaseEffect) tone->timelinePhases[RELEASE_TIMELINE] = buffer2Phase;
  if (hasLoop1Effect) tone->timelinePhases[LOOP_1_TIMELINE] = buffer3Phase;
  if (hasLoop2Effect) tone->timelinePhases[LOOP_2_TIMELINE] = buffer4Phase;
  if (options[LOWPASS] != 0) tone->prevLowPassOutput = prevLowPassOutput;
  if (options[LOWPASS] != 0) tone->prevLowPassDelta = prevLowPassDelta;
  if (options[HIGHPASS] != 0) tone->prevHighPassOutput = prevHighPassOutput;
  if (options[HIGHPASS] != 0) tone->prevHighPassInput = prevHighPassInput;
}


#ifdef __cplusplus
}
#endif
