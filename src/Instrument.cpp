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
#include <string>
#include <memory>
#include <math.h>

#include <fftw3.h>
#include "tinyxml.h"

#include "random.h"
#include "Instrument.h"
#include "Monitor.h"
#include "memoryBarrier.h"
#include "../loopjit/LoopJIT.h"

std::vector<Instrument*> Instrument::dirtyInstruments;
std::vector<Instrument*> Instrument::deathRow;
std::map<LoopOptions, void*> Instrument::loopFunctions;


const char* effectNames[] = {
  "volume",
  "highpass",
  "lowpass",
  "lowpassPeak",
  "pitch",
  "pitch2",
  "distortion",
  "pan",
  "loopSpeed",
  "loopDepth",
};

const wxString effectChoices[] =
{
  _T("Volume"),
  _T("High Pass"),
  _T("Low Pass"),
  _T("Low Pass Peak"),
  _T("Primary Pitch"),
  _T("Chorus Pitch"),
  _T("Distortion"),
  _T("Left/Right Pan"),
  _T("Loop Speed"),
  _T("Loop Depth"),
  _T("Delete Effect"),
};

const bool effectScaleType[] = 
{
  false,
  false,
  false,
  false,
  true,
  true,
  false,
  true,
  true,
  false,
};


const char* voiceNames[] = {
"voice1",
"voice2",
"voice3",
"voice4",
"voice5",
};

const wxString voiceChoices[] =
{
    _T("Voice 1"),
    _T("Voice 2"),
    _T("Voice 3"),
    _T("Voice 4"),
    _T("Voice 5"),
    _T("Noise"),
};

const char* timelineNames[] = {
"attack",
"release",
"loop1",
"loop2",
"constant",
};

const unsigned char bufferFlags[] = {
OPTION_ATTACK,
OPTION_RELEASE,
OPTION_LOOP_1,
OPTION_LOOP_2,
OPTION_CONSTANT,
};

Instrument::Instrument(Song* song) {
  this->song = song;
  setDefaultValues();
  updateWave();
  clean();
}

