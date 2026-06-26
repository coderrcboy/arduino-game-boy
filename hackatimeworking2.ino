#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin configurations
const int BUTTON_PIN = 3;

// Game variables
float birdY = 32;
float velocity = 0;
float gravity = 0.4;
float jump = -3.5;

int pipeX = 128;
int gapY = 20;
const int GAP_HEIGHT = 22;
const int PIPE_WIDTH = 12;

int score = 0;
int highScore = 0;
bool gameOver = false;
bool gameStarted = false;

// EEPROM Address for High Score
const int EEPROM_HIGH_SCORE_ADDR = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize OLED display (Address 0x3C is common for 0.96" OLEDs)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;); // Don't proceed, loop forever
  }
  
  // Read high score from EEPROM memory
  EEPROM.get(EEPROM_HIGH_SCORE_ADDR, highScore);
  // If EEPROM was empty/corrupted, default to 0
  if (highScore < 0 || highScore > 9999) {
    highScore = 0;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  randomSeed(analogRead(0));
}

void loop() {
  display.clearDisplay();

  if (!gameStarted) {
    showStartScreen();
  } else if (gameOver) {
    showGameOverScreen();
  } else {
    runGamePhysics();
    drawGameObjects();
  }

  display.display();
  delay(20); // Maintain roughly 50 FPS
}

void showStartScreen() {
  display.setTextSize(2);
  display.setCursor(20, 10);
  display.print(F("FLAPPY BIRD"));
  
  display.setTextSize(1);
  display.setCursor(25, 38);
  display.print(F("High Score: "));
  display.print(highScore);
  
  display.setCursor(15, 52);
  display.print(F("Press Button to Start"));
  
  if (digitalRead(BUTTON_PIN) == LOW) {
    resetGame();
    gameStarted = true;
    delay(300); // Debounce
  }
}

void showGameOverScreen() {
  display.setTextSize(2);
  display.setCursor(10, 5);
  display.print(F("GAME OVER"));
  
  display.setTextSize(1);
  display.setCursor(35, 30);
  display.print(F("Score: "));
  display.print(score);
  
  display.setCursor(20, 42);
  display.print(F("High Score: "));
  display.print(highScore);
  
  display.setCursor(10, 54);
  display.print(F("Press Button to Retry"));
  
  if (digitalRead(BUTTON_PIN) == LOW) {
    resetGame();
    delay(300); // Debounce
  }
}

void resetGame() {
  birdY = 32;
  velocity = 0;
  pipeX = 128;
  gapY = random(10, SCREEN_HEIGHT - GAP_HEIGHT - 10);
  score = 0;
  gameOver = false;
}

void runGamePhysics() {
  // Handle button jump input
  if (digitalRead(BUTTON_PIN) == LOW) {
    velocity = jump;
  }
  
  // Apply gravity forces
  velocity += gravity;
  birdY += velocity;
  
  // Move obstacles left
  pipeX -= 2;
  
  // Reset obstacle position and update score if cleared
  if (pipeX < -PIPE_WIDTH) {
    pipeX = SCREEN_WIDTH;
    gapY = random(10, SCREEN_HEIGHT - GAP_HEIGHT - 10);
    score++;
  }
  
  // Collision handling: Ceilings and floors
  if (birdY < 0 || birdY > SCREEN_HEIGHT - 4) {
    endGame();
  }
  
  // Collision handling: Pipes (Bird occupies X positions 20 to 24)
  if (pipeX <= 24 && (pipeX + PIPE_WIDTH) >= 20) {
    if (birdY < gapY || birdY > (gapY + GAP_HEIGHT - 4)) {
      endGame();
    }
  }
}

void drawGameObjects() {
  // Draw flappy bird icon (represented as a 4x4 square pixel asset)
  display.fillRect(20, (int)birdY, 4, 4, SSD1306_WHITE);
  
  // Draw top obstacle pipe
  display.fillRect(pipeX, 0, PIPE_WIDTH, gapY, SSD1306_WHITE);
  
  // Draw bottom obstacle pipe
  display.fillRect(pipeX, gapY + GAP_HEIGHT, PIPE_WIDTH, SCREEN_HEIGHT - (gapY + GAP_HEIGHT), SSD1306_WHITE);
  
  // Render running scoreboard text
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.print(F("Score: "));
  display.print(score);
}

void endGame() {
  gameOver = true;
  if (score > highScore) {
    highScore = score;
    EEPROM.put(EEPROM_HIGH_SCORE_ADDR, highScore);
  }
}
