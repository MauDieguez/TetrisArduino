/* LED Tetris v2 by Jose Mauricio Munoz Dieguez
   03/12/20 - First time written in Arduino
   07/06/20 - Joystick Adittion
   07/26/20 - Intro WIP
   07/28/20 - First Test w/o intro
*/

#include <FastLED.h>
#include <StopWatch.h>
#include <Adafruit_TiCoServo.h>

// We define the LED pin as well as the lenght and with of our matrix
#define LED_PIN 10
#define LENGTH 6
#define WIDTH 10

#define STICK_X A0
#define STICK_Y A1

#define SERVO_MIN 700
#define SERVO_MAX 2600
Adafruit_TiCoServo servo;

// We define our LEDs array
CRGB leds[60];

// Variables to make delays without pausing all the code`
StopWatch pieceDownTimer;
StopWatch moveDelayTimer;
StopWatch rotationDelayTimer;
StopWatch downDelayTimer;


// Due to the memory of the arduino is limited, we try to optimize as much memory as possible.

uint8_t nextTetrino; // Stores next tetrino

uint16_t targetMillis; 
uint16_t delayTime;
uint16_t rotationTime;

int tilePosX[4]; // Current tile X positions
int tilePosY[4]; // Current tile Y positions

int auxTilePosX[4]; // Aux tile X positions we use for the rotation algorithm
int auxTilePosY[4]; // Aux tile Y positions we use for the rotation algorithm

int newTilePosY[4]; // Aux array for another algorithm

bool taken[WIDTH][LENGTH]; // An array that helps us to know which cell is taken by a tetronimo

uint8_t currentLevel = 0; // Indicator of the current level of the game

uint32_t score = 0; // Total score of the game
int totalLines = 0; // Total lines we have completed
int linesObj = 5; // Number of lines we need to complete to advance to the next level

int lastScore = 0; // Last total score
int lastLevel = 0; // Last level

int stickX; // Value of the X-axis of the analog stick
int stickY; // Value of the Y-axis of the analog stick

// Data for the Super Rotation System
int JLSTZOffsetDataX[5] = {0, -1, -1, 0, -1}; 
int JLSTZOffsetDataY[5] = {0, -1, -1, 2, 2}; 

int IOffsetDataX[5] = {0, -2, 1, -2, 1}; // 
int IOffsetDataY[5] = {0, 0, 0, +1, -2};

// Values for the LED
uint8_t RED;
uint8_t GREEN;
uint8_t BLUE;


