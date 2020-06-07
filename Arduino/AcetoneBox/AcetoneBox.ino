const uint8_t SETTINGS_FAN_ENABLE_SECONDS = 30; // < 59
const uint8_t SETTINGS_RUN_FAN_EVERY_X_MIN = 5; // < 59
const bool SETTINGS_COMMON_ANODE = true; // anode is +
const uint8_t SETTINGS_MOTOR_START_POWER = 50;
const uint8_t SETTINGS_MOTOR_MAX_POWER = 255;

const uint8_t PIN_BUZZER = A5;
const uint8_t PIN_BUTTON = A4;
const uint8_t PIN_MOTOR = 3;
const uint8_t PIN_7SEG_DIGITS [4] = {10, 11, 9, 12};
const uint8_t PIN_7SEG_CHARS [8] = {4, A0, 8, 7, 5, A1, 13, 6};

const uint8_t CHARS[11] = {
  //ZABCDEFG
  0b10000001,
  0b11001111,
  0b10010010,
  0b10000110,
  0b11001100,
  0b10100100,
  0b10100000,
  0b10001111,
  0b10000000,
  0b10000100,
  0b11001000
};

class Button {
  public:
    uint8_t pin = 2;
    bool inversed = false;
    bool pullup = false;
    bool isPressed = false;
    bool isPressedOld = false;
    uint32_t pressTime;
    uint32_t releaseTime;
    bool isPressProcessed = true;
    bool isReleaseProcessed = true;
    Button(){}
    Button(uint8_t pinIn, bool inversedIn, bool pullupIn){
      pin = pinIn;
      inversed = inversedIn;
      pullup = pullupIn;
    }
    
    bool pressProcessed() {
      if (!isPressProcessed){
        isPressProcessed = true;
        return false;
      }
      return true;
    }
    bool releaseProcessed() {
      if (!isReleaseProcessed){
        isReleaseProcessed = true;
        return false;
      }
      return true;
    }
};

const uint8_t btnsCount = 1;
Button btns[btnsCount];

void buttonsCheck() {
   for (uint8_t i = 0; i < btnsCount; i++) {
    btns[i].isPressed = digitalRead(btns[i].pin);
    if (btns[i].inversed)
      btns[i].isPressed = !btns[i].isPressed;
      
    if (btns[i].isPressed && !btns[i].isPressedOld) {
      btns[i].pressTime = millis();
      btns[i].isReleaseProcessed = false;
    } else
    if (!btns[i].isPressed && btns[i].isPressedOld) {
      btns[i].releaseTime = millis();
      btns[i].isPressProcessed = false;
    }
    btns[i].isPressedOld = btns[i].isPressed;
  }
}

bool buzzerEnabled = false;
uint32_t buzzerTimer = 0;
uint32_t buzzerDelay = 500;
bool buzzerBeep = false;

uint32_t segTimer = 0;
uint32_t segDelay = 1;
uint8_t segDigitNow = 0;
uint8_t segMode = 0;
uint8_t hours = 0;
uint8_t minutes = 15;
uint8_t seconds = 59;
uint32_t countdownTimer = 0;
uint32_t countdownDelay = 1000;

void doChar() {
  if (segMode == 0) {
    if (segDigitNow == 0) {
      for (uint8_t i = 0; i < 8; i++)
        digitalWrite(PIN_7SEG_CHARS[i], (CHARS[10] >> i) & 1 == SETTINGS_COMMON_ANODE);
    } else if (segDigitNow == 1){
      for (uint8_t i = 0; i < 8; i++)
        digitalWrite(PIN_7SEG_CHARS[i], (CHARS[hours] >> i) & 1 == SETTINGS_COMMON_ANODE);      
      digitalWrite(PIN_7SEG_CHARS[7], !SETTINGS_COMMON_ANODE);      
    } else if (segDigitNow == 2){
      for (uint8_t i = 0; i < 8; i++)
        digitalWrite(PIN_7SEG_CHARS[i], (CHARS[(minutes / 10) % 10] >> i) & 1 == SETTINGS_COMMON_ANODE);
    } else if (segDigitNow == 3){
      for (uint8_t i = 0; i < 8; i++)
        digitalWrite(PIN_7SEG_CHARS[i], (CHARS[minutes % 10] >> i) & 1 == SETTINGS_COMMON_ANODE);
    }
  } else if (segMode == 1) {
    if (segDigitNow == 0){
      for (uint8_t i = 0; i < 8; i++)
        digitalWrite(PIN_7SEG_CHARS[i + 1], (~(59 - seconds) >> i) & 1 == SETTINGS_COMMON_ANODE);     
      digitalWrite(PIN_7SEG_CHARS[0], SETTINGS_COMMON_ANODE);            
    } else if (segDigitNow == 1){
      for (uint8_t i = 0; i < 8; i++)
        digitalWrite(PIN_7SEG_CHARS[i], (CHARS[hours] >> i) & 1 == SETTINGS_COMMON_ANODE);      
      digitalWrite(PIN_7SEG_CHARS[7], !SETTINGS_COMMON_ANODE);      
    } else if (segDigitNow == 2){
      for (uint8_t i = 0; i < 8; i++)
        digitalWrite(PIN_7SEG_CHARS[i], (CHARS[(minutes / 10) % 10] >> i) & 1 == SETTINGS_COMMON_ANODE);
    } else if (segDigitNow == 3){
      for (uint8_t i = 0; i < 8; i++)
        digitalWrite(PIN_7SEG_CHARS[i], (CHARS[minutes % 10] >> i) & 1 == SETTINGS_COMMON_ANODE);
    }
  } else {
    for (uint8_t i = 0; i < 8; i++)
      digitalWrite(PIN_7SEG_CHARS[i], !buzzerBeep);    
  }
}

