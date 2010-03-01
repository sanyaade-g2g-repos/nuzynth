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

#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <stdio.h>
#include <vector>
#include <map>
#include <wx/wx.h>
#include "Modulator.h"
#include "Oscillator.h"
#include "Effect.h"
#include "SinglyLinkedList.h"
#include "Pool.h"
#include "Song.h"

extern const char* effectNames[NUM_EFFECT_TYPES];
extern const wxString effectChoices[NUM_EFFECT_TYPES + 1];
extern const bool effectScaleType[NUM_EFFECT_TYPES];
extern const char* voiceNames[NUM_VOICES];
extern const wxString voiceChoices[NUM_VOICES + 1];
extern const char* timelineNames[NUM_TIMELINES];
extern const unsigned char bufferFlags[NUM_TIMELINES];

class Instrument {
public:
  static std::map<LoopOptions, void*> loopFunctions;
  Modulator mod;
  ModulatorSharer sharer;
  Oscillator oscillator;
  std::vector<Effect*> timelines[NUM_TIMELINES];
  int timelineEffectCount[NUM_TIMELINES];
  Song* song;
  
  Instrument(Song* song);
  Instrument(Song* song, FILE* file);
  ~Instrument();
  
  void save(FILE* file);
  void updateWave();
  unsigned char getDepth(int type, int timeline);
  void setDepth(int type, int timeline, unsigned char val);
  unsigned char getSpeed(int timeline);
  void setSpeed(int timeline, unsigned char val);
  int findUnusedEffectType(int timeline);
  Effect* createEffect(int timeline);
  void destroyEffect(Effect* effect);
  void switchEffectType(char timeline, char before, char after);
  void getChoicesForEffect(Effect* effect, bool* flags);
  unsigned char* copyEffectBuffer(int effect, int timeline);
  void replaceEffectBuffer(int effect, int timeline, unsigned char* newBuffer);
  
  static Pool* effectPool();
  static Pool* wavePool();
  static Pool* modulatorPool();
  
  void prepareToDie();
  static void cleanAllDirt();
  void clean();
  void markDirty();
  
private:
  static std::vector<Instrument*> dirtyInstruments;
  static std::vector<Instrument*> deathRow;
  SinglyLinkedList* oldModulators;
  bool dirty;
  
  void setDefaultValues();
  void readCommaSeparatedChars(unsigned char* dest, int maxCount, const char* ascii);
  void setEffectEnabled(int type, int timeline, bool enable);
};

#endif // INSTRUMENT_H
