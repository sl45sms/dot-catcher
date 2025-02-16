#include "WS_Matrix.h"
#include <Arduino.h>  // Ensure random(...) is available
//Please note that the brightness of the leds bead should not be too high, which can easily cause the temperature of the board to rise rapidly, thus damaging the board !!!
uint8_t RGB_Data[3] = {30,30,30}; 
uint8_t Matrix_Data[8][8];  
Adafruit_NeoPixel pixels(RGB_COUNT, RGB_Control_PIN, NEO_RGB + NEO_KHZ800); 

// We'll use '1' to represent the white player dot and '2' to represent the green target.Yellow is the enemy, red is the wall.
uint8_t x = 4, y = 4;   // The moving white dot
uint8_t greenX = 2, greenY = 2; // The green dot's position
static uint8_t touchCount = 0; // Track how many times white dot touches green dot

// Add new function to create yellow dots (enemys)
void addYellow(uint8_t numDots) {
  for (uint8_t i = 0; i < numDots; i++) {
    uint8_t rx = random(8);
    uint8_t ry = random(8);
    if (Matrix_Data[rx][ry] == 0) {
      Matrix_Data[rx][ry] = 4; // 4 for yellow
    }
  }
}

// Simple explosion effect, then reset map
void explosionEffect() {
  // Flash everything bright a few times
  for (int i = 0; i < 3; i++){
    for (int row = 0; row < 8; row++){
      for (int col = 0; col < 8; col++){
        pixels.setPixelColor(row*8 + col, pixels.Color(255, 255, 0));
      }
    }
    pixels.show();
    delay(200);

    // Turn everything off
    for (int row = 0; row < 8; row++){
      for (int col = 0; col < 8; col++){
        pixels.setPixelColor(row*8 + col, pixels.Color(0, 0, 0));
      }
    }
    pixels.show();
    delay(200);
  }

  // Ensure hardware LEDs are cleared
  pixels.clear();
  pixels.show();

  // Reset touches and board data
  touchCount = 0;
  memset(Matrix_Data, 0, sizeof(Matrix_Data));

  // Place white dot and green dot at default spots
  x = 4;     
  y = 4;        
  greenX = 2;  
  greenY = 2;  
  Matrix_Data[x][y] = 1;
  Matrix_Data[greenX][greenY] = 2;
  
  // Refresh
  RGB_Matrix();
}

// Update RGB_Matrix to render the new yellow dot
void RGB_Matrix() {
  for (int row = 0; row < Matrix_Row; row++) {
    for (int col = 0; col < Matrix_Col; col++) {
      if (Matrix_Data[row][col] == 1) { 
        // white
        pixels.setPixelColor(row*8 + col, pixels.Color(RGB_Data[0], RGB_Data[1], RGB_Data[2]));
      } else if (Matrix_Data[row][col] == 2) {
        // green
        pixels.setPixelColor(row*8 + col, pixels.Color(0, 255, 0));
      } else if (Matrix_Data[row][col] == 3) {
        // red wall
        pixels.setPixelColor(row*8 + col, pixels.Color(255, 0, 0));
      } else if (Matrix_Data[row][col] == 4) {
        // yellow
        pixels.setPixelColor(row*8 + col, pixels.Color(255, 255, 0));
      } else {
        pixels.setPixelColor(row*8 + col, pixels.Color(0, 0, 0));
      }
    }
  }
  pixels.show();
}

void addWalls(uint8_t numWalls) {
  for (uint8_t i = 0; i < numWalls; i++) {
    uint8_t rx = random(8);
    uint8_t ry = random(8);
    // Only place a wall if the cell is empty
    if (Matrix_Data[rx][ry] == 0) {
      Matrix_Data[rx][ry] = 3; // 3 represents a red wall
    }
  }
}

void Game(uint8_t X_EN, uint8_t Y_EN) 
{
  // Save the old position
  uint8_t oldX = x;
  uint8_t oldY = y;

  // Compute the new position
  int newX = x;
  int newY = y;
  if (X_EN && Y_EN) {
    if (X_EN == 1) newX++;
    else newX--;
    if (Y_EN == 1) newY++;
    else newY--;
  } else if (X_EN) {
    if (X_EN == 1) newX++;
    else newX--;
  } else if (Y_EN) {
    if (Y_EN == 1) newY++;
    else newY--;
  }

  // Boundary checks
  if (newX < 0) newX = 0;
  if (newX > 7) newX = 7;
  if (newY < 0) newY = 0;
  if (newY > 7) newY = 7;

  // If the new position is a red wall, don't move
  if (Matrix_Data[newX][newY] == 3) {
    newX = oldX;
    newY = oldY;
  }

  // Erase the old white dot position
  Matrix_Data[oldX][oldY] = 0;

  // Update the white dot’s actual position
  x = newX;
  y = newY;

  // Check if we've touched the green dot
  if (x == greenX && y == greenY) {
    touchCount++;
    // Erase the old green dot
    Matrix_Data[greenX][greenY] = 0;
    // Pick new random position
    greenX = random(8);
    greenY = random(8);
    Matrix_Data[greenX][greenY] = 2;

    // Add red walls after 3 touches
    if (touchCount == 3) {
      addWalls(5);
    }
    // Every 5 touches after that, add more walls
    if (touchCount >= 3 && (touchCount - 3) % 5 == 0) {
      addWalls(3);
    }
    // After 10 touches, place a first yellow dot
    if (touchCount == 10) {
      addYellow(1);
    }
    // Every additional 5 times after 10, place more yellow
    if (touchCount >= 10 && (touchCount - 10) % 5 == 0) {
      addYellow(1);
    }
  }

  // If stepped on a yellow dot, do explosion, reset
  if (Matrix_Data[x][y] == 4) {
    explosionEffect();
    return; // stop here so explosion handles the reset
  }

  // Set new white dot position
  Matrix_Data[x][y] = 1;

  // Refresh the display
  RGB_Matrix();
}

void Matrix_Init() {
  pixels.begin();
  //Please note that the brightness of the leds bead should not be too high, which can easily cause the temperature of the board to rise rapidly, thus damaging the board !!!
  pixels.setBrightness(20);                       // set brightness  
  memset(Matrix_Data, 0, sizeof(Matrix_Data)); 

  // Place the white dot and green dot initially
  Matrix_Data[x][y] = 1; // White dot at (4,4)
  Matrix_Data[greenX][greenY] = 2; // Green dot at (2,2)

  // Show initial state
  RGB_Matrix();
}