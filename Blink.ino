#include <FastLED.h>

//VARIABLES
const int MAX_LEVEL = 99;
int sequence[MAX_LEVEL];
int your_sequence[MAX_LEVEL];
int level = 1;
int score = 0;
int speedTimer = 2000; //milliseconds
bool lost = false;

//SETUP
void setup() {
  // put your setup code here, to run once:
}


//LOOP
void loop() {
  if(level == 1) {
    generateSequence();
  }

  if(score<99 && lost==false) {
    if(sequence[level] == 2) {          //2 = horn
      //hornCommand()
    } else if (sequence[level] == 3) {  //3 = signal
      //signalCommand()
    } else {                            //4 = turn
      //turnCommand()
    }
    level++
    score++
    speedTimer-=10;
  }
  if(score == 99) {
    //winSequence()
  } else if (lost == true) {
    //lostSequence()
  }

}

void generateSequence() {
  randomSeed(millis());

  for(int i = 0; i<3; i++) {
    sequence[i] = random(2, 5); // 2 = horn, 3 = signal, 4 = turn
  }
}
