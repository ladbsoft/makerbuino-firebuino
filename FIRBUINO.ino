/*-----------------------------------------------------------------
|  Author: Luis Dominguez - LADBSoft.com                          |
|  Date: 31/07/2017                                 Version: 1.4b |
|-----------------------------------------------------------------|
|  Name: FireBuino!                                               |
|  Description: Remake of the classic Game&Watch Fire, from 1980. |
|-----------------------------------------------------------------|
|   C H A N G E L O G                                             |
|  ===================                                            |
|  1.0b: Basic functionality.                                     |
|  1.1b: Bug corrections and speed tweaking.                      |
|  1.2b: Added new graphics by Erico Patricio Monteiro. Added     |
|        main menu and pause screen.                              |
|  1.3b: Added 5-slot highscore ranking. Survivor spawning        |
|        tweaked. Minor enhancements.                             |
|  1.4b: Finally solved the bug that made no more survivors to    |
         spawn! Remember, freeing memory is good :P               |
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
      if (floor == 0) {
        _step = 0;
      } else if (floor == 1) {
        _step = 1;
      } else if (floor == 2) {
        _step = 2;
      }
      _delay = delay;
      _bounced = false;
      _dead = false;
    }
};

#define STATE_MENU 0
#define STATE_PLAYING 1
#define STATE_GAMEOVER 2
#define STATE_PAUSED 3
#define STATE_ABOUT 4

#define HIGHSCORE_COUNT 5
#define NAME_LETTERS 10

byte survivorCount;

Gamebuino gb;
long score;
int highscoreScores[HIGHSCORE_COUNT];
char highscoreNames[HIGHSCORE_COUNT][NAME_LETTERS + 1];
long minHighscore;
short lives;
short playerPosition;
short moveTick;
short spawnDelay;
short noOfSurvivors;
boolean isClassic;
byte gameState;
Survivor *survivors[10]; //max. 10 survivors at the same time
boolean occupiedWindows[3];

const byte survivorNumberOfSteps = 20;

// Graphics (declared in graphics.ino)
extern const byte titleScreenBitmap[] PROGMEM;
extern const byte subBackgroundBitmap[][530] PROGMEM;
extern const byte backgroundBitmap[][530] PROGMEM;
extern const byte livesBitmap[][8] PROGMEM;
extern const byte livesLightBitmap[][8] PROGMEM;
extern const byte livesPositions[][3][2];
extern const byte ambulanceBitmap[][50] PROGMEM;
extern const byte ambulanceShadowBitmap[][50] PROGMEM;
extern const byte ambulanceLightBitmap[][50] PROGMEM;
extern const byte ambulancePositions[][2];
extern const byte playerBitmap[][20] PROGMEM;
extern const byte playerPositions[][3][2];
extern const byte survivor0Bitmap[][11] PROGMEM;
extern const byte survivor1Bitmap[][10] PROGMEM;
extern const byte survivor2Bitmap[][10] PROGMEM;
extern const byte survivor3Bitmap[][7] PROGMEM;
extern const byte survivor4Bitmap[][9] PROGMEM;
extern const byte survivorPositions[][survivorNumberOfSteps][3];
extern const byte survivorIdlePositions[][3];
extern const byte survivorKOPositions[][3][2];

//Menu
#define MENULENGTH 4
const char strPlayNew[] PROGMEM = "Play (new)";
const char strPlayClassic[] PROGMEM = "Play (classic)";
const char strHighscores[] PROGMEM = "High scores";
const char strAbout[] PROGMEM = "About";
const char* const menu[MENULENGTH] PROGMEM = {
  strPlayNew,
  strPlayClassic,
  strHighscores,
  strAbout
};

void titleScreen() {
  gb.titleScreen(F("FireBuino!"), titleScreenBitmap);
  gb.battery.show = false;
}

void initGame() {
  gb.pickRandomSeed();

  gameState = STATE_MENU;
  lives = 3;
  score = 0;
  playerPosition = 1;
  spawnDelay = 2;
  isClassic = false;
  for (int i = 0; i < 10; i++) {
    delete(survivors[i]);
    survivors[i] = NULL;
  }
  survivors[0] = new Survivor(0, 3);
  noOfSurvivors = 1;
  survivorCount = 1;
  for (int i = 0; i < 3; i++) {
    occupiedWindows[i] = false;
  }
  occupiedWindows[0] = true;
}

//Based on code from Crabator, by Rodot
void loadHighscores() {
  for (byte j = 0; j < HIGHSCORE_COUNT; j++) {
    for (byte i = 0; i < NAME_LETTERS; i++) {
      highscoreNames[j][i] = EEPROM.read(i + j * (NAME_LETTERS + 2));
    }
    highscoreScores[j] = EEPROM.read(NAME_LETTERS + j * (NAME_LETTERS + 2)) & 0x00FF; //LSB
    highscoreScores[j] += (EEPROM.read(NAME_LETTERS + 1 + j * (NAME_LETTERS + 2)) << 8) & 0xFF00; //MSB
    highscoreScores[j] = (highscoreScores[j] == 0xFFFF) ? 0 : highscoreScores[j];

    minHighscore = highscoreScores[j];
  }
}

void menuScreen() {
  switch (gb.menu(menu, MENULENGTH)) {
    case 0: //Play (new)
      isClassic = false;
      gameState = STATE_PLAYING;
      break;
    case 1: //Play (classic)
      isClassic = true;
      gameState = STATE_PLAYING;
      break;
    case 2: //Highscores
      drawHighScores();
      break;
    case 3: //About
      gameState = STATE_ABOUT;
      break;
    default:
      titleScreen();
      break;
  }
}

void drawBackground() {
  //Draw gray background
  gb.display.setColor(GRAY, WHITE);
  gb.display.drawBitmap(0, 0, subBackgroundBitmap[isClassic]);

  //Draw black background
  gb.display.setColor(BLACK, WHITE);
  gb.display.drawBitmap(0, 0, backgroundBitmap[isClassic]);
}

void drawScore() {
  if (!isClassic) {
    gb.display.fontSize = 2;
    gb.display.setColor(WHITE, BLACK);

    if (score <= 9) {
      gb.display.cursorX = 76;
    } else if (score <= 99) {
      gb.display.cursorX = 68;
    } else if (score <= 999) {
      gb.display.cursorX = 60;
    } else {
      gb.display.cursorX = 52;
    }

    gb.display.cursorY = 2;
    gb.display.print(score);

    gb.display.fontSize = 1;
    gb.display.setColor(BLACK, WHITE);
  } else {
    gb.display.fontSize = 1;
    gb.display.setColor(BLACK, WHITE);

    if (score <= 9) {
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
}

void drawLives() {
  gb.display.setColor(WHITE);
  switch (lives) {
    case 3: gb.display.drawBitmap(livesPositions[isClassic][2][0],
                                    livesPositions[isClassic][2][1],
                                    livesLightBitmap[isClassic]);
    case 2: gb.display.drawBitmap(livesPositions[isClassic][1][0],
                                    livesPositions[isClassic][1][1],
                                    livesLightBitmap[isClassic]);
    case 1: gb.display.drawBitmap(livesPositions[isClassic][0][0],
                                    livesPositions[isClassic][0][1],
                                    livesLightBitmap[isClassic]);
  }

  gb.display.setColor(BLACK);
  switch (lives) {
    case 3: gb.display.drawBitmap(livesPositions[isClassic][2][0],
                                    livesPositions[isClassic][2][1],
                                    livesBitmap[isClassic]);
    case 2: gb.display.drawBitmap(livesPositions[isClassic][1][0],
                                    livesPositions[isClassic][1][1],
                                    livesBitmap[isClassic]);
    case 1: gb.display.drawBitmap(livesPositions[isClassic][0][0],
                                    livesPositions[isClassic][0][1],
                                    livesBitmap[isClassic]);
  }
}

void drawAmbulance() {
  byte posX = ambulancePositions[isClassic][0];
  byte posY = ambulancePositions[isClassic][1];

  gb.display.setColor(WHITE);
  gb.display.drawBitmap(posX, posY, ambulanceLightBitmap[isClassic]);
  gb.display.setColor(GRAY);
  gb.display.drawBitmap(posX, posY, ambulanceShadowBitmap[isClassic]);
  gb.display.setColor(BLACK, WHITE);
  gb.display.drawBitmap(posX, posY, ambulanceBitmap[isClassic]);
}

void movePlayer() {
  //Move player to the left if BTN_LEFT pressed
  if (gb.buttons.pressed(BTN_LEFT)) {
    if (playerPosition > 0) {
      playerPosition--;
    }
  }

  //Move player to the left if BTN_A pressed
  if (gb.buttons.pressed(BTN_A)) {
    if (playerPosition > 0) {
      playerPosition--;
    }
  }

  //Move player to the right if BTN_RIGHT pressed
  if (gb.buttons.pressed(BTN_RIGHT)) {
    if (playerPosition < 2) {
      playerPosition++;
    }
  }

  //Move player to the right if BTN_B pressed
  if (gb.buttons.pressed(BTN_B)) {
    if (playerPosition < 2) {
      playerPosition++;
    }
  }
}

void spawnSurvivor() {
  byte mustSpawn;
  byte floorNo;
  byte delayTicks;

  if (noOfSurvivors != 0) {
    //More probable with higher score
    if (score <= (5 * 150)) {
      mustSpawn = random(0, (5 + 5 - (score / 150)));
    } else {
      mustSpawn = random(0, 5);
    }
  } else {
    //More probable if no survivors on screen
    mustSpawn = random(0, 2);
  }

  //Spawn if zero
  if (mustSpawn == 0) {
    //Search for a blank spot in the array
    for (int i = 0; i < 10; i++) {
      if (survivors[i] == NULL) { //found!
        if (score <= 300) {
          //Only Third floor
          floorNo = 0;
          delayTicks = random(1, 6);
        } else if (score <= 600) {
          //Third or second floor
          floorNo = random(0, 2);
          delayTicks = random(2, 6);
        } else {
          //Any floor
          floorNo = random(0, 3);
          delayTicks = random(3, 6);
        }

        //Avoid occupied windows
        while (occupiedWindows[floorNo]) {
          if (floorNo > 0) {
            floorNo--;
          } else {
            break;
          }
        }

        //If all windows are occupied, try later
        if (floorNo < 0) {
          break;
        }

        //Else, add the new survivor to the array
        survivors[i] = new Survivor(floorNo, delayTicks);

        noOfSurvivors++;
        survivorCount++;
        spawnDelay = 2;
        break;
      }
    }
  }
}

void gameLogic() {
  //Decrement movement tick delay
  moveTick--;

  //Game tick
  if (moveTick <= 0) {
    moveSurvivors();

    gb.sound.playTick();

    //Decrement spawn delay
    if (spawnDelay > 0) {
      spawnDelay--;
    }

    //Get faster with more score
    if (score <= (13 * 50)) {
      moveTick = 4 + (13 - (score / 50));
    } else {
      moveTick = 4;
    }

    //Try to spawn a new survivor
    if (spawnDelay <= 0) {
      spawnSurvivor();
    }
  }
}

void moveSurvivors() {
  for (int i = 0; i < 10; i++) {
    if (survivors[i] != NULL) {
      //If dead, remove
      if (survivors[i]->_dead) {
        delete(survivors[i]);
        survivors[i] = NULL;
        noOfSurvivors--;
        continue;
      }

      //Movement or delay decrementation
      if (survivors[i]->_delay > 0) {
        survivors[i]->_delay--;
      } else {
        survivors[i]->_step++;
      }

      //Jump logic
      if ((survivors[i]->_floor == 0 && survivors[i]->_step == 1) ||
          (survivors[i]->_floor == 1 && survivors[i]->_step == 2) ||
          (survivors[i]->_floor == 2 && survivors[i]->_step == 3)) {

        occupiedWindows[survivors[i]->_floor] = false;
      }

      //After bounce logic
      if ((survivors[i]->_step == 4) ||
          (survivors[i]->_step == 10) ||
          (survivors[i]->_step == 16)) {

        //Bounced against player; proceed
        if (survivors[i]->_bounced) {
          survivors[i]->_bounced = false;
          score++;
          //Missed, mark as dead
        } else {
          survivors[i]->_dead = true;
          gb.sound.playCancel();
          lives--;
          if (lives <= 0) {
            gameState = STATE_GAMEOVER;
          }
          continue;
        }
      }

      //Got to the ambulance
      if (survivors[i]->_step >= survivorNumberOfSteps) {
        delete(survivors[i]);
        survivors[i] = NULL;
        noOfSurvivors--;
        score += 10;
      }
    }
  }
}

void drawSurvivors() {
  byte posX;
  byte posY;
  byte mult;

  for (int i = 0; i < 10; i++) {
    if (survivors[i] != NULL) {
      if (survivors[i]->_dead) {
        if (survivors[i]->_step > 10) {
          posX = survivorKOPositions[isClassic][2][0];
          posY = survivorKOPositions[isClassic][2][1];
        } else if (survivors[i]->_step > 4) {
          posX = survivorKOPositions[isClassic][1][0];
          posY = survivorKOPositions[isClassic][1][1];
        } else {
          posX = survivorKOPositions[isClassic][0][0];
          posY = survivorKOPositions[isClassic][0][1];
        }

        gb.display.drawBitmap(posX, posY, survivor3Bitmap[isClassic]);
      } else {
        if (survivors[i]->_delay > 0) {
          posX = survivorIdlePositions[isClassic][0];
          posY = survivorIdlePositions[isClassic][1];
          mult = survivorIdlePositions[isClassic][2];

          gb.display.drawBitmap(posX, posY + (survivors[i]->_floor * mult), survivor0Bitmap[isClassic]);
        } else {
          posX = survivorPositions[isClassic][survivors[i]->_step][0];
          posY = survivorPositions[isClassic][survivors[i]->_step][1];

          switch (survivorPositions[isClassic][survivors[i]->_step][2]) {
            case 0: gb.display.drawBitmap(posX, posY, survivor0Bitmap[isClassic]);
              break;
            case 1: gb.display.drawBitmap(posX, posY, survivor1Bitmap[isClassic]);
              break;
            case 2: gb.display.drawBitmap(posX, posY, survivor2Bitmap[isClassic]);
              break;
            case 3: gb.display.drawBitmap(posX, posY, survivor3Bitmap[isClassic]);
              break;
            case 4: gb.display.drawBitmap(posX, posY, survivor4Bitmap[isClassic]);
              break;

            //Flipped horizontally
            case 5: gb.display.drawBitmap(posX, posY, survivor0Bitmap[isClassic], NOROT, FLIPH);
              break;
            case 6: gb.display.drawBitmap(posX, posY, survivor1Bitmap[isClassic], NOROT, FLIPH);
              break;
            case 7: gb.display.drawBitmap(posX, posY, survivor2Bitmap[isClassic], NOROT, FLIPH);
              break;
            case 8: gb.display.drawBitmap(posX, posY, survivor3Bitmap[isClassic], NOROT, FLIPH);
              break;
            case 9: gb.display.drawBitmap(posX, posY, survivor4Bitmap[isClassic], NOROT, FLIPH);
              break;

          }
        }
      }
    }
  }
}

void drawPlayer() {
  byte posX = playerPositions[isClassic][playerPosition][0];
  byte posY = playerPositions[isClassic][playerPosition][1];

  gb.display.drawBitmap(posX, posY, playerBitmap[isClassic]);
}

void checkBounces() {
  for (int i = 0; i < 10; i++) {
    if (!survivors[i]->_bounced) {
      if ((survivors[i]->_step == 3 && playerPosition == 0) ||
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
  gb.display.fillRect(24, 20, 37, 7);
  gb.display.setColor(BLACK, WHITE);
  gb.display.cursorX = 25;
  gb.display.cursorY = 21;
  gb.display.print("GAME OVER");

  //Draw "New Highscore!" if a new Highscore was reached
  if (score > minHighscore) {
    gb.display.setColor(WHITE);
    gb.display.fillRect(14, 29, 57, 7);
    gb.display.setColor(BLACK, WHITE);
    gb.display.cursorX = 15;
    gb.display.cursorY = 30;
    gb.display.print("NEW HIGHSCORE!");
  }

  gb.display.setColor(WHITE);
  gb.display.fillRect(55, LCDHEIGHT - gb.display.fontHeight - 1, (gb.display.fontWidth * 7), gb.display.fontHeight);
  gb.display.setColor(BLACK, WHITE);
  gb.display.cursorX = 56;
  gb.display.cursorY = LCDHEIGHT - gb.display.fontHeight;
  gb.display.print("\x17: Menu");
}

//Based on code from Crabator, by Rodot
void saveHighscore() {
  gb.getDefaultName(highscoreNames[HIGHSCORE_COUNT - 1]);
  gb.keyboard(highscoreNames[HIGHSCORE_COUNT - 1], NAME_LETTERS + 1);
  highscoreScores[HIGHSCORE_COUNT - 1] = score;

  //Sort highscores
  for (byte i = HIGHSCORE_COUNT - 1; i > 0; i--) {
    if (highscoreScores[i - 1] < highscoreScores[i]) {
      char tempName[NAME_LETTERS];
      strcpy(tempName, highscoreNames[i - 1]);
      strcpy(highscoreNames[i - 1], highscoreNames[i]);
      strcpy(highscoreNames[i], tempName);
      unsigned int tempScore;
      tempScore = highscoreScores[i - 1];
      highscoreScores[i - 1] = highscoreScores[i];
      highscoreScores[i] = tempScore;

      //update minimum highscore
      minHighscore = highscoreScores[i];
    }
    else {
      break;
    }
  }

  //Save highscores in EEPROM
  for (byte j = 0; j < HIGHSCORE_COUNT; j++) {
    for (byte i = 0; i < NAME_LETTERS; i++) {
      EEPROM.write(i + j * (NAME_LETTERS + 2), highscoreNames[j][i]);
    }
    EEPROM.write(NAME_LETTERS + j * (NAME_LETTERS + 2), highscoreScores[j] & 0x00FF); //LSB
    EEPROM.write(NAME_LETTERS + 1 + j * (NAME_LETTERS + 2), (highscoreScores[j] >> 8) & 0x00FF); //MSB
  }
  drawHighScores();
}

void drawHighScores() {
  while (true) {
    if (gb.update()) {
      gb.display.drawRect(0, 0, LCDWIDTH, LCDHEIGHT);

      //Title
      gb.display.cursorX = 20;
      gb.display.cursorY = 3;
      gb.display.println(F("HIGH SCORES"));

      gb.display.cursorY = gb.display.fontHeight * 2;
      for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
        gb.display.cursorX = 6;

        //Name
        if (highscoreScores[i] == 0) {
          gb.display.print('-');
        } else {
          gb.display.print(highscoreNames[i]);
        }

        //Score
        if (highscoreScores[i] > 9999) {
          gb.display.cursorX = LCDWIDTH - 6 - 5 * gb.display.fontWidth;
        } else if (highscoreScores[i] > 999) {
          gb.display.cursorX = LCDWIDTH - 6 - 4 * gb.display.fontWidth;
        } else if (highscoreScores[i] > 99) {
          gb.display.cursorX = LCDWIDTH - 6 - 3 * gb.display.fontWidth;
        } else if (highscoreScores[i] > 9) {
          gb.display.cursorX = LCDWIDTH - 6 - 2 * gb.display.fontWidth;
        } else {
          gb.display.cursorX = LCDWIDTH - 6 - gb.display.fontWidth;
        }
        gb.display.cursorY = (gb.display.fontHeight * 2) + (gb.display.fontHeight * i);
        gb.display.println(highscoreScores[i]);
      }

      if (gb.buttons.pressed(BTN_A) || gb.buttons.pressed(BTN_B) || gb.buttons.pressed(BTN_C)) {
        gb.sound.playOK();
        break;
      }
    }
  }
}

void drawPaused() {
  gb.display.setColor(WHITE);
  gb.display.fillRect(28, 19, (gb.display.fontWidth * 6), gb.display.fontHeight);
  gb.display.setColor(BLACK, WHITE);
  gb.display.cursorX = 29;
  gb.display.cursorY = 20;
  gb.display.print("PAUSED");

  gb.display.setColor(WHITE);
  gb.display.fillRect(39, LCDHEIGHT - (gb.display.fontHeight * 2) - 1, (gb.display.fontWidth * 11), gb.display.fontHeight);
  gb.display.setColor(BLACK, WHITE);
  gb.display.cursorX = 40;
  gb.display.cursorY = LCDHEIGHT - (gb.display.fontHeight * 2);
  gb.display.print("\x15: Continue");

  gb.display.setColor(WHITE);
  gb.display.fillRect(55, LCDHEIGHT - gb.display.fontHeight - 1, (gb.display.fontWidth * 7), gb.display.fontHeight);
  gb.display.setColor(BLACK, WHITE);
  gb.display.cursorX = 56;
  gb.display.cursorY = LCDHEIGHT - gb.display.fontHeight;
  gb.display.print("\x17: Quit");
}

void drawCredits() {
  gb.display.cursorX = 24;
  gb.display.cursorY = 6;
  gb.display.print("Developer:");
  gb.display.cursorX = 28;
  gb.display.cursorY = 14;
  gb.display.print("LADBSoft");
  gb.display.cursorX = 2;
  gb.display.cursorY = 26;
  gb.display.print("Awesome new graphics:");
  gb.display.cursorX = 34;
  gb.display.cursorY = 34;
  gb.display.print("erico");

  gb.display.cursorX = 56;
  gb.display.cursorY = LCDHEIGHT - gb.display.fontHeight;
  gb.display.print("\x17: Back");
}

void setup() {
  gb.begin();
  titleScreen();
  initGame();
  loadHighscores();
}

void loop() {
  menuScreen();

  while (1) {
    if (gb.update()) {
      if (gameState == STATE_PLAYING || gameState == STATE_PAUSED || gameState == STATE_GAMEOVER) {
        drawBackground();

        //Draw score on the top right corner, before the number of lives
        drawScore();

        //Draw lives on the top right corner, after the score
        drawLives();

        //Draw the ambulance
        drawAmbulance();

        if (gameState == STATE_PLAYING) {
          //Move the player acordingly to the pressed buttons
          movePlayer();

          //Do game stuff (delays, spawning...)
          gameLogic();

          //Check collisions with player
          checkBounces();
        }

        //Draw all current survivors
        drawSurvivors();

        //Draw the player
        drawPlayer();

        if (gameState == STATE_PAUSED) {
          drawPaused();

          //Unpause
          if (gb.buttons.pressed(BTN_A)) {
            gameState = STATE_PLAYING;
          }
        }

        if (gameState == STATE_GAMEOVER) {
          //Draw GAME OVER screen
          drawGameOver();
        }
      } else if (gameState == STATE_ABOUT) {
        drawCredits();
      }

      //GoTo title screen if C button is pressed
      if (gb.buttons.pressed(BTN_C)) {
        if (gameState == STATE_PLAYING) {
          gameState = STATE_PAUSED;
        } else if (gameState == STATE_ABOUT) {
          gb.sound.playOK();
          gameState = STATE_MENU;
          break;
        } else {
          //UPDATE highscore if necessary
          if (score > minHighscore) {
            saveHighscore();
          }

          titleScreen();
          initGame();
          break;
        }
      }
    }

    //Return to menu
    if (gameState == STATE_MENU) {
      break;
    }
  }
}