void doSegments(){
  digitalWrite(PIN_7SEG_DIGITS[segDigitNow], !SETTINGS_COMMON_ANODE);
  segDigitNow++;
  if (segDigitNow == 4)
    segDigitNow = 0;
  doChar();
  digitalWrite(PIN_7SEG_DIGITS[segDigitNow], SETTINGS_COMMON_ANODE);
}

uint8_t motorPower = 0;
uint32_t motorTimer = 0;
uint32_t motorDelay = 20;
bool motorEnabled = false;
uint8_t senods_for_fan = 60 - SETTINGS_FAN_ENABLE_SECONDS;
void motorWork() {
  if (segMode == 1) {
    if (millis() > motorTimer) {
      motorTimer = millis() + motorDelay;
      if (minutes % SETTINGS_RUN_FAN_EVERY_X_MIN == 0 && seconds > senods_for_fan)
        motorEnabled = true;
      else
        motorEnabled = false;
      if (motorEnabled) {
        if (motorPower < SETTINGS_MOTOR_MAX_POWER)
          motorPower++;
        if (motorPower < SETTINGS_MOTOR_START_POWER)
          motorPower = SETTINGS_MOTOR_START_POWER;
      } else {
        if (motorPower > 0)
          motorPower--;
        if (motorPower < SETTINGS_MOTOR_START_POWER)
          motorPower = 0;    
      }
      analogWrite(PIN_MOTOR, motorPower);
    }
  } else {
    analogWrite(PIN_MOTOR, 0);
  }
}

void setup() {
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  btns[0] = Button(PIN_BUTTON, true, true);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(PIN_7SEG_CHARS[i], OUTPUT);
    digitalWrite(PIN_7SEG_CHARS[i], 1);
  }
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(PIN_7SEG_DIGITS[i], OUTPUT);
    digitalWrite(PIN_7SEG_DIGITS[i], 0);
  }
  delay(500);
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(PIN_7SEG_DIGITS[i], 1);
  }
}

void loop() {
  buttonsCheck();
  motorWork();
  if (segMode == 0){
    if (!btns[0].pressProcessed()) {
      if (btns[0].releaseTime - btns[0].pressTime > 10) {
        if (btns[0].releaseTime - btns[0].pressTime < 1000) {
          minutes += 15;
          if (minutes > 59) {
            hours++;
            minutes = 0;
          }
          if (hours > 3){
            hours = 0;
            minutes =  15;
          }
        } else {
          segMode = 1;
          seconds = 59;
          countdownTimer = millis();
        }
      }
    }
  }
  if (segMode == 1) {
    if (millis() > countdownTimer) {
      countdownTimer += countdownDelay;
      if (seconds > 0)
        seconds--;
      else {
        if (minutes > 0){
          minutes--;
          seconds = 59; 
        } else {
          if (hours > 0) {
            seconds = 59; 
            minutes = 59;
            hours--;
          } else {
            segMode = 2;
          }
        }
      }
    }
  }
  if (segMode == 2){
    if (millis() > buzzerTimer) {
      buzzerTimer = millis() + buzzerDelay;
      buzzerBeep = !buzzerBeep;
      digitalWrite(PIN_BUZZER, buzzerBeep);
    }
  }
  if (millis() > segTimer){
    segTimer = millis() + segDelay;
    doSegments();
  }
}