Instrument::Instrument(Song* song, FILE* file) {
  this->song = song;
  setDefaultValues();
  
  TiXmlDocument doc;
  doc.LoadFile(file);
  
  TiXmlElement* instrumentXML = doc.FirstChildElement("instrument");
  
  TiXmlElement* oscillatorXML = instrumentXML->FirstChildElement("oscillator");
  TiXmlElement* voiceXML = oscillatorXML->FirstChildElement("voice");
  int voiceNumber = 0;
  for (; voiceXML != 0 && voiceNumber < NUM_VOICES; voiceXML = voiceXML->NextSiblingElement("voice")) {
    HarmonicSet* set = &oscillator.harmonicSet[voiceNumber];
    int temp;
    
    if (voiceXML->QueryIntAttribute("blur", &temp) == TIXML_SUCCESS) {
      set->blur = temp;
    } else {
      set->blur = 0;
    }
    if (voiceXML->QueryIntAttribute("stretch", &temp) == TIXML_SUCCESS) {
      set->stretch = temp;
    } else {
      set->stretch = 0;
    }
    if (voiceXML->QueryIntAttribute("numerator", &temp) == TIXML_SUCCESS) {
      set->numerator = temp;
    } else {
      set->numerator = 1;
    }
    if (voiceXML->QueryIntAttribute("denominator", &temp) == TIXML_SUCCESS) {
      set->denominator = temp;
    } else {
      set->denominator = 1;
    }
    
    const char* amplitudes = voiceXML->Attribute("amplitudes");
    readCommaSeparatedChars(set->amplitudes, MAX_HARMONICS, amplitudes);
    
    voiceNumber++;
  }
  
  TiXmlElement* noiseXML = oscillatorXML->FirstChildElement("noise");
  if (noiseXML != 0) {
    const char* spectrum = noiseXML->Attribute("spectrum");
    readCommaSeparatedChars(oscillator.noise.logSpectrum, NOISE_SPECTRUM_LENGTH, spectrum);
  }
  
  
  TiXmlElement* modulatorXML = instrumentXML->FirstChildElement("modulator");
  TiXmlElement* timelineXML = modulatorXML->FirstChildElement("timeline");
  for (; timelineXML != 0; timelineXML = timelineXML->NextSiblingElement("timeline")) {
    const char* timelineName = timelineXML->Attribute("type");
    int timelineNum = -1;
    for (int i = 0; i < NUM_TIMELINES; i++) {
      if (strcmp(timelineName, timelineNames[i]) == 0) {
        timelineNum = i;
        break;
      }
    }
    if (timelineNum == -1) continue;
    
    bool isConstant = (timelineNum == CONSTANT_TIMELINE);
    if (!isConstant) {
      int speed;
      timelineXML->QueryIntAttribute("speed", &speed);
      mod.speeds[timelineNum] = speed;
    }
    
    TiXmlElement* effectXML = timelineXML->FirstChildElement("effect");
    for (; effectXML != 0; effectXML = effectXML->NextSiblingElement("effect")) {
      const char* effectName = effectXML->Attribute("type");
      int effectNum = -1;
      for (int i = 0; i < NUM_EFFECT_TYPES; i++) {
        if (strcmp(effectName, effectNames[i]) == 0) {
          effectNum = i;
          break;
        }
      }
      if (effectNum == -1) continue;
      
      if (isConstant || effectScaleType[effectNum])
      {
        int depth;
        effectXML->QueryIntAttribute("depth", &depth);
        mod.depths[timelineNum][effectNum] = depth;
      }
      
      if (!isConstant) {
        const char* curveString = effectXML->Attribute("curve");
        unsigned char* buffer = (unsigned char*)Pool_draw(effectPool());
        mod.buffers[timelineNum][effectNum] = buffer;
        readCommaSeparatedChars(buffer, EFFECT_LENGTH, curveString);
      }
      
      Effect* effect = new Effect();
      effect->timeline = timelineNum;
      effect->type = effectNum;
      effect->inst = this;
      timelines[timelineNum].push_back(effect);
      Monitor::setProperty(&timelineEffectCount[timelineNum], timelineEffectCount[timelineNum] + 1);
      
      mod.loopOptions.options[effectNum] |= bufferFlags[timelineNum];
    }
  }
  
  NoiseSpectrum_updateAll(&oscillator.noise);
  updateWave();
  clean();
}

void Instrument::readCommaSeparatedChars(unsigned char* dest, int maxCount, const char* ascii) {
  // the following few lines are based on code by Dave Sinkula:
  // http://www.daniweb.com/code/snippet216535.html
  // you can tell it's not mine due to the single-line c-style comments. :P
  char field [ 32 ];
  int n;
  int i = 0;
  while ( i < maxCount && sscanf(ascii, "%31[^,]%n", field, &n) == 1 ) {
    int val;
    sscanf(field, "%d", &val);
    dest[i] = val;
    ascii += n; /* advance the pointer by the number of characters read */
    if ( *ascii != ',' ) {
      break; /* didn't find an expected delimiter, done? */
    }
    ++ascii; /* skip the delimiter */
    i++;
  }
}

Instrument::~Instrument() {}

void Instrument::setDefaultValues() {
  memset(&mod, 0, sizeof(Modulator));
  memset(&sharer, 0, sizeof(ModulatorSharer));
  mod.speeds[0] = 80;
  mod.speeds[1] = 30;
  mod.speeds[2] = 100;
  mod.speeds[3] = 60;
  mod.wave = 0;
  
  Oscillator_init(&oscillator);
  
  oldModulators = 0;
  sharer.onDeathRow = 0;
  sharer.numReferences = 0;
  
  dirty = true;
}

