#include <LiquidCrystal_I2C.h> // LCD library, utilizes I2C communication, pins A4 and A5
#include <Wire.h>
#include <FastLED.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // common address, 16x2 LCD

#define NUM_LEDS 12
#define DATA_PIN 5
CRGB leds[NUM_LEDS];

// AS5600 Setup
#define AS5600_ADDR 0x36
#define AS5600_RAW_ANGLE 0x0C

// Stepper motor control pins
#define STEP_PIN 6         // Step control on PD6
#define DIR_PIN 8          // Direction control on PB0
#define EN_PIN 7           // Enable control on PD7

// Game States
enum GameState { IDLE, START_GAME, HORN_COMMAND, SIGNAL_COMMAND, TURN_COMMAND, CHECK_WIN, LOST_SEQUENCE };
GameState gameState = IDLE;

// VARIABLES
const int MAX_LEVEL = 99;
int sequence[MAX_LEVEL];
int level = 0;
int score = 0;
int speedTimer = 3000; // milliseconds
bool dirRight;
bool inputReceived = false;
unsigned long startTime;
long cumulativePosition = 0;
long originPosition = 0;

// Pins
const int buzzerPin = 9;
const int hornButtonPin = 0;
const int turnSignalRightPin = 2;
const int turnSignalLeftPin = 1;
const int startPin = 3;

// Direction thresholds
const int leftTurnThreshold = -3000;
const int rightTurnThreshold = 3000;

// SETUP
void setup() {
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  lcd.init();
  lcd.backlight();
  pinMode(buzzerPin, OUTPUT);
  pinMode(startPin, INPUT_PULLUP);
  lcd.setCursor(0, 0);
  lcd.print("Press Start");

  Wire.begin();

  // Stepper motor setup - disable the motor
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); // Set enable pin HIGH to disable the motor
}

void loop() {
  int position = readAS5600();
  trackRotations(position);
  long relativePosition = cumulativePosition - originPosition;

  switch (gameState) {
    case IDLE:
      if (digitalRead(startPin) == HIGH) {
        startGame();
        gameState = START_GAME;
      }
      break;

    case START_GAME:
      generateSequence();
      gameState = static_cast<GameState>(sequence[level]);
      break;

    case HORN_COMMAND:
      hornCommand();
      break;

    case SIGNAL_COMMAND:
      signalCommand();
      break;

    case TURN_COMMAND:
      turnCommand();
      break;

    case CHECK_WIN:
      if (score >= 99) {
        winSequence();
        gameState = IDLE;
      } else if (inputReceived) {
        level++;
        score++;
        speedTimer -= 20;
        gameState = static_cast<GameState>(sequence[level]);
      }
      break;

    case LOST_SEQUENCE:
      lostSequence();
      gameState = IDLE;
      break;
  }
}

void startGame() {
  level = 0;
  score = 0;
  speedTimer = 3000;
  inputReceived = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Started!");

  originPosition = cumulativePosition;

  delay(1000);
}

void generateSequence() {
  randomSeed(millis());
  for (int i = 0; i < MAX_LEVEL; i++) {
    sequence[i] = random(HORN_COMMAND, TURN_COMMAND + 1);
  }
}

// AS5600 Functions
int readAS5600() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AS5600_RAW_ANGLE);
  Wire.endTransmission();
  Wire.requestFrom(AS5600_ADDR, 2);
  if (Wire.available() == 2) {
    int highByte = Wire.read();
    int lowByte = Wire.read();
    return (highByte << 8) | lowByte;
  }
  return 0;
}

void trackRotations(int currentPosition) {
  const int wrapThreshold = 2048;
  int positionDifference = currentPosition - cumulativePosition;

  if (positionDifference > wrapThreshold) {
    cumulativePosition -= (4096 - positionDifference);
  } else if (positionDifference < -wrapThreshold) {
    cumulativePosition += (4096 + positionDifference);
  } else {
    cumulativePosition += positionDifference;
  }
}

void displayCountdownBar(unsigned long elapsedTime) {
  int progress = map(elapsedTime, 0, speedTimer, 16, 0); // Map elapsed time to 16 segments
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(i < progress ? char(255) : ' '); // Use a solid block for filled sections
  }
}