// This arrays contains the intro of the game. All this colors codes form T - E - T - R - I - S at the beggining of the game.
CRGB _initT_01[10][6] =  {
                       {CRGB(255,0,0), CRGB(255,0,0),     CRGB(255,0,0),     CRGB(255,0,0), CRGB(255,0,0), CRGB(255,0,0)},
                       {CRGB(255,0,0), CRGB(128,0,0),     CRGB(128,0,0),     CRGB(128,0,0), CRGB(128,0,0), CRGB(255,0,0)},
                       {CRGB(128,0,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(128,0,0)},
                       {CRGB(128,0,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(128,0,0)},
                       {CRGB(255,0,0), CRGB(128,0,0),     CRGB(255,255,255), CRGB(255,255,255), CRGB(128,0,0), CRGB(255,0,0)},
                       {CRGB(255,0,0), CRGB(128,0,0),     CRGB(255,255,255), CRGB(255,255,255), CRGB(128,0,0), CRGB(255,0,0)},
                       {CRGB(255,0,0), CRGB(128,0,0),     CRGB(255,255,255), CRGB(255,255,255), CRGB(128,0,0), CRGB(255,0,0)},
                       {CRGB(255,0,0), CRGB(128,0,0),     CRGB(255,255,255), CRGB(255,255,255), CRGB(128,0,0), CRGB(255,0,0)},
                       {CRGB(255,0,0), CRGB(255,0,0),     CRGB(128,0,0),     CRGB(128,0,0), CRGB(255,0,0), CRGB(255,0,0)},
                       {CRGB(255,0,0), CRGB(255,0,0),     CRGB(255,0,0),     CRGB(255,0,0), CRGB(255,0,0), CRGB(255,0,0)}
                      };

CRGB _initE[10][6] =  {
                       {CRGB(255,114,0), CRGB(255,114,0),   CRGB(255,114,0),   CRGB(255,114,0),   CRGB(255,114,0),   CRGB(255,114,0)},
                       {CRGB(255,114,0), CRGB(128,57,0),    CRGB(128,57,0),    CRGB(128,57,0),    CRGB(128,57,0),    CRGB(255,114,0)},
                       {CRGB(128,57,0),  CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(128,57,0)},
                       {CRGB(128,57,0),  CRGB(255,255,255), CRGB(255,255,255), CRGB(128,57,0),    CRGB(128,57,0),    CRGB(128,57,0)},
                       {CRGB(128,57,0),  CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(128,57,0),    CRGB(255,114,0)},
                       {CRGB(128,57,0),  CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(128,57,0),    CRGB(255,114,0)},
                       {CRGB(128,57,0),  CRGB(255,255,255), CRGB(255,255,255), CRGB(128,57,0),    CRGB(128,57,0),    CRGB(128,57,0)},
                       {CRGB(128,57,0),  CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(128,57,0)},
                       {CRGB(255,114,0), CRGB(128,57,0),    CRGB(128,57,0),    CRGB(128,57,0),    CRGB(128,57,0),    CRGB(255,114,0)},
                       {CRGB(255,114,0), CRGB(255,114,0),   CRGB(255,114,0),   CRGB(255,114,0),   CRGB(255,114,0),   CRGB(255,114,0)}
                      };
                      
CRGB _initT_02[10][6] = {
                            {CRGB(255,210,0), CRGB(255,210,0),   CRGB(255,210,0),   CRGB(255,210,0),   CRGB(255,210,0),   CRGB(255,210,0)},
                            {CRGB(255,210,0), CRGB(128,105,0),   CRGB(128,105,0),   CRGB(128,105,0),   CRGB(128,105,0),   CRGB(255,210,0)},
                            {CRGB(128,105,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(128,105,0)},
                            {CRGB(128,105,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(128,105,0)},
                            {CRGB(255,210,0), CRGB(128,105,0),   CRGB(255,255,255), CRGB(255,255,255), CRGB(128,105,0),   CRGB(255,210,0)},
                            {CRGB(255,210,0), CRGB(128,105,0),   CRGB(255,255,255), CRGB(255,255,255), CRGB(128,105,0),   CRGB(255,210,0)},
                            {CRGB(255,210,0), CRGB(128,105,0),   CRGB(255,255,255), CRGB(255,255,255), CRGB(128,105,0),   CRGB(255,210,0)},
                            {CRGB(255,210,0), CRGB(128,105,0),   CRGB(255,255,255), CRGB(255,255,255), CRGB(128,105,0),   CRGB(255,210,0)},
                            {CRGB(255,210,0), CRGB(255,210,0),   CRGB(128,105,0),   CRGB(128,105,0),   CRGB(255,210,0),   CRGB(255,210,0)},
                            {CRGB(255,210,0), CRGB(255,210,0),   CRGB(255,210,0),   CRGB(255,210,0),   CRGB(255,210,0),   CRGB(255,210,0)}
                        };

CRGB _initR[10][6] = {
                            {CRGB(84,255,0), CRGB(84,255,0),    CRGB(84,255,0),    CRGB(84,255,0),    CRGB(84,255,0),    CRGB(84,255,0)},
                            {CRGB(84,255,0), CRGB(42,128,0),    CRGB(42,128,0),    CRGB(42,128,0),    CRGB(84,255,0),    CRGB(84,255,0)},
                            {CRGB(42,128,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(42,128,0),    CRGB(84,255,0)},
                            {CRGB(42,128,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(42,128,0),    CRGB(255,255,255), CRGB(42,128,0)},
                            {CRGB(42,128,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(42,128,0),    CRGB(255,255,255), CRGB(42,128,0)},
                            {CRGB(42,128,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(42,128,0),    CRGB(84,255,0)},
                            {CRGB(42,128,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(42,128,0),    CRGB(255,255,255), CRGB(42,128,0)},
                            {CRGB(42,128,0), CRGB(255,255,255), CRGB(255,255,255), CRGB(42,128,0),    CRGB(255,255,255), CRGB(42,128,0)},
                            {CRGB(42,128,0), CRGB(42,128,0),    CRGB(42,128,0),    CRGB(42,128,0),    CRGB(42,128,0),    CRGB(42,128,0)},
                            {CRGB(84,255,0), CRGB(84,255,0),    CRGB(84,255,0),    CRGB(84,255,0),    CRGB(84,255,0),    CRGB(84,255,0)}
                     };


CRGB _initI[10][6] = {
                            {CRGB(0,255,192), CRGB(0,255,192), CRGB(0,255,192),   CRGB(0,255,192),   CRGB(0,255,192), CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,255,192), CRGB(0,128,96),    CRGB(0,128,96),    CRGB(0,255,192), CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,128,96),  CRGB(255,255,255), CRGB(255,255,255), CRGB(0,128,96),  CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,128,96),  CRGB(255,255,255), CRGB(255,255,255), CRGB(0,128,96),  CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,128,96),  CRGB(255,255,255), CRGB(255,255,255), CRGB(0,128,96),  CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,128,96),  CRGB(255,255,255), CRGB(255,255,255), CRGB(0,128,96),  CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,128,96),  CRGB(255,255,255), CRGB(255,255,255), CRGB(0,128,96),  CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,128,96),  CRGB(255,255,255), CRGB(255,255,255), CRGB(0,128,96),  CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,255,192), CRGB(0,128,96),    CRGB(0,128,96),    CRGB(0,255,192), CRGB(0,255,192)},
                            {CRGB(0,255,192), CRGB(0,255,192), CRGB(0,255,192),   CRGB(0,255,192),   CRGB(0,255,192), CRGB(0,255,192)}
                     };
                                   

CRGB _initS[10][6] = {
                            {CRGB(0,66,255), CRGB(0,66,255),    CRGB(0,66,255),    CRGB(0,66,255),    CRGB(0,66,255),    CRGB(0,66,255)},
                            {CRGB(0,66,255), CRGB(0,48,128),    CRGB(0,48,128),    CRGB(0,48,128),    CRGB(0,48,128),    CRGB(0,66,255)},
                            {CRGB(0,66,255), CRGB(0,48,128),    CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(0,48,128)},
                            {CRGB(0,48,128), CRGB(255,255,255), CRGB(255,255,255), CRGB(0,48,128),    CRGB(0,48,128),    CRGB(0,66,255)},
                            {CRGB(0,48,128), CRGB(255,255,255), CRGB(255,255,255), CRGB(0,48,128),    CRGB(0,66,255),    CRGB(0,66,255)},
                            {CRGB(0,66,255), CRGB(0,48,128),    CRGB(255,255,255), CRGB(255,255,255), CRGB(0,48,128),    CRGB(0,66,255)},
                            {CRGB(0,66,255), CRGB(0,48,128),    CRGB(0,48,128),    CRGB(0,48,128),    CRGB(255,255,255), CRGB(0,48,128)},
                            {CRGB(0,48,128), CRGB(255,255,255), CRGB(255,255,255), CRGB(255,255,255), CRGB(0,48,128),    CRGB(0,66,255)},
                            {CRGB(0,66,255), CRGB(0,48,128),    CRGB(0,48,128),    CRGB(0,48,128),    CRGB(0,66,255),    CRGB(0,66,255)},
                            {CRGB(0,66,255), CRGB(0,66,255),    CRGB(0,66,255),    CRGB(0,66,255),    CRGB(0,66,255),    CRGB(0,66,255)} 
                     };


// We define a random color for the tetrino
void defineLEDColor()
{
  RED = random(1,255);
  GREEN = random(1,255);
  BLUE = random(1,255);
}


// This function converts a x,y coordinate to array form.
int arrayConverter(int i, int j)
{
  if (i%2 == 0)
  {
    return LENGTH * i  + j;
  }
  return (LENGTH * (i+ 1)) - (j+1);
}

// This function updates a cell and lits the an specific LED in the strip
void updateTile(int nTile, int x, int y)
{
  tilePosX[nTile] = x;
  tilePosY[nTile] = y;

  leds[arrayConverter(tilePosY[nTile], tilePosX[nTile])] = CRGB(RED,GREEN,BLUE); 
  taken[tilePosY[nTile]][tilePosX[nTile]] = true; // Now it is taken
}



bool isOTetrino = false;
bool isITetrino = false;

// Where the pieces should spawn

void t_tetrino()
{
    updateTile(0,2,1);
    updateTile(1,3,1); 
    updateTile(2,4,1);
    updateTile(3,3,0);
    isOTetrino = false;
    isITetrino = false;
}

void j_tetrino()
{
    updateTile(0,3,1);
    updateTile(1,2,1);
    updateTile(2,4,1);
    updateTile(3,4,0);
    isOTetrino = false;
    isITetrino = false;
}

void l_tetrino()
{
    updateTile(0,2,1);
    updateTile(1,3,1);
    updateTile(2,4,1);
    updateTile(3,2,0);
    isOTetrino = false;
    isITetrino = true;
}

void s_tetrino()
{
    updateTile(0,2,1);
    updateTile(1,3,1);
    updateTile(2,3,0);
    updateTile(3,4,0);
    isOTetrino = false;
    isITetrino = false;
}

void z_tetrino()
{
    updateTile(0,2,0);
    updateTile(1,3,0);
    updateTile(2,3,1);
    updateTile(3,4,1);
    isOTetrino = false;
    isITetrino = false;
}

void o_tetrino()
{
    updateTile(0,3,0);
    updateTile(1,4,0);
    updateTile(2,3,1);
    updateTile(3,4,1);
    isOTetrino = true;
    isITetrino = false;
}

// This function moves the servo according to the next tetronimo that will appear
void moveServo()
{
  Serial.println("Moving Servo");
  switch(nextTetrino)
  {
        case 0: 
        {
          Serial.println("T Tetrino");
          servo.write(map(180,0,180,SERVO_MIN, SERVO_MAX)); 
          break;
        }
        case 1: 
        {
          Serial.println("L Tetrino"); 
          servo.write(map(110,0,180,SERVO_MIN, SERVO_MAX)); 
          break;
        }
        case 2:
        {
          Serial.println("J Tetrino");
          servo.write(map(63,0,180,SERVO_MIN, SERVO_MAX));
          break;
        }
        case 3:
        {
          Serial.println("Z Tetrino");
          servo.write(map(35,0,180,SERVO_MIN, SERVO_MAX)); 
          break;
        }
        case 4:
        {
          Serial.println("S Tetrino");
          servo.write(map(137,0,180,SERVO_MIN, SERVO_MAX)); 
          break;
        }
        case 5:
        {
          Serial.println("O Tetrino");
          servo.write(map(5,0,180,SERVO_MIN, SERVO_MAX)); 
          break;
        }
        default: break;
  }
  delay(15);
}

// This functions chooses randomly one tetrino as the next piece.
void generateTetrino()
{
  defineLEDColor();
  uint8_t selectedPiece = nextTetrino;
  nextTetrino = (int)random(0,6);
  moveServo();
  switch(selectedPiece)
  {
        case 0: t_tetrino(); break;
        case 1: l_tetrino(); break;
        case 2: j_tetrino(); break;
        case 3: z_tetrino(); break;
        case 4: s_tetrino(); break;
        case 5: o_tetrino(); break;
        default: break;
  }
}

// Deletes an entire tetronino 
void deleteCurrent()
{
  for (int i=0;i<4;i++)
  {
    leds[arrayConverter(tilePosY[i], tilePosX[i])] = CRGB::Black;
    taken[tilePosY[i]][tilePosX[i]] = false;

  }
}

// Prints the current LED state
void printState()
{
  FastLED.show();
}

// Returns true if the current position is not taken and if is not out of bounds.
bool isValid(int currentPositionY, int currentPositionX) 
{
  return currentPositionX < LENGTH && currentPositionX >= 0 && currentPositionY < WIDTH  && currentPositionY >= 0 && !taken[currentPositionY][currentPositionX];
  //return leds[arrayConverter(currentPositionY, currentPositionX)] == CRGB::Black;
}

// This functions tests all the possible offsets when we rotate a piece. The first one that is compatible is returned. Otherwise it returns false without updating anything
bool rotationOffset()
{
  int globalTilePosX[5];
  int globalTilePosY[5];

  int currentOffsetDataX[5];
  int currentOffsetDataY[5];

  if (!isITetrino) //Different that for I tetrino
  {
    for (int i=0;i<5;i++)
    {
      currentOffsetDataX[i] = JLSTZOffsetDataX[i];
      currentOffsetDataY[i] = JLSTZOffsetDataY[i];
    }
  }
  else
  {
    for (int i=0;i<5;i++)
    {
      currentOffsetDataX[i] = IOffsetDataX[i];
      currentOffsetDataY[i] = IOffsetDataY[i];
    }
  }

  for (int offsetIndex = 0;offsetIndex<5;offsetIndex++) // We test all 5 offset data
  {
    int count = 0; 
    for (int i=0;i<4;i++) // We ensure every tetronimo position is valid
    {
      count++;
      globalTilePosX[i] = auxTilePosX[i] + currentOffsetDataX[offsetIndex];
      globalTilePosY[i] = auxTilePosY[i] + currentOffsetDataY[offsetIndex];

      if (!isValid(globalTilePosY[i], globalTilePosX[i]))break;
    }
    if (count == 4) // If every position of the tetronimo is valid, then update to an aux array and return true
    {
      for (int i=0;i<4;i++)
      {
        auxTilePosX[i] = globalTilePosX[i];
        auxTilePosY[i] = globalTilePosY[i];
      }
      return 1;
    }
  }
  return 0; // Offset is impossible 
}



// This function makes a simple rotation without any offset  
void rotateTile(int currentPositionX, int currentPositionY, int nTile)
{
  int originPositionX = tilePosX[1], originPositionY = tilePosY[1]; // We save the center piece / pivot
  int relativePosX = currentPositionX - originPositionX, relativePosY = currentPositionY - originPositionY; // We make the coordinates relative to be able to multiply it by the rotation matrix

  int newX = relativePosY * -1; // Rotation Matrix formula
  int newY = relativePosX;

  int newCurrentX = newX + originPositionX; // We return to the correct coordinate system
  int newCurrentY = newY + originPositionY;

  auxTilePosX[nTile] = newCurrentX; // We update our aux array
  auxTilePosY[nTile] = newCurrentY;
}

// This is the head function of the rotation. It performs the rotation, look if any offset is possible and then updates.
void rotation()
{
  if (isOTetrino)return; // O Tetrinos don't rotate
  deleteCurrent(); // We delete the current tetronimo position

  // We rotate all 4 tiles
  for (int i=0;i<4;i++)
  {
    rotateTile(tilePosX[i], tilePosY[i], i);
  }

  // We check if an offset is possible
  bool possible = rotationOffset();

  
  if (possible) // If possible then we apply it
  {
    for (int i=0;i<4;i++)
    {
      tilePosX[i] = auxTilePosX[i];
      tilePosY[i] = auxTilePosY[i];
      updateTile(i, tilePosX[i], tilePosY[i]);
    }
  }
  else // Otherwise we stay the same
  {
    for (int i=0;i<4;i++)
    {
      updateTile(i,tilePosX[i], tilePosY[i]);
    }
  }
 
}


// This function offsets a piece in the X axis
int offsetX(int currentPositionX, int offsetX)
{
  return currentPositionX + offsetX;
}

// This function offsets a piece in the Y axis
int offsetY(int currentPositionY, int offsetY)
{
  return currentPositionY + offsetY;
}

// This functions makes a full tetronimo to move in the X-axis
void movementX(int direction)
{
  bool collision = false;
  deleteCurrent(); // We delete the current tetronimo
  
  for (int i=0;i<4;i++) // We offset all the pieces
  {
    auxTilePosX[i] = offsetX(tilePosX[i], direction);
    if (!isValid(tilePosY[i], auxTilePosX[i]))collision = true; // If a movement is not valid, then there's a collision going on
  }

  if (collision) // If there's a collision then we don't move anything
  {
    for (int i=0;i<4;i++)
    {
      updateTile(i,tilePosX[i], tilePosY[i]);
    }
  }
  else
  {
    for (int i=0;i<4;i++) // Otherwise we update with the new position
    {
      tilePosX[i] = auxTilePosX[i];
      updateTile(i, tilePosX[i], tilePosY[i]);
    }
  }
}

// This functions offsets all the pieces in the Y direction
void allOffset(int yBreak)
{

  for (int i=yBreak; i>=0; i--)
  {
    for (int j=LENGTH - 1; j>=0; j--)
    {
      if (i==0)break;
      leds[arrayConverter(i,j)] = leds[arrayConverter(i-1,j)];
      taken[i][j] = taken[i-1][j];
    }
    FastLED.show();
  }
}

// We level up if the totalLines are greater than the objective
void levelUp()
{
    if (totalLines >= linesObj)
    {
      currentLevel++;
      totalLines = 0;
      linesObj += 5;
      //targetMillis =- 150;
    }
    //Serial.print("Target: ");
    //Serial.println(targetMillis);
   
}

// We check if we have a line
void isLine()
{
  int nLines = 0;
  for (int i=0;i<WIDTH;i++)
    {
      // If al tetronimos in a line are taken, then there's a line.
        int tetronimo_count = 0;
        for (int j=0;j<LENGTH;j++)
        {
          if (taken[i][j])tetronimo_count++;
            // if (isalpha(game_field[i][j]))tetronimo_count++;
        }
        if (tetronimo_count == LENGTH)
        {
            for (int j=0;j<LENGTH;j++) // We delete that line 
            {
              leds[arrayConverter(i,j)] = CRGB::Black;
              delay(100);
              FastLED.show();
            }
            nLines++;
            allOffset(i); // Offset everything above the line
        }
    }

    totalLines += nLines;

    switch (nLines) // We calculte the score
    {
        case 1: score += (40 * (currentLevel + 1)); generateTetrino(); break;
        case 2: score += (200 * (currentLevel + 1)); generateTetrino(); break;
        case 3: score += (300 * (currentLevel + 1)); generateTetrino(); break;
        case 4: score+= (1200 * (currentLevel + 1)); generateTetrino(); break;
        default: generateTetrino(); break;
    }

    levelUp();
}

// This function moves a piece in the Y-axis
void movementY(int direction)
{
    bool end = false;
    bool collision = false;
    deleteCurrent(); // Delete current tetronimo
    for (int i=0;i<4;i++)
    {
        newTilePosY[i] = offsetY(tilePosY[i],direction);
        // if (isEndOfField(newTilePosY[i]))end = true;
        if (!isValid(newTilePosY[i], tilePosX[i]))collision = true; //If a movement is not valid, then there's a collision
    }
    for (int i=0;i<4;i++)
    {
        if (collision)
        {
            updateTile(i,tilePosX[i], newTilePosY[i] - 1);
            end = true; // We end the current tetronimo
        }
        else
        {
            updateTile(i,tilePosX[i], newTilePosY[i]); // We simple update to the new position
        }
    }
    if (end)isLine(); // We check if we formed a line
}


// This funcion helps us with the instant down feature
void down()
{

  // We look for the highest y
  int maxY = -1;
  for (int i=0;i<4;i++)
  {
    if (tilePosY[i] > maxY)maxY = tilePosY[i];
  }

  int collisions[4] = {999};
  int yPositions[4] = {999};
  int j = 0;

  // We will do scans for every block below each piece. The highest collision is the objective
  for (int i=0;i<4;i++)
  {    
      if (!taken[tilePosY[i]+1][tilePosX[i]])
      {
        int currentYScan = tilePosY[i];
        while (true)
        {
           currentYScan++;
           if (!isValid(currentYScan, tilePosX[i])) // isValid Y X
           {
             //Serial.print("Not Valid! ");
             //Serial.print("Init Y: ");
             //Serial.println(tilePosY[i]);
             //Serial.println(currentYScan - 1);
             collisions[j] = currentYScan - 1;
             yPositions[j] = tilePosY[i];
             j++; 
             break;
          }
        }
      }
   }
  /*
      if (tilePosY[i] == maxY)
      {
        int currentYScan = tilePosY[i];
        
        // Start looking for the first collision
        while (true)
        {
          currentYScan++;
          if (!isValid(currentYScan, tilePosX[i])) // isValid Y X
          {
            Serial.print("Not Valid! ");
            Serial.println(currentYScan - 1);
            collisions[j] = currentYScan - 1;
            j++;
            break;
          }
        }
      }
    }
    */
  deleteCurrent();
  
  // Now we need to look which collision index is the lowest
  int lowerCollision = 998;
  int startingYValue = 998;
  for (int i=0;i<j;i++)
  {
    if (collisions[i] < lowerCollision)
    {
      lowerCollision = collisions[i];
      startingYValue = yPositions[i];
    }
  }
  
  int newYPos[4];
 // Serial.print("lowerCollision: ");
  //Serial.println(lowerCollision);
  //Serial.print("Starting y value: ");
  //Serial.println(startingYValue);

   // We update our new value
  int yOffset = lowerCollision - startingYValue;
  for (int i=0;i<4;i++)
  {
    newYPos[i] = tilePosY[i] + yOffset;
    //Serial.print(tilePosX[i]);
    //Serial.print(" ");
    //Serial.println(newYPos[i]);
    updateTile(i,tilePosX[i], newYPos[i]);
  }
  isLine(); // We check if we did a line
}

// We start the game displaying T - E - T - R - I - S
void initGame()
{
  // Colors
  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = CRGB(255/WIDTH * (i+1), 0, 0);
    }
    delay(20);
    FastLED.show();
  }

  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = CRGB(0, 255/WIDTH * (i+1), 0);
    }
    delay(20);
    FastLED.show();
  }

  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = CRGB(0, 0, 255/WIDTH * (i+1));
    }
    delay(20);
    FastLED.show();
  }

  // T
  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = _initT_02[i][j];
    }
  }

  delay(500);
  FastLED.show();
  
  // E
  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = _initE[i][j];
    }
  }
  
  delay(500);
  FastLED.show();

  // T
  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = _initT_02[i][j];
    }
  }
  delay(500);
  FastLED.show();

  // R
  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = _initR[i][j];
    }
  }
  delay(500);
  FastLED.show();

  // I
  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = _initI[i][j];
    }
  }
  delay(500);
  FastLED.show();

  // S
  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = _initS[i][j];
    }
  }
  delay(500);
  FastLED.show();
  delay(500);
  
  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = CRGB(255/WIDTH * (i+1), 0, 0);
    }
    delay(20);
    FastLED.show();
  }

  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = CRGB(0, 255/WIDTH * (i+1), 0);
    }
    delay(20);
    FastLED.show();
  }

  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = CRGB(0, 0, 255/WIDTH * (i+1));
    }
    delay(20);
    FastLED.show();
  }

  for (int i=0;i<WIDTH;i++)
  {
    for (int j=0;j<LENGTH;j++)
    {
      leds[arrayConverter(i,j)] = CRGB::Black;
    }
    delay(20);
    FastLED.show();
  }
  delay(1000);
}