void Instrument::save(FILE* file) {
  TiXmlDocument doc;
  
  TiXmlElement instrumentXML("instrument");
  
  TiXmlElement oscillatorXML("oscillator");
  for (int voiceNumber = 0; voiceNumber < NUM_VOICES; voiceNumber++) {
    TiXmlElement voiceXML("voice");
    
    HarmonicSet* set = &oscillator.harmonicSet[voiceNumber];
    
    voiceXML.SetAttribute("blur", set->blur);
    voiceXML.SetAttribute("stretch", set->stretch);
    voiceXML.SetAttribute("numerator", set->numerator);
    voiceXML.SetAttribute("denominator", set->denominator);
    
    char shortString[10];
    std::string ampString;
    for (int k = 0; k < MAX_HARMONICS; k++) {
      sprintf(shortString, "%d", set->amplitudes[k]);
      ampString += shortString;
      ampString += ",";
    }
    voiceXML.SetAttribute("amplitudes", ampString.c_str());
    
    oscillatorXML.InsertEndChild(voiceXML);
  }
  
  TiXmlElement noiseXML("noise");
  char shortString2[10];
  std::string noiseString;
  for (int k = 0; k < NOISE_SPECTRUM_LENGTH; k++) {
    sprintf(shortString2, "%d", oscillator.noise.logSpectrum[k]);
    noiseString += shortString2;
    noiseString += ",";
  }
  noiseXML.SetAttribute("spectrum", noiseString.c_str());
  oscillatorXML.InsertEndChild(noiseXML);
  
  instrumentXML.InsertEndChild(oscillatorXML);
  
  
  TiXmlElement modulatorXML("modulator");
  
  for (int timeline = 0; timeline < NUM_TIMELINES; timeline++) {
    TiXmlElement timelineXML("timeline");
    timelineXML.SetAttribute("type", timelineNames[timeline]);
    if (timeline != CONSTANT_TIMELINE) timelineXML.SetAttribute("speed", mod.speeds[timeline]);
    std::vector<Effect*>::iterator iter = timelines[timeline].begin();
    for (; iter != timelines[timeline].end(); iter++) {
      Effect* effect = *iter;
      TiXmlElement effectXML("effect");
      effectXML.SetAttribute("type", effectNames[effect->type]);
      
      if (timeline == (int) CONSTANT_TIMELINE || effectScaleType[effect->type])
        effectXML.SetAttribute("depth", mod.depths[effect->timeline][effect->type]);
      
      if (timeline != (int) CONSTANT_TIMELINE) {
        unsigned char* buffer = mod.buffers[effect->timeline][effect->type];
        std::string bufferString;
        char shortString[10];
        for (int k = 0; k < EFFECT_LENGTH; k++) {
          sprintf(shortString, "%d", buffer[k]);
          bufferString += shortString;
          bufferString += ",";
        }
        effectXML.SetAttribute("curve", bufferString.c_str());
      }
      
      timelineXML.InsertEndChild(effectXML);
    }
    modulatorXML.InsertEndChild(timelineXML);
  }
  
  instrumentXML.InsertEndChild(modulatorXML);
  
  doc.InsertEndChild(instrumentXML);
  
  // How should I save the file?
  //doc->SaveFile(file);
  doc.Print(file);
  // *file << *doc;
}

void Instrument::prepareToDie() {
  markDirty();
  mod.wave = 0; // this is not a memory leak because it will be compared against the previous modulator's wave.
  for (int timeline = 0; timeline < NUM_TIMELINES; timeline++) {
    while (timelines[timeline].size() > 0) {
      destroyEffect(timelines[timeline].back());
    }
  }
  
  deathRow.push_back(this);
  PaUtil_WriteMemoryBarrier();
  sharer.onDeathRow = 1;
}

