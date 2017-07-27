/*----------------------------------------------------------------
|  Author: Luis Dominguez - LADBSoft.com                          |
|  Date: 25/07/2017                                 Version: 1.0b |
|-----------------------------------------------------------------|
|  Name: FireBuino!                                               |
|  Description: Remake of the classic Game&Watch Fire, from 1980. |
|----------------------------------------------------------------*/

#include <Gamebuino.h>
#include <EEPROM.h>

class Survivor {
    public:
      short _floor;
      short _step;
      short _delay;
      bool _bounced;
      bool _dead;

      Survivor(short floor, short delay) {
        _floor = floor;
        if(floor == 0) {
          _step = 0;
        } else if(floor == 1) {
          _step = 1;
        } else if(floor == 2) {
          _step = 2;
        }
        _delay = delay;
        _bounced = false;
        _dead = false;
      }
};

Gamebuino gb;
long score;
long highscore;
short lives;
short playerPosition;
short moveTick;
short spawnDelay;
Survivor *survivors[10]; //max. 10 survivors at the same time
short noOfSurvivors;
short randNo;

const byte titleScreenBitmap[] PROGMEM = {64,30,
B11100000,B00000000,B00000000,B00000111,B10001110,B01111000,B11100000,B01100000,
B11100000,B00000000,B00000000,B00001111,B11101100,B00001101,B11111001,B11110000,
B11100000,B00000000,B00000000,B00011111,B11111000,B00001111,B00011111,B10011000,
B11111100,B00000000,B00000000,B00011000,B00011000,B00000110,B00001110,B00001100,
B11100011,B11000000,B00000000,B01111000,B00010000,B00000100,B00000100,B00001101,
B11100010,B00111000,B00000000,B11101100,B00000111,B00000011,B10000000,B00000111,
B11100010,B00001001,B10000001,B10000000,B00011111,B11001111,B11000001,B11000001,
B11100010,B00001111,B00010001,B00000000,B00011000,B11011000,B11100111,B11110000,
B11100010,B00001110,B11000001,B00000110,B00110000,B01100000,B00111110,B01111000,
B11100010,B00001111,B11000001,B10001111,B00110000,B01000000,B00111100,B00111000,
B11100010,B00001111,B10000000,B11011001,B11100000,B00000000,B00011000,B00011100,
B11100010,B00001111,B10000000,B00010000,B10000000,B00110000,B00000000,B00100111,
B11111010,B00001111,B00000000,B00000000,B00000000,B01110000,B00000000,B00000001,
B11111110,B00001111,B00000000,B00000000,B01110000,B01110000,B00000000,B00000000,
B11111111,B11001110,B00000000,B01100000,B11110000,B01111000,B00000000,B00000000,
B11111111,B11111010,B00000000,B01000000,B11100000,B01100001,B10000000,B00000001,
B11111111,B11111000,B00000001,B01000000,B01111000,B11100111,B11100000,B00000001,
B11111111,B11111000,B00000011,B11000000,B00001100,B10001111,B11110000,B00001011,
B11111111,B11111001,B00000011,B11000000,B00000100,B10011111,B11111000,B00001110,
B11111111,B11111010,B10000111,B11001000,B00000111,B11111110,B11111000,B00011110,
B11111111,B11111111,B10000111,B10001000,B00100111,B11111100,B01111000,B00100011,
B11111111,B11111111,B11001011,B00111000,B00000011,B11101100,B01110000,B00000001,
B11100111,B11111111,B11111000,B11011000,B00000011,B00101110,B01100000,B00000111,
B11100011,B11111111,B11111000,B11000010,B00000001,B00100110,B11110100,B01111110,
B11100000,B11111111,B11111111,B01100000,B10000011,B00110000,B01110011,B11100000,
B11100000,B01111000,B11111111,B11111111,B10000110,B00011110,B00000000,B00000000,
B11100000,B00000000,B00000111,B11111111,B00011100,B00011100,B00000000,B00000000,
B11100000,B00000000,B00000001,B11111000,B00011110,B00001110,B00000000,B00000000,
B11100000,B00000000,B00000000,B00000000,B00001111,B00000110,B00000000,B00000000,
B11100000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
};