void (* resetFunc) (void) = 0;

void setup()
{
  delay(2000);

  servo.attach(9,SERVO_MIN,SERVO_MAX);

  FastLED.addLeds<WS2812,LED_PIN,GRB>(leds, 60); // Agregamos los LEDs
  
  pinMode(9,OUTPUT);

  Serial.begin(9600);
  randomSeed(analogRead(2));
  nextTetrino = random(0,6);
  generateTetrino();
  printState();
  score = 0;
  lastScore = -1;
  targetMillis = 1500;
  delayTime = 100; 
  rotationTime = 200;
  pieceDownTimer.restart();
  moveDelayTimer.restart();
  rotationDelayTimer.restart();
  downDelayTimer.restart();

  servo.write(map(90, 0, 180, SERVO_MIN, SERVO_MAX));

  initGame();

  pinMode(12,INPUT_PULLUP);
  //Serial.println("Score: " + score);
  //Serial.println("Current Level"  + currentLevel);
}


// We read the analog stick input and according to that we do movement x / movement y / rotate / instant down
void loop() 
{
  if (score != lastScore)
  {
    //Serial.println("Score: " + score);
    lastScore = score;
  }

  if (currentLevel != lastLevel)
  {
    //Serial.println("Current Level: " + currentLevel);
    lastLevel = currentLevel;
  }

  /*
   * Stick Movement
   * y 0 --------- y 1024
   */
   
   stickY = analogRead(STICK_Y);
   stickX = analogRead(STICK_X);

   if (moveDelayTimer.ms() > delayTime)
   {
    if (stickY > 900)
    {
     // Serial.println("Right");
      movementX(-1);
    }
    else if (stickY < 100)
    {
    //  Serial.println("Left");
      movementX(1);
    }
    moveDelayTimer.restart();
   }
   
   if (rotationDelayTimer.ms() > rotationTime && stickX > 1000)
   {
  //  Serial.println("Rotation");
    rotationDelayTimer.restart();
    rotation();
   }
   else if (downDelayTimer.ms() > 500 && stickX < 100)
   {
  //  Serial.println("Going down");
    downDelayTimer.restart();
    down();
   }


  /*
   
  if (rightMovement.getSingleDebouncedPress())
  {
    Serial.println("Right");
    movementX(1);
  }
  else if (leftMovement.getSingleDebouncedPress())
  {
    Serial.println("Left");
    movementX(-1);
  }
  else if (rotateMovement.getSingleDebouncedPress())
  {
    Serial.println("Rotation");
    rotation();
  }

  */
  
  if (lastScore != score)Serial.println(score);

  if (pieceDownTimer.ms() > targetMillis)
  {
    Serial.println("Movement Y");
    Serial.print("Target Millis: ");
    Serial.println(targetMillis);
    movementY(1);
    pieceDownTimer.restart();
  }
  
  lastScore = score;
  
  printState();
}