void Instrument::cleanAllDirt() {
  while (dirtyInstruments.size() > 0) {
    //printf("cleaning up instrument\n");
    Instrument* inst = dirtyInstruments.back();
    dirtyInstruments.pop_back();
    inst->clean();
  }
  while (deathRow.size() > 0) {
    printf("killing death row member\n");
    Instrument* inst = deathRow.back();
    // TODO: Make sure this is thread safe:
    if (inst->sharer.numReferences == 0) {
      deathRow.pop_back();
      delete inst;
    } else {
      break; // TODO: skip and try the next one.
    }
  }
}

void Instrument::clean() {
  if (oscillator.dirty) {
    updateWave();
  }
  
  void* loopPointer;
  if (loopFunctions.find(mod.loopOptions) != loopFunctions.end()) {
    loopPointer = loopFunctions[mod.loopOptions];
  } else {
    loopPointer = RunLoopJIT(mod.loopOptions);
    loopFunctions[mod.loopOptions] = loopPointer;
  }
  mod.loopFunction = (void (*)(struct Tone*, struct Modulator*, float*, int)) loopPointer;
  
  Modulator* copy = (Modulator*) Pool_draw(modulatorPool());
  memcpy(copy, &mod, sizeof(Modulator));
  
  if (sharer.useMod1) {
    sharer.mod2 = copy;
    PaUtil_WriteMemoryBarrier();
    sharer.useMod1 = 0;
  } else {
    sharer.mod1 = copy;
    PaUtil_WriteMemoryBarrier();
    sharer.useMod1 = 1;
  }
  
  SinglyLinkedList *l1, *l2;
  l1 = (SinglyLinkedList*) malloc(sizeof(SinglyLinkedList));
  l1->val = copy;
  l1->next = oldModulators;
  oldModulators = l1;
  
  Modulator *t1, *t2;
  while (l1 != 0) {
    t1 = (Modulator*) l1->val;
    if (t1->used) {
      // all Modulators following this one are unused. 
      
      // first pass: free any unused buffers after l1.
      l2 = l1->next;
      while (l2 != 0) {
        t2 = (Modulator*) l2->val;
        // any buffers used by t2 but not t1 can be returned to the pool.
        for (int j = 0; j < NUM_TIMELINES-1; j++) {
          for (int i = 0; i < NUM_EFFECT_TYPES; i++) {
            if (t1->buffers[j][i] != t2->buffers[j][i] && t2->buffers[j][i] != 0) {
              Pool_return(effectPool(), t2->buffers[j][i]);
            }
          }
        }
        if (t1->wave != t2->wave) {
          //fftwf_free(t2->wave);
          Pool_return(wavePool(), t2->wave);
        }
        // This would also be a good place to free any unused loop functions, except I'm 
        // planning on saving those in a map indefinitely...
        t1 = t2;
        l2 = l2->next;
      }
      
      // second pass: truncate the linked list after l1.
      l2 = l1->next;
      l1->next = 0;
      while (l2 != 0) {
        l1 = l2;
        l2 = l2->next;
        Pool_return(modulatorPool(), l1->val);
        free(l1);
      }
      
      break;
    }
    l1 = l1->next;
  }
  
  dirty = false;
}

void Instrument::updateWave()
{
  float* wave = mod.wave;
  //if ( (!dirty) || wave == 0 ) {
    markDirty();
    wave = (float*) fftwf_malloc(sizeof(float) * SAMPLES_IN_WAVE);
    wave = (float*) Pool_draw(wavePool());
  //}
  //memset(wave, 0, sizeof(float) * SAMPLES_IN_WAVE);
  
  Oscillator_renderWave(&oscillator, wave);
  
  Monitor::setProperty(&mod.wave, wave);
}