const byte subBackgroundBitmap[] PROGMEM = {88,48,
B00000000,B01000000,B00100001,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10100000,B01000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10010000,B01000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000001,B00110000,B00000010,B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00101001,B00000000,B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10001000,B00000010,B00000100,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00001010,B00000100,B00000100,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00010010,B00000100,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B01100010,B00001000,B00001000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00001000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00000000,B00100000,B00001000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B01000000,B01000000,B00010000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10100000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000001,B00101000,B10000000,B00100000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000001,B00010100,B00000000,B00100000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00000100,B10000000,B01000000,B00000000,B00000000,B00000000,B00111110,B11100000,B00000000,B00000000,
B00000000,B01000100,B10000000,B01000000,B00000000,B00000000,B00000000,B11000001,B00010000,B00000000,B00000000,
B00000000,B01001000,B00000011,B00000000,B00000000,B00000000,B00000111,B00000000,B10001000,B00000000,B00000000,
B00000000,B00010000,B00011100,B00000000,B00000000,B00000000,B00001000,B10000000,B10001000,B00000000,B00000000,
B00000000,B00010000,B01000000,B00000000,B00000000,B00000000,B00001000,B00000100,B00000100,B00000000,B00000000,
B00000000,B00100000,B10000000,B00000000,B00000000,B00000000,B00001000,B00001000,B00000100,B00000000,B00000000,
B00000000,B01000001,B00000000,B00000000,B00000000,B00000000,B00001000,B00001000,B00000100,B00000000,B00000000,
B00000000,B00000001,B00000000,B00000000,B00000000,B00000000,B00000100,B00001000,B00001000,B00000000,B00000000,
B00000000,B00010000,B00000000,B00000000,B00000000,B00000000,B00001011,B00001000,B00001000,B00000000,B00000000,
B00000000,B10101000,B00000000,B00000000,B00000000,B00000000,B00010000,B01000100,B10010100,B00000000,B00000000,
B00000001,B01001000,B00000000,B00000000,B00000000,B00000000,B00010000,B10000000,B01000010,B00000000,B00000000,
B00000000,B00001000,B00000000,B00000000,B00000000,B00000000,B00010000,B10000000,B01000010,B00000000,B00000000,
B00000000,B00001000,B00000000,B00000000,B00000000,B00000000,B00001000,B01000000,B10000010,B00000000,B00000000,
B00000000,B10010000,B00000000,B00000000,B00000000,B00000000,B00001000,B00001000,B00000010,B00000000,B00000000,
B00000001,B00100000,B00000000,B00000000,B00000000,B00000000,B00000100,B01001100,B10000100,B00000000,B00000000,
B00000000,B00100000,B00000000,B00000000,B00000000,B00000000,B00000011,B00100001,B00001000,B00000000,B00000000,
B00000000,B00100000,B00000000,B00000000,B00000000,B00000000,B00000000,B10010010,B00110000,B00000000,B00000000,
B00000000,B01000000,B00000000,B00000000,B00000000,B00000000,B00000000,B01110011,B11000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00010010,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00010010,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00010010,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00010010,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00010001,B00000000,B00000000,B00000000,
B00000000,B00000010,B00000001,B00000000,B01000000,B00000001,B00000000,B00010001,B00000000,B00000000,B00000000,
B00000000,B00001010,B00000001,B10000000,B01010000,B00000001,B01000000,B00010001,B00000000,B00000000,B00000000,
B00000000,B00101100,B00000011,B00000000,B00100000,B00000010,B01000000,B00010001,B00000000,B00000000,B00000000,
B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B00001000,B00000000,B00000010,B00000000,B00000000,B01000000,B00000000,B00010000,B00000000,B00000000,
B00000000,B00010000,B00000000,B00000010,B00000000,B00000000,B01000000,B00000000,B00001000,B00000000,B00000000,
B00000000,B00010000,B00000000,B00000100,B00000000,B00000000,B00100000,B00000000,B00001000,B00000000,B00000000,
B00000000,B00100000,B00000000,B00000100,B00000000,B00000000,B00100000,B00000000,B00000100,B00000000,B00000000,
B00000000,B00100000,B00000000,B00000100,B00000000,B00000000,B00100000,B00000000,B00000100,B00000000,B00000000,
B00000000,B01000000,B00000000,B00001000,B00000000,B00000000,B00010000,B00000000,B00000010,B00000000,B00000000,
};

