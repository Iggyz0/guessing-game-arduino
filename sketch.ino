// LEARNING IN PROGRESS / WORK IN PROGRESS
#include <TimerOne.h>

#define MAX_SEQUENCE 4
#define ERR_PIN 13

#define SHOW_SEQUENCE 400
#define WAITING_FOR_USER_INPUT 401
#define CHECK_SEQUENCE 402
#define GAME_OVER 403

const int totalLeds = 4;
const int greenPins[totalLeds] = {9, 10, 11, 12};
const int buttons[totalLeds] = {2, 3, 4, 5};

int errValue = 0;
int state;
int mistakes = 0;
int allowedMistakes = 2;
int currentSequenceLength = 2;
int sequencePointer = 0;
int score = 0;
int timeCounter = 0;
bool userPressedSomeButton = false;

int arduinoSequence[MAX_SEQUENCE] = {0};
int playerSequence[MAX_SEQUENCE] = {0};

void setupPins() {
  for (int i = 0; i < totalLeds; i++) {
    pinMode(buttons[i], INPUT);
    pinMode(greenPins[i], OUTPUT);
  }
  pinMode(ERR_PIN, OUTPUT);
}

void stateMachine() {
  switch(state) {
    case SHOW_SEQUENCE: {
      break;
    }
    case WAITING_FOR_USER_INPUT: {
      readButtonState();
      break;
    }
    case CHECK_SEQUENCE: {
      check();
      break;
    }
    case GAME_OVER: {
      if (mistakes > allowedMistakes) {
        gameOver("\nGame over! your score is: ");
      }
      else {
        gameOver("\nYou have reached the end! Your score is: ");
      }
      score = 0;
      break;
    }
    default: {
      break;
    }
  }
}

void endGameLed() {
  for(int i = 0; i < totalLeds; i++) {
    digitalWrite(greenPins[i], HIGH);
  }
  digitalWrite(ERR_PIN, HIGH);
  delay(1500);
  for(int i = 0; i < totalLeds; i++) {
    digitalWrite(greenPins[i], LOW);
  }
  digitalWrite(ERR_PIN, LOW);
}

void gameOver(char msg[50]) {
  Serial.print(msg);
  Serial.println(score);
  mistakes = 0;
  errValue = 0;
  sequencePointer = 0;
  currentSequenceLength = 2;
  endGameLed();
  generateStartingSequence();
  state = SHOW_SEQUENCE;
}

void lightLed(int led) {
  digitalWrite(led, HIGH);
  delay(200);
  digitalWrite(led, LOW);
}

void controlGame(int pressedPinPosition) {
  playerSequence[sequencePointer] = pressedPinPosition;

  if (sequencePointer == currentSequenceLength-1) {
    state = CHECK_SEQUENCE;
  }
  else {
    sequencePointer++;
  }
}

bool checkCorrectSequence() {
  for(int i = 0; i < currentSequenceLength; i++) {
    int correctButton = arduinoSequence[i];
    int pressedButton = playerSequence[i];
    if (correctButton != pressedButton) {
      return false;
    }
  }
  return true;
}

void check() {
  if (!checkCorrectSequence()) {
    mistakes++;
    Serial.print("Incorrect sequence. Tries left: ");
    Serial.println((allowedMistakes - mistakes) + 1);
    if (mistakes > allowedMistakes) {
      state = GAME_OVER;
    }
    else {
      sequencePointer = 0;
      state = SHOW_SEQUENCE;
    }
  }
  else {
    score++;
    Serial.print("Correct! Score: ");
    Serial.println(score);
    arduinoSequence[currentSequenceLength] = random(0,4);
    currentSequenceLength++;
    sequencePointer = 0;

    if (currentSequenceLength > MAX_SEQUENCE) {
      state = GAME_OVER;
    }
    else {
      state = SHOW_SEQUENCE;
    }
  }
}

void readButtonState() {
  if(digitalRead(buttons[2]) == HIGH) {
    lightLed(greenPins[2]);
    controlGame(2);
  }
  if(digitalRead(buttons[3]) == HIGH) {
    lightLed(greenPins[3]);
    controlGame(3);
  }
}

void isr0() {
  lightLed(greenPins[0]);
  controlGame(0);
}

void isr1() {
  lightLed(greenPins[1]);
  controlGame(1);
}

void timedInterrupt() {
  timeCounter++;

  if (mistakes > 0) {
    if (errValue == LOW) {
      errValue = HIGH;
    } 
    else {
      errValue = LOW;
    }
    digitalWrite(ERR_PIN, errValue);
  }

  if (timeCounter == 1 && state == SHOW_SEQUENCE) {
    if (sequencePointer > 0) {
      digitalWrite(greenPins[arduinoSequence[sequencePointer-1]], LOW);
    }
  }

  if (timeCounter == 2 && state == SHOW_SEQUENCE) {
    if (sequencePointer < currentSequenceLength) {
      digitalWrite(greenPins[arduinoSequence[sequencePointer]], HIGH);
      sequencePointer++;
    } else {
      digitalWrite(greenPins[arduinoSequence[sequencePointer]], LOW);
      sequencePointer = 0;
      state = WAITING_FOR_USER_INPUT;
    }
    timeCounter = 0;
  }

}

void generateStartingSequence() {
  arduinoSequence[0] = random(0,4);
  arduinoSequence[1] = random(0,4);
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));
  setupPins();
  generateStartingSequence();

  attachInterrupt(digitalPinToInterrupt(buttons[0]), isr0, RISING);
  attachInterrupt(digitalPinToInterrupt(buttons[1]), isr1, RISING);

  Timer1.initialize(500000);
  Timer1.attachInterrupt(timedInterrupt);

  state = SHOW_SEQUENCE;
}

void loop() {
  stateMachine();
}
