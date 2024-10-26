#include <LiquidCrystal_I2C.h> //LCD library, utilizes I2C communication, pins A4 and A5
#include <Wire.h>
#include <FastLED.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); //common address, 16x2 LCD

//VARIABLES
const int MAX_LEVEL = 99;
int sequence[MAX_LEVEL];
int your_sequence[MAX_LEVEL];
int level = 1;
int score = 0;
int speedTimer = 2000; //milliseconds
bool lost = false;
//const buzzerPin = 

//SETUP
void setup() {
  // put your setup code here, to run once:
  lcd.begin(); // Initialize LCD
  lcd.backlight(); //Turn on the backlight
  lcd.setCursor(0, 0); //Set the cursor beginning
  // pinMode(buzzerPin, OUTPUT); //establish buzzer pin (?)
}


//LOOP
void loop() {
  lcd.setCursor(0, 0);      // Set the cursor to the first row, first column
  if(level == 1) {
    generateSequence();
  }

  if(score<99 && lost==false) {
    if(sequence[level] == 2) {          //2 = horn
      hornCommand()
    } else if (sequence[level] == 3) {  //3 = signal
      signalCommand()
    } else {                            //4 = turn
      turnCommand()
    }
    level++
    score++
    speedTimer-=10;
  }
  if(score == 99) {
    winSequence()
  } else if (lost == true) {
    lostSequence()
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
  delay(1000);
  //if horn not pressed within timer, lost=true
}


void signalCommand() {
  lcd.print("Signal!"); //display "signal" on screen
  //randomly display right or left LED
  signalTone(); //buzz signal command
  delay(1000);
  //if signal not indicated within timer or wrong direction signaled, lost=true
}

void turnCommand() {
  lcd.print("Turn"); //display "turn" on screen
  //randomly display right or left LED
  turnTone(); //buzz turn command
  delay(1000);
  //if wheel not turned within timer or wrong direction turned, lost=true
}

void winSequence() {
  lcd.print("You Win!"); //display "you win" on screen
  //led win sequence
  winTone(); //buzzer win sequence
  delay(1000);

}

void loseSequence() {
  lcd.print("Final Score: "); 
  lcd.print(score); //display final score on screen
  //led lose sequence
  loseTone(); //buzzer lose sequence
  delay(1000);
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

void winTone {
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

void loseTone {
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