const byte backgroundBitmap[] PROGMEM = {88,48,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B11111111,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B11111111,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B11111111,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B11111111,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B11111111,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000000,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B11111111,B10000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000010,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,
B00000011,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11110000,
B00000100,B00001000,B00000000,B00000010,B00000000,B00000000,B01000000,B00000000,B00010000,B00000000,B00000000,
B00001000,B00010000,B00000000,B00000010,B00000000,B00000000,B01000000,B00000000,B00001000,B00000000,B00000000,
B00010000,B00010000,B00000000,B00000100,B00000000,B00000000,B00100000,B00000000,B00001000,B00000000,B00000000,
B00100000,B00100000,B00000000,B00000100,B00000000,B00000000,B00100000,B00000000,B00000100,B00000000,B00000000,
B01000000,B00100000,B00000000,B00000100,B00000000,B00000000,B00100000,B00000000,B00000100,B00000000,B00000000,
B10000000,B01000000,B00000000,B00001000,B00000000,B00000000,B00010000,B00000000,B00000010,B00000000,B00000000,
};

const byte livesBitmap[] PROGMEM = {8,6,
B00110000,
B00110000,
B11111100,
B11111100,
B00110000,
B00110000,
};

const byte ambulanceBitmap[] PROGMEM = {24,16,
B00000000,B01100000,B00000000,
B00000000,B10010000,B00000000,
B00011111,B11111000,B00000000,
B00100000,B00000100,B00000000,
B01001111,B10011110,B00000000,
B01001111,B10011110,B00000000,
B01001111,B10011111,B00000000,
B01000000,B00000000,B10000000,
B10000000,B11000000,B01000000,
B10000001,B11100000,B11000000,
B10000001,B11100000,B11000000,
B10000000,B11000000,B01000000,
B10011000,B00001100,B01000000,
B11100111,B11110011,B11000000,
B00100100,B00010010,B00000000,
B00011000,B00001100,B00000000,
};

const byte playerBitmap[] PROGMEM = {16,9,
B11100000,B00011100,
B11100000,B00011100,
B11100000,B00011100,
B01000000,B00001000,
B01100000,B00011000,
B01011111,B11101000,
B01000000,B00001000,
B01000000,B00001000,
B10100000,B00010100,
};

const byte survivor0Bitmap[] PROGMEM = {8,9,
B01110000,
B01110000,
B01110000,
B00100000,
B01110000,
B10101000,
B00100000,
B00100000,
B01010000,
};

const byte survivor1Bitmap[] PROGMEM = {8,8,
B01110000,
B01110000,
B01110000,
B00101000,
B11110000,
B00100000,
B00110000,
B01001000,
};

const byte survivor2Bitmap[] PROGMEM = {8,8,
B00000100,
B00001110,
B00011111,
B01101110,
B00010100,
B00101000,
B11001000,
B01000000,
};

const byte survivor3Bitmap[] PROGMEM = {8,5,
B00100000,
B10010111,
B01111111,
B10010111,
B00001000,
};

const byte survivor4Bitmap[] PROGMEM = {8,7,
B11100000,
B11100000,
B11100000,
B01000000,
B01110000,
B01000000,
B01110000,
};

const byte survivorsPath[23][3] = {
  {13,6,0},  //Free fall
  {15,15,0}, //Free fall
  {17,24,0}, //Free fall
  {19,30,4}, //Bounce no. 1
  {21,21,1}, //Up in the air
  {23,10,1}, //Up in the air
  {28,6,1},  //Top after bounce no. 1
  {34,11,0}, //Free fall
  {36,20,0}, //Free fall
  {37,30,4}, //Bounce no. 2
  {40,21,1}, //Up in the air
  {42,14,1}, //Up in the air
  {46,11,1}, //Top after bounce no. 2
  {51,14,0}, //Free fall
  {54,23,0}, //Free fall
  {55,30,4}, //Bounce no. 3
  {59,21,1}, //Up in the air
  {64,16,1}, //Top after bounce no. 3
  {69,19,0}, //Free fall
  {71,25,4}, //Ambulance
};

const byte survivorsNumberOfSteps[3] = {20,20,20};

void titleScreen() {
  gb.titleScreen(F("FireBuino!"), titleScreenBitmap);
}

void initGame() {
  lives = 3;
  score = 0;
  playerPosition = 1;
  for(int i = 0; i < 10; i++) {
    survivors[i] = NULL;
  }
  survivors[0] = new Survivor(0, 3);
  noOfSurvivors = 1;
  spawnDelay = 2;
  gb.pickRandomSeed();
}