Pool* Instrument::effectPool() {
  static Pool* pool = 0;
  if (pool == 0) {
    pool = (Pool*)malloc(sizeof(Pool));
    Pool_init(pool, EFFECT_LENGTH * sizeof(char), true);
  }
  return pool;
}
Pool* Instrument::wavePool() {
  static Pool* pool = 0;
  if (pool == 0) {
    pool = (Pool*)malloc(sizeof(Pool));
    Pool_init(pool, SAMPLES_IN_WAVE * sizeof(float), true);
  }
  return pool;
}
Pool* Instrument::modulatorPool() {
  static Pool* pool = 0;
  if (pool == 0) {
    pool = (Pool*)malloc(sizeof(Pool));
    Pool_init(pool, sizeof(Modulator), false);
  }
  return pool;
}

void Instrument::draw(int effect, int timeline, int prevIndex, int nextIndex, float value) {
  unsigned char* buffer;
  unsigned char* oldBuffer = mod.buffers[timeline][effect];
  if (!dirty) {
    markDirty();
    buffer = (unsigned char*) Pool_draw(effectPool());
    memcpy(buffer, oldBuffer, EFFECT_LENGTH * sizeof(unsigned char));
    Monitor::setProperty(&mod.buffers[timeline][effect], buffer);
  } else {
    // TODO: If instument is dirty for some reason other than previous drawing, this 
    // will end up drawing on a buffer that is in use by the audio callback!
    buffer = oldBuffer;
  }
  
  buffer[nextIndex] = (unsigned char)(value * 254.0f);
  if (prevIndex == nextIndex) {
    return;
  }
  
  int leftIndex, rightIndex;
  float leftValue, rightValue, delta;
  if (prevIndex < nextIndex) {
    leftIndex = prevIndex;
    rightIndex = nextIndex;
  } else {
    leftIndex = nextIndex;
    rightIndex = prevIndex;
  }
  leftValue = (float)buffer[leftIndex] * (1.0f / 254.0f);
  rightValue = (float)buffer[rightIndex] * (1.0f / 254.0f);
  delta = (rightValue - leftValue) / (float)(rightIndex - leftIndex);
  for (int i = leftIndex + 1; i < rightIndex; i++) {
    leftValue += delta;
    buffer[i] = (unsigned char)(leftValue * 254.0f);
  }
}

void Instrument::markDirty() {
  if (!dirty) {
    dirty = true;
    dirtyInstruments.push_back(this);
  }
}

unsigned char Instrument::getDepth(int type, int timeline) {
  return mod.depths[timeline][type];
}

void Instrument::setDepth(int type, int timeline, unsigned char val) {
  if (mod.depths[timeline][type] == val) return;
  Monitor::setProperty(&mod.depths[timeline][type], val);
  markDirty();
}

unsigned char Instrument::getSpeed(int timeline) {
  return mod.speeds[timeline];
}

void Instrument::setSpeed(int timeline, unsigned char val) {
  if (mod.speeds[timeline] == val) return;
  markDirty();
  Monitor::setProperty(&mod.speeds[timeline], val);
}

void Instrument::setEffectEnabled(int type, int timeline, bool enable) {
  unsigned char flag = bufferFlags[timeline];
  bool wasEnabled = GET_OPTION(mod.loopOptions.options[type], flag);
  if (enable == wasEnabled) return;
  
  markDirty();
  Monitor::setProperty(&mod.loopOptions.options[type], 
             SET_OPTION(mod.loopOptions.options[type], flag, enable));
  
  if (timeline == CONSTANT_TIMELINE) return; // don't create a buffer for constant effect.
  
  unsigned char** bufferPtr = &mod.buffers[timeline][type];
  if (enable) {
    unsigned char* newBuffer = (unsigned char*) Pool_draw(effectPool());
    
    float val;
    switch (timeline) {
      case 0:
        for (int i = 0; i < EFFECT_LENGTH; i++) {
          val = (i) / (EFFECT_LENGTH - 1.0f);
          newBuffer[i] = 254.0f * val;
        }
        break;
      case 1:
        for (int i = 0; i < EFFECT_LENGTH; i++) {
          val = (EFFECT_LENGTH - i - 1.0f) / (EFFECT_LENGTH - 1.0f);
          newBuffer[i] = 254.0f * val;
        }
        break;
      case 2:
      case 3:
        for (int i = 0; i < EFFECT_LENGTH; i++) {
          if (i < EFFECT_LENGTH / 2) val = (i) / (EFFECT_LENGTH - 1.0f) * 2.0f;
          else                         val = (EFFECT_LENGTH - i - 1.0f) / (EFFECT_LENGTH - 1.0f) * 2.0f;
          newBuffer[i] = 254.0f * val;
        }
        break;
    }
    
    Monitor::setProperty(bufferPtr, newBuffer);
  } else {
    Monitor::setProperty(bufferPtr, (unsigned char*)0);
  }
}

