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
#include "keyboard.hpp"
#include <stdio.h>

bool allKeys[128];
std::vector<unsigned char> pressedKeys;
std::vector<unsigned char> releasedKeys;

void keyboard_init() {
  memset (allKeys, 0, 128 * sizeof(bool));
}

char pitchIndexFromKeyCode(long keyCode) {
  //printf("%d\n", keyCode);
  unsigned char pitchIndex = 69 - 9 - 12;
  
  /*
  // base-12 style:
  switch(keyCode) {
    case 96:
      pitchIndex+=12;
      break;
    case 49:
      pitchIndex+=13;
      break;
    case 50:
      pitchIndex+=14;
      break;
    case 51:
      pitchIndex+=15;
      break;
    case 52:
      pitchIndex+=16;
      break;
    case 53:
      pitchIndex+=17;
      break;
    case 54:
      pitchIndex+=18;
      break;
    case 55:
      pitchIndex+=19;
      break;
    case 56:
      pitchIndex+=20;
      break;
    case 57:
      pitchIndex+=21;
      break;
    case 48:
      pitchIndex+=22;
      break;
    case 45:
      pitchIndex+=23;
      break;
    case 61:
      pitchIndex+=24;
      break;

    default:
      pitchIndex = 0;
      break;
  }
  */
  // wicki-hayden style:
  
  switch(keyCode) {
    case 96:
      pitchIndex+=12;
      break;
    case 49:
      pitchIndex+=14;
      break;
    case 50:
      pitchIndex+=16;
      break;
    case 51:
      pitchIndex+=18;
      break;
    case 52:
      pitchIndex+=20;
      break;
    case 53:
      pitchIndex+=22;
      break;
    case 54:
      pitchIndex+=24;
      break;
    case 55:
      pitchIndex+=26;
      break;
    case 56:
      pitchIndex+=28;
      break;
    case 57:
      pitchIndex+=30;
      break;
    case 48:
      pitchIndex+=32;
      break;
    case 45:
      pitchIndex+=34;
      break;
    case 61:
      pitchIndex+=36;
      break;
      
    case 81:
      pitchIndex+=9;
      break;
    case 87:
      pitchIndex+=11;
      break;
    case 69:
      pitchIndex+=13;
      break;
    case 82:
      pitchIndex+=15;
      break;
    case 84:
      pitchIndex+=17;
      break;
    case 89:
      pitchIndex+=19;
      break;
    case 85:
      pitchIndex+=21;
      break;
    case 73:
      pitchIndex+=23;
      break;
    case 79:
      pitchIndex+=25;
      break;
    case 80:
      pitchIndex+=27;
      break;
    case 91:
      pitchIndex+=29;
      break;
    case 93:
      pitchIndex+=31;
      break;
    case 92:
      pitchIndex+=33;
      break;
      
    case 65:
      pitchIndex+=4;
      break;
    case 83:
      pitchIndex+=6;
      break;
    case 68:
      pitchIndex+=8;
      break;
    case 70:
      pitchIndex+=10;
      break;
    case 71:
      pitchIndex+=12;
      break;
    case 72:
      pitchIndex+=14;
      break;
    case 74:
      pitchIndex+=16;
      break;
    case 75:
      pitchIndex+=18;
      break;
    case 76:
      pitchIndex+=20;
      break;
    case 59:
      pitchIndex+=22;
      break;
    case 39:
      pitchIndex+=24;
      break;
      
    case 90:
      pitchIndex+=-1;
      break;
    case 88:
      pitchIndex+=1;
      break;
    case 67:
      pitchIndex+=3;
      break;
    case 86:
      pitchIndex+=5;
      break;
    case 66:
      pitchIndex+=7;
      break;
    case 78:
      pitchIndex+=9;
      break;
    case 77:
      pitchIndex+=11;
      break;
    case 44:
      pitchIndex+=13;
      break;
    case 46:
      pitchIndex+=15;
      break;
    case 47:
      pitchIndex+=17;
      break;

    default:
      pitchIndex = 0;
      break;
  }
  
  // janko style:
  /*
  switch(keyCode) {
    case 96:
      pitchIndex+=3;
      break;
    case 49:
      pitchIndex+=5;
      break;
    case 50:
      pitchIndex+=7;
      break;
    case 51:
      pitchIndex+=9;
      break;
    case 52:
      pitchIndex+=11;
      break;
    case 53:
      pitchIndex+=13;
      break;
    case 54:
      pitchIndex+=15;
      break;
    case 55:
      pitchIndex+=17;
      break;
    case 56:
      pitchIndex+=19;
      break;
    case 57:
      pitchIndex+=21;
      break;
    case 48:
      pitchIndex+=23;
      break;
    case 45:
      pitchIndex+=25;
      break;
    case 61:
      pitchIndex+=27;
      break;
      
    case 81:
      pitchIndex+=6;
      break;
    case 87:
      pitchIndex+=8;
      break;
    case 69:
      pitchIndex+=10;
      break;
    case 82:
      pitchIndex+=12;
      break;
    case 84:
      pitchIndex+=14;
      break;
    case 89:
      pitchIndex+=16;
      break;
    case 85:
      pitchIndex+=18;
      break;
    case 73:
      pitchIndex+=20;
      break;
    case 79:
      pitchIndex+=22;
      break;
    case 80:
      pitchIndex+=24;
      break;
    case 91:
      pitchIndex+=26;
      break;
    case 93:
      pitchIndex+=28;
      break;
    case 92:
      pitchIndex+=30;
      break;
      
    case 65:
      pitchIndex+=7;
      break;
    case 83:
      pitchIndex+=9;
      break;
    case 68:
      pitchIndex+=11;
      break;
    case 70:
      pitchIndex+=13;
      break;
    case 71:
      pitchIndex+=15;
      break;
    case 72:
      pitchIndex+=17;
      break;
    case 74:
      pitchIndex+=19;
      break;
    case 75:
      pitchIndex+=21;
      break;
    case 76:
      pitchIndex+=23;
      break;
    case 59:
      pitchIndex+=25;
      break;
    case 39:
      pitchIndex+=27;
      break;
      
    case 90:
      pitchIndex+=8;
      break;
    case 88:
      pitchIndex+=10;
      break;
    case 67:
      pitchIndex+=12;
      break;
    case 86:
      pitchIndex+=14;
      break;
    case 66:
      pitchIndex+=16;
      break;
    case 78:
      pitchIndex+=18;
      break;
    case 77:
      pitchIndex+=20;
      break;
    case 44:
      pitchIndex+=22;
      break;
    case 46:
      pitchIndex+=24;
      break;
    case 47:
      pitchIndex+=26;
      break;

    default:
      pitchIndex = 0;
      break;
  }
  */
  
  //3x4 octave style:
  /*
  pitchIndex -= 9;
  switch(keyCode) {
    case 96:
      pitchIndex+=5;
      break;
    case 49:
      pitchIndex+=9;
      break;
    case 50:
      pitchIndex+=13;
      break;
    case 51:
      pitchIndex+=17;
      break;
    case 52:
      pitchIndex+=21;
      break;
    case 53:
      pitchIndex+=25;
      break;
    case 54:
      pitchIndex+=29;
      break;
    case 55:
      pitchIndex+=33;
      break;
    case 56:
      pitchIndex+=37;
      break;
    case 57:
      pitchIndex+=41;
      break;
    case 48:
      pitchIndex+=45;
      break;
    case 45:
      pitchIndex+=49;
      break;
    case 61:
      pitchIndex+=53;
      break;
      
    case 81:
      pitchIndex+=10;
      break;
    case 87:
      pitchIndex+=14;
      break;
    case 69:
      pitchIndex+=18;
      break;
    case 82:
      pitchIndex+=22;
      break;
    case 84:
      pitchIndex+=26;
      break;
    case 89:
      pitchIndex+=30;
      break;
    case 85:
      pitchIndex+=34;
      break;
    case 73:
      pitchIndex+=38;
      break;
    case 79:
      pitchIndex+=42;
      break;
    case 80:
      pitchIndex+=46;
      break;
    case 91:
      pitchIndex+=50;
      break;
    case 93:
      pitchIndex+=54;
      break;
    case 92:
      pitchIndex+=58;
      break;
      
    case 65:
      pitchIndex+=11;
      break;
    case 83:
      pitchIndex+=15;
      break;
    case 68:
      pitchIndex+=19;
      break;
    case 70:
      pitchIndex+=23;
      break;
    case 71:
      pitchIndex+=27;
      break;
    case 72:
      pitchIndex+=31;
      break;
    case 74:
      pitchIndex+=35;
      break;
    case 75:
      pitchIndex+=39;
      break;
    case 76:
      pitchIndex+=43;
      break;
    case 59:
      pitchIndex+=47;
      break;
    case 39:
      pitchIndex+=51;
      break;
      
    case 90:
      pitchIndex+=12;
      break;
    case 88:
      pitchIndex+=16;
      break;
    case 67:
      pitchIndex+=20;
      break;
    case 86:
      pitchIndex+=24;
      break;
    case 66:
      pitchIndex+=28;
      break;
    case 78:
      pitchIndex+=32;
      break;
    case 77:
      pitchIndex+=36;
      break;
    case 44:
      pitchIndex+=40;
      break;
    case 46:
      pitchIndex+=44;
      break;
    case 47:
      pitchIndex+=48;
      break;
    default:
      pitchIndex = 0;
      break;
  }
  */
  
  //axis-49 style:
  /*pitchIndex -= 6;
  switch(keyCode) {
    case 96:
      pitchIndex+=5;
      break;
    case 49:
      pitchIndex+=9;
      break;
    case 50:
      pitchIndex+=13;
      break;
    case 51:
      pitchIndex+=17;
      break;
    case 52:
      pitchIndex+=21;
      break;
    case 53:
      pitchIndex+=25;
      break;
    case 54:
      pitchIndex+=29;
      break;
    case 55:
      pitchIndex+=33;
      break;
    case 56:
      pitchIndex+=37;
      break;
    case 57:
      pitchIndex+=41;
      break;
    case 48:
      pitchIndex+=45;
      break;
    case 45:
      pitchIndex+=49;
      break;
    case 61:
      pitchIndex+=53;
      break;
      
    case 81:
      pitchIndex+=6;
      break;
    case 87:
      pitchIndex+=10;
      break;
    case 69:
      pitchIndex+=14;
      break;
    case 82:
      pitchIndex+=18;
      break;
    case 84:
      pitchIndex+=22;
      break;
    case 89:
      pitchIndex+=26;
      break;
    case 85:
      pitchIndex+=30;
      break;
    case 73:
      pitchIndex+=34;
      break;
    case 79:
      pitchIndex+=38;
      break;
    case 80:
      pitchIndex+=42;
      break;
    case 91:
      pitchIndex+=46;
      break;
    case 93:
      pitchIndex+=50;
      break;
    case 92:
      pitchIndex+=54;
      break;
      
    case 65:
      pitchIndex+=3;
      break;
    case 83:
      pitchIndex+=7;
      break;
    case 68:
      pitchIndex+=11;
      break;
    case 70:
      pitchIndex+=15;
      break;
    case 71:
      pitchIndex+=19;
      break;
    case 72:
      pitchIndex+=23;
      break;
    case 74:
      pitchIndex+=27;
      break;
    case 75:
      pitchIndex+=31;
      break;
    case 76:
      pitchIndex+=35;
      break;
    case 59:
      pitchIndex+=39;
      break;
    case 39:
      pitchIndex+=43;
      break;
      
    case 90:
      pitchIndex+=0;
      break;
    case 88:
      pitchIndex+=4;
      break;
    case 67:
      pitchIndex+=8;
      break;
    case 86:
      pitchIndex+=12;
      break;
    case 66:
      pitchIndex+=16;
      break;
    case 78:
      pitchIndex+=20;
      break;
    case 77:
      pitchIndex+=24;
      break;
    case 44:
      pitchIndex+=28;
      break;
    case 46:
      pitchIndex+=32;
      break;
    case 47:
      pitchIndex+=36;
      break;
    default:
      pitchIndex = 0;
      break;
  }*/
  
  // piano style:
  /*
  switch(keyCode) {
    case 81:
      break;
    case 50:
      pitchIndex+=1;
      break;
    case 87:
      pitchIndex+=2;
      break;
    case 51:
      pitchIndex+=3;
      break;
    case 69:
      pitchIndex+=4;
      break;
    case 82:
      pitchIndex+=5;
      break;
    case 53:
      pitchIndex+=6;
      break;
    case 84:
      pitchIndex+=7;
      break;
    case 54:
      pitchIndex+=8;
      break;
    case 89:
      pitchIndex+=9;
      break;
    case 55:
      pitchIndex+=10;
      break;
    case 85:
      pitchIndex+=11;
      break;
    case 73:
    case 90:
      pitchIndex+=12;
      break;
    case 83:
    case 57:
      pitchIndex+=13;
      break;
    case 88:
    case 79:
      pitchIndex+=14;
      break;
    case 68:
    case 48:
      pitchIndex+=15;
      break;
    case 67:
    case 80:
      pitchIndex+=16;
      break;
    case 86:
      pitchIndex+=17;
      break;
    case 71:
      pitchIndex+=18;
      break;
    case 66:
      pitchIndex+=19;
      break;
    case 72:
      pitchIndex+=20;
      break;
    case 78:
      pitchIndex+=21;
      break;
    case 74:
      pitchIndex+=22;
      break;
    case 77:
      pitchIndex+=23;
      break;
    case 44:
      pitchIndex+=24;
      break;
    case 76:
      pitchIndex+=25;
      break;
    case 46:
      pitchIndex+=26;
      break;
    case 59:
      pitchIndex+=27;
      break;
    case 47:
      pitchIndex+=28;
      break;
    default:
      pitchIndex = 0;
      break;
  }
  */
  return pitchIndex;
}