void drawScore() {
  if(score <= 9) {
    gb.display.cursorX = 53;
  } else if (score <= 99) {
    gb.display.cursorX = 49;
  } else if (score <= 999) {
    gb.display.cursorX = 45;
  } else if (score <= 9999) {
    gb.display.cursorX = 41;
  } else if (score <= 99999) {
    gb.display.cursorX = 37;
  } else if (score <= 999999) {
    gb.display.cursorX = 33;
  } else if (score <= 9999999) {
    gb.display.cursorX = 29;
  } else {
    gb.display.cursorX = 25;
  }
  gb.display.cursorY = 1;
  gb.display.print(score);
}

void drawLives() {
  if(lives > 3) {
    gb.display.drawBitmap(70, 0, livesBitmap);
    gb.display.cursorX = 77;
    gb.display.cursorY = 1;
    gb.display.print(lives);
  } else {
    for(int i = 0; i < lives; i++) {
      gb.display.drawBitmap(60+(i*7), 0, livesBitmap);
    }
  }
}

void drawAmbulance() {
  gb.display.drawBitmap(67, 28, ambulanceBitmap);
  gb.display.setColor(GRAY, WHITE);
  gb.display.fillRect(67+5, 28+5, 3, 1);
  gb.display.fillRect(67+13, 28+4, 1, 1);
  gb.display.fillRect(67+12, 28+5, 2, 1);
  gb.display.setColor(WHITE, WHITE);
  gb.display.fillRect(67+3, 28+13, 2, 2);
  gb.display.fillRect(67+12, 28+13, 2, 2);
  gb.display.setColor(BLACK, WHITE);
}

void moveSurvivors() {
  for(int i = 0; i < 10; i++) {
    if(survivors[i] != NULL) {
      //If dead, remove
      if(survivors[i]->_dead) {
        survivors[i] = NULL;
        noOfSurvivors--;
        continue;
      }
      
      if(survivors[i]->_delay > 0) {
        survivors[i]->_delay--;
      } else {
        survivors[i]->_step++;
      }
      
      //After bounce logic
      if((survivors[i]->_step == 4) ||
         (survivors[i]->_step == 10) ||
         (survivors[i]->_step == 16)) {
          
        //Bounced against player; proceed
        if(survivors[i]->_bounced) {
          survivors[i]->_bounced = false;
          score++;
        //Missed, mark as dead
        } else {
          survivors[i]->_dead = true;
          gb.sound.playCancel();
          lives--;
          continue;
        }
      }

      //Got to the ambulance
      if(survivors[i]->_step >= survivorsNumberOfSteps[survivors[i]->_floor]) {
        survivors[i] = NULL;
        noOfSurvivors--;
        score += 10;
      }
    }
  }
}

void drawSurvivors() {
  for(int i = 0; i < 10; i++) {
    if(survivors[i] != NULL) {
      if(survivors[i]->_dead) {
        if(survivors[i]->_step > 10) {
          gb.display.drawBitmap(53, 43, survivor3Bitmap);
        } else if(survivors[i]->_step > 4) {
          gb.display.drawBitmap(35, 43, survivor3Bitmap);
        } else {
          gb.display.drawBitmap(16, 43, survivor3Bitmap);
        }
      } else {
        if(survivors[i]->_delay > 0) {
          gb.display.drawBitmap(0, 12 * survivors[i]->_floor, survivor0Bitmap);
        } else {
          switch(survivorsPath[survivors[i]->_step][2]) {
            case 0: gb.display.drawBitmap(survivorsPath[survivors[i]->_step][0],
                                          survivorsPath[survivors[i]->_step][1],
                                          survivor0Bitmap);
                    break;
            case 1: gb.display.drawBitmap(survivorsPath[survivors[i]->_step][0],
                                          survivorsPath[survivors[i]->_step][1],
                                          survivor1Bitmap);
                    break;
            case 2: gb.display.drawBitmap(survivorsPath[survivors[i]->_step][0],
                                          survivorsPath[survivors[i]->_step][1],
                                          survivor2Bitmap);
                    break;
            case 3: gb.display.drawBitmap(survivorsPath[survivors[i]->_step][0],
                                          survivorsPath[survivors[i]->_step][1],
                                          survivor3Bitmap);
                    break;
            case 4: gb.display.drawBitmap(survivorsPath[survivors[i]->_step][0],
                                          survivorsPath[survivors[i]->_step][1],
                                          survivor4Bitmap);
                    break;
          }
        }
      }
    }
  }
}