void hornCommand() {
  lcd.clear();
  lcd.print("Honk Horn!");
  tone(buzzerPin, 440);
  delay(500);
  noTone(buzzerPin);
  
  digitalWrite(EN_PIN, LOW); // Enable stepper motor

  startTime = millis();
  inputReceived = false;

  // Countdown timer with display update
  while (!inputReceived && millis() - startTime < speedTimer) {
    if (digitalRead(hornButtonPin) == HIGH) {
      inputReceived = true;
      gameState = CHECK_WIN;
      return;
    }

    // Update countdown bar on LCD
    displayCountdownBar(millis() - startTime);
  }

  if (!inputReceived) gameState = LOST_SEQUENCE;
}

void signalCommand() {
  lcd.clear();
  dirRight = random(0, 2);
  lcd.print(dirRight ? "Signal Right!" : "Signal Left!");

  // Set LED indicators based on direction
  if (dirRight) {
    for (int i = 0; i < NUM_LEDS / 2; i++) leds[i] = CRGB::Blue;
    for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  } else {
    for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) leds[i] = CRGB::Blue;
    for (int i = 0; i < NUM_LEDS / 2; i++) leds[i] = CRGB::Black;
  }
  FastLED.show();

  signalTone();
  digitalWrite(EN_PIN, LOW); // Enable stepper motor

  startTime = millis();
  inputReceived = false;

  int buttonPin = dirRight ? turnSignalRightPin : turnSignalLeftPin;
  while (!inputReceived && millis() - startTime < speedTimer) {
    if (digitalRead(buttonPin) == HIGH) {
      inputReceived = true;
      FastLED.clear();
      FastLED.show();
      gameState = CHECK_WIN;
      return;
    }

    // Update countdown bar on LCD
    displayCountdownBar(millis() - startTime);
  }

  if (!inputReceived) gameState = LOST_SEQUENCE;
}

void turnCommand() {
  lcd.clear();
  dirRight = random(0, 2);
  lcd.print(dirRight ? "Turn Right!" : "Turn Left!");

  // Set LED indicators based on direction
  if (dirRight) {
    for (int i = 0; i < NUM_LEDS / 2; i++) leds[i] = CRGB::Red;
    for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  } else {
    for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) leds[i] = CRGB::Red;
    for (int i = 0; i < NUM_LEDS / 2; i++) leds[i] = CRGB::Black;
  }
  FastLED.show();

  digitalWrite(EN_PIN, HIGH); // Disable stepper motor for free spinning

  long targetPosition = dirRight ? rightTurnThreshold : leftTurnThreshold;
  startTime = millis();
  inputReceived = false;

  // Step 1: Check for turning in the specified direction
  while (!inputReceived && millis() - startTime < speedTimer) {
    int position = readAS5600();
    trackRotations(position);
    long relativePosition = cumulativePosition - originPosition;

    if ((dirRight && relativePosition > targetPosition) ||
        (!dirRight && relativePosition < targetPosition)) {
      inputReceived = true;
    }

    // Update countdown bar on LCD
    displayCountdownBar(millis() - startTime);
  }

  if (!inputReceived) {
    gameState = LOST_SEQUENCE;
    return;
  }

  // Step 2: Prompt user to return to center position
  lcd.clear();
  lcd.print("Back to Center");

  FastLED.clear();
  FastLED.show();

  startTime = millis();
  inputReceived = false;

  while (!inputReceived && millis() - startTime < speedTimer) {
    int position = readAS5600();
    trackRotations(position);
    long relativePosition = cumulativePosition - originPosition;

    if (abs(relativePosition) < 100) {
      inputReceived = true;
    }

    // Update countdown bar on LCD
    displayCountdownBar(millis() - startTime);
  }

  digitalWrite(EN_PIN, LOW); // Re-enable stepper motor after return to center

  if (!inputReceived) {
    gameState = LOST_SEQUENCE;
  } else {
    gameState = CHECK_WIN;
  }
}

void winSequence() {
  lcd.clear();
  lcd.print("You Win!");
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Green;
  FastLED.show();
  tone(buzzerPin, 1047);
  delay(1000);
  noTone(buzzerPin);
}

void lostSequence() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Over");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);  // Display the final score on the second row

  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Red;
  FastLED.show();
  tone(buzzerPin, 392);
  delay(1000);
  noTone(buzzerPin);
}

void signalTone() {
  tone(buzzerPin, 523); // C5
  delay(200);
  noTone(buzzerPin);
  delay(100);
  tone(buzzerPin, 523); // C5
  delay(200);
  noTone(buzzerPin);
}
