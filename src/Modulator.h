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

#ifndef MODULATOR_H 
#define MODULATOR_H 

#ifdef __cplusplus
extern "C" {
#endif

#define SUB_OCTAVES                  (2)
#define OCTAVES_IN_PERIOD            (7)
#define SAMPLES_IN_PERIOD            (1 << OCTAVES_IN_PERIOD)
#define EXTRA_OCTAVES_IN_WAVE        (9)
#define PERIODS_IN_WAVE              (1 << EXTRA_OCTAVES_IN_WAVE)
#define PERIODS_IN_WAVE_INVERSE      (1.0f / PERIODS_IN_WAVE)
#define OCTAVES_IN_WAVE              (OCTAVES_IN_PERIOD + EXTRA_OCTAVES_IN_WAVE)
#define SAMPLES_IN_WAVE              (1 << OCTAVES_IN_WAVE)
#define WAVE_MASK                    (SAMPLES_IN_WAVE - 1)
#define EFFECT_LENGTH                (0x40)
#define EFFECT_MASK                  (EFFECT_LENGTH - 1)
#define ALIGNMENT                    (0x20) // Hopefully reasonably similar to cache page size?

#define OPTION_ATTACK       ((unsigned char)(0x01))
#define OPTION_RELEASE      ((unsigned char)(0x02))
#define OPTION_LOOP_1       ((unsigned char)(0x04))
#define OPTION_LOOP_2       ((unsigned char)(0x08))
#define OPTION_CONSTANT     ((unsigned char)(0x10))
/*
#define OPTION_TRACK        ((unsigned char)(0x20))
#define OPTION_NOTE_STATIC  ((unsigned char)(0x40))
#define OPTION_NOTE_DYNAMIC ((unsigned char)(0x80))
*/



#define GET_OPTION(x,y)     ((x & y) != 0)
#define SET_OPTION(x,y,z)   ((unsigned char)(z ? (x | y) : (x & ~y)))

enum {
VOLUME,
HIGHPASS,
LOWPASS,
LOWPASS_PEAK,
PITCH,
PITCH_2,
DISTORTION,
PAN,
LOOP_SPEED,
LOOP_DEPTH,
NUM_EFFECT_TYPES
};

enum {
ATTACK_TIMELINE,
RELEASE_TIMELINE,
LOOP_1_TIMELINE,
LOOP_2_TIMELINE,
CONSTANT_TIMELINE,
NUM_TIMELINES
};

struct Tone;

// For each effect type, for each timeline that could potentially use the 
// effect type, is this combination enabled?
typedef struct SynthOptions {
  unsigned char options[NUM_EFFECT_TYPES];
} SynthOptions;


// SynthOptions are used as a key to access synth functions. The functions
// are stored in a map which uses "<" as a comparison on the keys. (I think
// it's a binary tree?) So let's define the "<" comparison for SynthOptions:
static inline bool operator < (const SynthOptions& left, const SynthOptions& right) {
  int i;
  for (i = 0; i < NUM_EFFECT_TYPES; i++) {
    if (left.options[i] == right.options[i]) continue;
    else if (left.options[i] < right.options[i]) return true;
    else return false;
  }
  return false;
}

typedef struct Modulator {
  unsigned char* buffers[NUM_TIMELINES-1][NUM_EFFECT_TYPES];
  unsigned char depths[NUM_TIMELINES][NUM_EFFECT_TYPES];
  unsigned char speeds[NUM_TIMELINES-1];
  SynthOptions synthOptions;
  float* wave;
  void (*synthFunction)(struct Tone*, struct Modulator*, float*, int);
} Modulator;

#ifdef __cplusplus
}
#endif

#endif // MODULATOR_H 
