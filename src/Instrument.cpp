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
#include "../synthjit/SynthJIT.h"

std::map<SynthOptions, void*> Instrument::synthFunctions;


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
  updateClone();
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
      sharedData->speeds[timelineNum] = speed;
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
        sharedData->depths[timelineNum][effectNum] = depth;
      }
      
      if (!isConstant) {
        const char* curveString = effectXML->Attribute("curve");
        unsigned char* buffer = (unsigned char*)Pool_draw(effectPool());
        sharedData->buffers[timelineNum][effectNum] = buffer;
        readCommaSeparatedChars(buffer, EFFECT_LENGTH, curveString);
      }
      
      Effect* effect = new Effect();
      effect->timeline = timelineNum;
      effect->type = effectNum;
      effect->inst = this;
      timelines[timelineNum].push_back(effect);
      Monitor::setProperty(&timelineEffectCount[timelineNum], timelineEffectCount[timelineNum] + 1);
      
      sharedData->synthOptions.options[effectNum] |= bufferFlags[timelineNum];
    }
  }
  
  NoiseSpectrum_updateAll(&oscillator.noise);
  updateWave();
  updateClone();
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
  sharedData = (Modulator*) Pool_draw(sharedPool());
  memset(sharedData, 0, sizeof(Modulator));
  
  sharedData->speeds[0] = 80;
  sharedData->speeds[1] = 30;
  sharedData->speeds[2] = 100;
  sharedData->speeds[3] = 60;
  sharedData->wave = 0;
  
  Oscillator_init(&oscillator);
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
    if (timeline != CONSTANT_TIMELINE) timelineXML.SetAttribute("speed", sharedData->speeds[timeline]);
    std::vector<Effect*>::iterator iter = timelines[timeline].begin();
    for (; iter != timelines[timeline].end(); iter++) {
      Effect* effect = *iter;
      TiXmlElement effectXML("effect");
      effectXML.SetAttribute("type", effectNames[effect->type]);
      
      if (timeline == (int) CONSTANT_TIMELINE || effectScaleType[effect->type])
        effectXML.SetAttribute("depth", sharedData->depths[effect->timeline][effect->type]);
      
      if (timeline != (int) CONSTANT_TIMELINE) {
        unsigned char* buffer = sharedData->buffers[effect->timeline][effect->type];
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
  markSharedDataDirty();
  sharedData->wave = 0; /// this is not a memory leak because it will be compared against the previous modulator's wave.
  for (int timeline = 0; timeline < NUM_TIMELINES; timeline++) {
    while (timelines[timeline].size() > 0) {
      destroyEffect(timelines[timeline].back());
    }
  }
  
  condemn();
}

void Instrument::cleanSharedData() {
  printf("Instrument::cleanSharedData()\n");
  if (oscillator.dirty) {
    updateWave();
  }
  
  void* synthPointer;
  if (synthFunctions.find(sharedData->synthOptions) != synthFunctions.end()) {
    synthPointer = synthFunctions[sharedData->synthOptions];
  } else {
    synthPointer = RunSynthJIT(sharedData->synthOptions);
    synthFunctions[sharedData->synthOptions] = synthPointer;
  }
  sharedData->synthFunction = (void (*)(struct Tone*, struct Modulator*, float*, int)) synthPointer;
}

void Instrument::destroyOldClone(Modulator* newClone, Modulator* oldClone) {
  printf("Instrument::destroyOldClone(newClone, oldClone)\n");
  // any buffers used by oldClone but not newClone can be returned to the pool.
  for (int j = 0; j < NUM_TIMELINES-1; j++) {
    for (int i = 0; i < NUM_EFFECT_TYPES; i++) {
      if (oldClone->buffers[j][i] != newClone->buffers[j][i] &&
          oldClone->buffers[j][i] != 0)
      {
        Pool_return(effectPool(), oldClone->buffers[j][i]);
      }
    }
  }
  if (newClone->wave != oldClone->wave) {
    /// TODO: Make sure this is okay. :)
    //fftwf_free(oldClone->wave);
    Pool_return(wavePool(), oldClone->wave);
  }
}

void Instrument::updateWave()
{
  float* wave = sharedData->wave;
  markSharedDataDirty();
  //wave = (float*) fftwf_malloc(sizeof(float) * SAMPLES_IN_WAVE);
  
  wave = (float*) Pool_draw(wavePool());
  
  Oscillator_renderWave(&oscillator, wave);
  
  Monitor::setProperty(&sharedData->wave, wave);
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

unsigned char* Instrument::copyEffectBuffer(int effect, int timeline) {
  unsigned char* buffer = (unsigned char*) Pool_draw(effectPool());
  unsigned char* oldBuffer = sharedData->buffers[timeline][effect];
  memcpy(buffer, oldBuffer, EFFECT_LENGTH * sizeof(unsigned char));
  return buffer;
}

void Instrument::replaceEffectBuffer(int effect, int timeline, unsigned char* newBuffer) {
  Monitor::setProperty(&sharedData->buffers[timeline][effect], newBuffer);
  markSharedDataDirty();
}

unsigned char Instrument::getDepth(int type, int timeline) {
  return sharedData->depths[timeline][type];
}

void Instrument::setDepth(int type, int timeline, unsigned char val) {
  if (sharedData->depths[timeline][type] == val) return;
  Monitor::setProperty(&sharedData->depths[timeline][type], val);
  markSharedDataDirty();
}

unsigned char Instrument::getSpeed(int timeline) {
  return sharedData->speeds[timeline];
}

void Instrument::setSpeed(int timeline, unsigned char val) {
  if (sharedData->speeds[timeline] == val) return;
  markSharedDataDirty();
  Monitor::setProperty(&sharedData->speeds[timeline], val);
}

void Instrument::setEffectEnabled(int type, int timeline, bool enable) {
  unsigned char flag = bufferFlags[timeline];
  bool wasEnabled = GET_OPTION(sharedData->synthOptions.options[type], flag);
  if (enable == wasEnabled) return;
  
  markSharedDataDirty();
  Monitor::setProperty(&sharedData->synthOptions.options[type], 
             SET_OPTION(sharedData->synthOptions.options[type], flag, enable));
  
  if (timeline == CONSTANT_TIMELINE) return; // don't create a buffer for constant effect.
  
  unsigned char** bufferPtr = &sharedData->buffers[timeline][type];
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
    if ((sharedData->synthOptions.options[i] & bufferFlags[timeline]) == 0) {
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
  setEffectEnabled(effect->type, effect->timeline, true);
  setDepth(effect->type, effect->timeline, 127);
  Monitor::setProperty(&timelineEffectCount[timeline], timelineEffectCount[timeline] + 1);
  
  return effect;
}

void Instrument::destroyEffect(Effect* effect) {
  std::vector<Effect*>::iterator iter = timelines[effect->timeline].begin();
  for (; iter != timelines[effect->timeline].end(); iter++) {
    if (*iter == effect) {
      timelines[effect->timeline].erase(iter);
      Monitor::setProperty(&timelineEffectCount[effect->timeline], timelineEffectCount[effect->timeline] - 1);
      setEffectEnabled(effect->type, effect->timeline, false);
      delete effect;
      return;
    }
  }
}

void Instrument::switchEffectType(char timeline, char before, char after) {
  /// TODO: consider rewriting this so that you don't need to copy 
  /// to a local buffer and then copy back.
  unsigned char buffer[EFFECT_LENGTH];
  unsigned char oldDepth = getDepth(before, timeline);
  
  std::vector<Effect*>::iterator iter = timelines[timeline].begin();
  
  Effect* effect = 0;
  Effect* other = 0;
  unsigned char** buffers = sharedData->buffers[timeline];
  for (; iter != timelines[timeline].end(); iter++) {
    if ((*iter)->type == before) effect = *iter;
    if ((*iter)->type == after) other = *iter;
    
    if (effect != 0 && other != 0 && effect != other) {
      setDepth(before, timeline, getDepth(after, timeline));
      setDepth(after, timeline, oldDepth);
      
      if (timeline != (int)CONSTANT_TIMELINE) {
        memcpy(buffer, buffers[before], sizeof(unsigned char) * EFFECT_LENGTH);
        memcpy(buffers[before], buffers[after], sizeof(unsigned char) * EFFECT_LENGTH);
        memcpy(buffers[after], buffer, sizeof(unsigned char) * EFFECT_LENGTH);
      }
      
      Monitor::setProperty(&other->type, before);
      Monitor::setProperty(&effect->type, after);
      return;
    }
  }
  
  if (timeline != (int)CONSTANT_TIMELINE) {
    memcpy(buffer, sharedData->buffers[timeline][before], sizeof(unsigned char) * EFFECT_LENGTH);
  }
  
  setDepth(after, timeline, oldDepth);
  
  setEffectEnabled(before, timeline, false);
  Monitor::setProperty(&effect->type, after);
  setEffectEnabled(after, timeline, true);
  
  if (timeline != (int)CONSTANT_TIMELINE) {
    memcpy(sharedData->buffers[timeline][after], buffer, sizeof(unsigned char) * EFFECT_LENGTH);
  }
}
