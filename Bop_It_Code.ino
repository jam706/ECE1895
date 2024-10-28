#include <LiquidCrystal_I2C.h> //LCD library, utilizes I2C communication, pins A4 and A5
#include <Wire.h>
#include <FastLED.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); //common address, 16x2 LCD

#define NUM_LEDS 12
#define DATA_PIN 5
CRGB leds[NUM_LEDS];



//VARIABLES
const int MAX_LEVEL = 99;
int sequence[MAX_LEVEL];
int your_sequence[MAX_LEVEL];
int level = 1;
int score = 0;
int speedTimer = 2000; //milliseconds
bool lost = false;
const int buzzerPin = 9;
const int hornButtonPin = 0;
const int turnSignalRightPin = 2; 
const int turnSignalLeftPin = 1;
bool inputRecieved = false;
unsigned long startTime;
bool dirRight;

//SETUP
void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
  //lcd.begin(); // Initialize LCD
  lcd.backlight(); //Turn on the backlight
  lcd.setCursor(0, 0); //Set the cursor beginning
  pinMode(buzzerPin, OUTPUT); //establish buzzer pin
}


//LOOP
void loop() {
  lcd.setCursor(0, 0);      // Set the cursor to the first row, first column
  if(level == 1) {
    generateSequence();
  }

  if(score<99 && lost==false) {
    if(sequence[level] == 2) {          //2 = horn
      hornCommand();
    } else if (sequence[level] == 3) {  //3 = signal
      signalCommand();
    } else {                            //4 = turn
      turnCommand();
    }
    level++;
    score++;
    speedTimer-=10;
  }
  if(score == 99) {
    winSequence();
  } else if (lost == true) {
    lostSequence();
  }

}

void generateSequence() {
  randomSeed(millis());

  for(int i = 0; i<3; i++) {
    sequence[i] = random(2, 5); // 2 = horn, 3 = signal, 4 = turn
  }
}

void hornCommand() {
  lcd.print("Honk Horn!"); //display "honk horn" on screen
  hornTone(); //buzz horn command
  startTime = millis();
  while(!inputRecieved && millis()-startTime < speedTimer) {  //wait for user input
    if (digitalRead(hornButtonPin) == HIGH)
      inputRecieved = true;
  }
  if(!inputRecieved) { //if horn not pressed within timer, lost=true
    lost = true;
  }
  inputRecieved = false;
}


void signalCommand() {
  lcd.print("Signal!"); //display "signal" on screen
  signalTone(); //buzz signal command
  randomDirection(); //randomly display right or left LED
  startTime = millis();
  while(!inputRecieved && millis()-startTime < speedTimer) {  //wait for user input
    if ((digitalRead(turnSignalRightPin == HIGH && dirRight == true)) || (turnSignalLeftPin == HIGH && dirRight == false)) { //if correct input read
      inputRecieved = true;
    }
    if ((digitalRead(turnSignalRightPin == HIGH && dirRight == false)) || (turnSignalLeftPin == HIGH && dirRight == true)) { //incorrect input read
      startTime = 0; // this will make it so the user cannot try both inputs
    }
  }
  if(!inputRecieved) { //if signal not set within timer (or wrong signal indicated), lost=true
    lost = true;
  }
  inputRecieved = false;
}

void turnCommand() {
  lcd.print("Turn"); //display "turn" on screen
  turnTone(); //buzz turn command
  randomDirection(); //randomly display right or left LED
  //if wheel not turned within timer or wrong direction turned, lost=true
}

void winSequence() {
  lcd.print("You Win!"); //display "you win" on screen
  winLights(); //led win sequence
  winTone(); //buzzer win sequence
  //delay(1000);

}

void lostSequence() {
  lcd.print("Final Score: "); 
  lcd.print(score); //display final score on screen
  loseLights(); //led lose sequence
  loseTone(); //buzzer lose sequence
  //delay(1000);
}

void hornTone() {
  tone(buzzerPin, 440); // A4
  delay(500);
  noTone(buzzerPin);
}

void signalTone() {
  tone(buzzerPin, 523); // C5
  delay(200);           
  noTone(buzzerPin);
  delay(100);
  tone(buzzerPin, 523);
  delay(200);
  noTone(buzzerPin);
}

void turnTone() {
  tone(buzzerPin, 330); // E4
  delay(200);
  noTone(buzzerPin);
  delay(100);           
  tone(buzzerPin, 392); // G4
  delay(200);
  noTone(buzzerPin);
  delay(100);           
  tone(buzzerPin, 523); // C5
  delay(200);
  noTone(buzzerPin);
}

void winTone() {
tone(buzzerPin, 523); // C5
delay(150);
noTone(buzzerPin);
delay(50);
tone(buzzerPin, 659); // E5
delay(150);
noTone(buzzerPin);
delay(50);
tone(buzzerPin, 784); // G5
delay(150);
noTone(buzzerPin);
delay(50);
tone(buzzerPin, 1047); // C6
delay(300);
noTone(buzzerPin);
}

void loseTone() {
tone(buzzerPin, 523); // C5
delay(300);
noTone(buzzerPin);
delay(100);
tone(buzzerPin, 494); // B4
delay(300);
noTone(buzzerPin);
delay(100);
tone(buzzerPin, 440); // A4
delay(300);
noTone(buzzerPin);
delay(100);
tone(buzzerPin, 392); // G4
delay(400);
noTone(buzzerPin);
}

void randomDirection() {
  FastLED.clear();
  int half = random (0,2);

  if(half == 0) {
    for (int i = 0; i < NUM_LEDS / 2; i++) {
      leds[i] = CRGB::Blue;
      dirRight = false;
    }
  } else {
    for (int i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Blue;
      dirRight = true;
    }
  }
  FastLED.show();
  delay(500);
  FastLED.clear();

}

void winLights() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(500);

  FastLED.clear();
  FastLED.show();
  delay(500);
  FastLED.clear();
}

void loseLights() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(500);

  FastLED.clear();
  FastLED.show();
  delay(500);
  FastLED.clear();
}