int Instrument::findUnusedEffectType(int timeline) {
  for (int i = 0; i < NUM_EFFECT_TYPES; i++) {
    if ((mod.loopOptions.options[i] & bufferFlags[timeline]) == 0) {
      return i;
    }
  }
  return -1;
}

Effect* Instrument::createEffect(int timeline) {
  Effect* effect = new Effect();
  effect->inst = this;
  effect->timeline = timeline;
  effect->type = findUnusedEffectType(timeline);
  timelines[timeline].push_back(effect);
  Monitor::setProperty(&timelineEffectCount[timeline], timelineEffectCount[timeline] + 1);
  setEffectEnabled(effect->type, effect->timeline, true);
  setDepth(effect->type, effect->timeline, 127);
  
  return effect;
}

void Instrument::destroyEffect(Effect* effect) {
  std::vector<Effect*>::iterator iter = timelines[effect->timeline].begin();
  for (; iter != timelines[effect->timeline].end(); iter++) {
    if (*iter == effect) {
      Monitor::setProperty(&timelineEffectCount[effect->timeline], timelineEffectCount[effect->timeline] - 1);
      timelines[effect->timeline].erase(iter);
      break;
    }
  }
  setEffectEnabled(effect->type, effect->timeline, false);
  delete effect;
}

void Instrument::switchEffectType(Effect* effect, char type) {
  // TODO: consider rewriting this so that you don't need to copy 
  // to a local buffer and then copy back.
  unsigned char buffer[EFFECT_LENGTH];
  unsigned char oldDepth = getDepth(effect->type, effect->timeline);
  
  std::vector<Effect*>::iterator iter = timelines[effect->timeline].begin();
  
  unsigned char** buffers = mod.buffers[effect->timeline];
  for (; iter != timelines[effect->timeline].end(); iter++) {
    Effect* other = *iter;
    if (other != effect && other->type == type) {
      setDepth(effect->type, effect->timeline, getDepth(type, effect->timeline));
      setDepth(type, effect->timeline, oldDepth);
      
      if (effect->timeline != (int)CONSTANT_TIMELINE) {
        memcpy(buffer, buffers[effect->type], sizeof(unsigned char) * EFFECT_LENGTH);
        memcpy(buffers[effect->type], buffers[other->type], sizeof(unsigned char) * EFFECT_LENGTH);
        memcpy(buffers[other->type], buffer, sizeof(unsigned char) * EFFECT_LENGTH);
      }
      
      Monitor::setProperty(&other->type, effect->type);
      Monitor::setProperty(&effect->type, type);
      return;
    }
  }
  
  if (effect->timeline != (int)CONSTANT_TIMELINE) {
    memcpy(buffer, mod.buffers[effect->timeline][effect->type], sizeof(unsigned char) * EFFECT_LENGTH);
  }
  
  setDepth(type, effect->timeline, oldDepth);
  
  setEffectEnabled(effect->type, effect->timeline, false);
  Monitor::setProperty(&effect->type, type);
  setEffectEnabled(effect->type, effect->timeline, true);
  
  if (effect->timeline != (int)CONSTANT_TIMELINE) {
    memcpy(mod.buffers[effect->timeline][effect->type], buffer, sizeof(unsigned char) * EFFECT_LENGTH);
  }
}