void checkBounces() {
  for(int i = 0; i < 10; i++) {
    if(!survivors[i]->_bounced) {
      if((survivors[i]->_step == 3 && playerPosition == 0) ||
         (survivors[i]->_step == 9 && playerPosition == 1) ||
         (survivors[i]->_step == 15 && playerPosition == 2)) {
        survivors[i]->_bounced = true;
        gb.sound.playOK();
      }
    }
  }
}

void drawGameOver() {
  gb.display.setColor(WHITE);
  gb.display.fillRect(24,20,37,7);
  gb.display.setColor(BLACK);
  gb.display.cursorX = 25;
  gb.display.cursorY = 21;
  gb.display.print("GAME OVER");

  gb.display.setColor(WHITE);
  gb.display.fillRect(0, 41, 84, 7);
  gb.display.setColor(BLACK, WHITE);
  gb.display.cursorX = 1;
  gb.display.cursorY = 42;
  gb.display.print("Highscore:");
  gb.display.cursorX = 45;
  gb.display.cursorY = 42;
  gb.display.print(highscore);
}

void setup() {
  gb.begin();
  titleScreen();
  initGame();
  EEPROM.get(0x00, highscore);
}

void loop() {
  if(gb.update()){
    //Draw gray background
    gb.display.setColor(GRAY, WHITE);
    gb.display.drawBitmap(0, 0, subBackgroundBitmap);

    //Draw black background
    gb.display.setColor(BLACK, WHITE);
    gb.display.drawBitmap(0, 0, backgroundBitmap);

    //Draw score on the top right corner, before the number of lives
    drawScore();

    //Draw lives on the top right corner, after the score
    drawLives();

    //Draw the ambulance
    drawAmbulance();

    if(lives > 0) {
      //Draw game screen

      //Move player to the left if BTN_LEFT pressed
      if(gb.buttons.pressed(BTN_LEFT)) {
        if(playerPosition > 0) {
          playerPosition--;
        }
      }

      //Move player to the left if BTN_A pressed
      if(gb.buttons.pressed(BTN_A)) {
        if(playerPosition > 0) {
          playerPosition--;
        }
      }
  
      //Move player to the right if BTN_RIGHT pressed
      if(gb.buttons.pressed(BTN_RIGHT)) {
        if(playerPosition < 2) {
          playerPosition++;
        }
      }

      //Move player to the right if BTN_B pressed
      if(gb.buttons.pressed(BTN_B)) {
        if(playerPosition < 2) {
          playerPosition++;
        }
      }
  
      //Decrement movement tick delay
      moveTick--;
  
      //Game tick
      if (moveTick <= 0) {
        moveSurvivors();
  
        gb.sound.playTick();

        //Decrement spawn delay
        if(spawnDelay > 0) {
          spawnDelay--;
        }

        //Get faster with more score
        if(score <= (14 * 50)) {
          moveTick = 3 + (14 - (score / 50));
        }

        //Try to spawn a new survivor
        if(spawnDelay <= 0) {
          if(noOfSurvivors > 0) {
            randNo = random(0,9);
          } else {
            randNo = random(0,1);
          }
          
          if(randNo == 0) {
            //Search for a blank spot in the array
            for(int i = 0; i < 10; i++) {
              if(survivors[i] == NULL) { //found!
                if(score <= 100) {
                  //Only Third floor
                  survivors[i] = new Survivor(0, random(1,5));
                } else if(score <= 300) {
                  //Third or second floor
                  survivors[i] = new Survivor(random(0,1), random(1,5));
                } else {
                  //Any floor
                  survivors[i] = new Survivor(random(0,2), random(1,5));
                }
                noOfSurvivors++;
                spawnDelay = 2;
                break;
              }
            }
          }
        }
      }

      //Check collisions with player
      checkBounces();
    }

    //Draw all current survivors
    drawSurvivors();

    //Draw the player
    if(playerPosition == 0) {
      gb.display.drawBitmap(14, 32, playerBitmap);
    } else if(playerPosition == 1) {
      gb.display.drawBitmap(33, 32, playerBitmap);
    } else if(playerPosition == 2) {
      gb.display.drawBitmap(51, 32, playerBitmap);
    }

     if(lives <= 0) {
      //UPDATE highscore if necessary
      if(score > highscore) {
        highscore = score;
        EEPROM.put(0x00, highscore);
      }
      
      //Draw GAME OVER screen
      drawGameOver();
    }
      
    //GoTo title screen if C button is pressed
    if(gb.buttons.pressed(BTN_C)) {
      titleScreen();
      initGame();
    }
  }
}
