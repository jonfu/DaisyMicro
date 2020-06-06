//Runs an RGB LED through a color wheel cycle

#include <EEPROM.h>

//int brightness = 0;    // how bright the LED is. Maximum value is 255
int rad = 0;

#define GND 2

#define RED12 3
#define GREEN12 5
#define BLUE12 6

#define RED34 9
#define GREEN34 10
#define BLUE34 11

#define LED1  4
#define LED2  13
#define LED3  8
#define LED4  7

#define IRT A3
#define IRD A2
#define IR_INITDELAY 2


const int rainbowRGB[7][3] = {

  { 255, 0, 0 }, //Red
  { 255, 165, 0 }, //Orange
  { 255, 255, 0 }, //Yellow
  { 0, 128, 0 }, //Green
  { 0, 0, 255 }, //Blue
  { 75, 0, 130 }, //Indigo
  { 238, 130, 238 } //Violet
  
};


int stepUpDelay = 50;
int mainDelay = 200;
int mainLoop = 50;
int stepDownDelay = 20;
int seqDelay = 400;
int radIncrement = 24;

int whichLed = 0;

int factor = 0;
boolean rainbowMode = false;
boolean useToggle = false;

unsigned long lastKnownWaveInTime = 0;
boolean turnOffDaisy = false;
boolean rainbowJump = false;
int colorWheelMode = -1;




boolean MASTEROFF = false;

byte master;




void setup()  {
  
  Serial.begin (9600);
  
  //pinMode(13, OUTPUT);
  //digitalWrite(13, HIGH);  
  
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);  
  
  master = EEPROM.read(0);
  master++;
  if (master > 1) {
    master = 0; 
  }
  
  EEPROM.write(0, master);
  
  if (master == 0) {
     MASTEROFF = false;
  }
  else {
     MASTEROFF = true;
     return; 
  }
  
  // declare pins to be an output:
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, HIGH);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, HIGH);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED3, HIGH);
  pinMode(LED4, OUTPUT);
  digitalWrite(LED4, HIGH);
  
  pinMode(RED12, OUTPUT);
  pinMode(GREEN12, OUTPUT);
  pinMode(BLUE12, OUTPUT);
  pinMode(RED34, OUTPUT);
  pinMode(GREEN34, OUTPUT);
  pinMode(BLUE34, OUTPUT);  
  
  pinMode(IRD, OUTPUT);
  digitalWrite(IRD, HIGH);  
  pinMode(IRT, INPUT_PULLUP);  

}

//from 0 to 127
void displayColor(uint16_t WheelPos)
{
  
  byte red, green, blue;
  
  int useLed;
  
  
  if (rainbowMode || (factor==2) || (factor==5)) {
    useLed = random(4);
  }
  else {
    useLed = whichLed%4;
    if (factor == 3) {
      if (useLed == 0) {
        useToggle = !useToggle;
      }
      if (useToggle) {
        useLed = 3-useLed; 
      }
    }
  }
  
  switch (useLed) {
    case 0:
      digitalWrite(LED1, LOW);
      red = RED12;
      green = GREEN12;
      blue = BLUE12;
      break;
    case 1:
      digitalWrite(LED2, LOW);
      red = RED12;
      green = GREEN12;
      blue = BLUE12;
      break;      
    case 2:
      digitalWrite(LED3, LOW);
      red = RED34;
      green = GREEN34;
      blue = BLUE34;
      break;      
    case 3:
      digitalWrite(LED4, LOW);
      red = RED34;
      green = GREEN34;
      blue = BLUE34;
      break;
  }
  
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128; //Red down
      g = WheelPos % 128; // Green up
      b = 0; //blue off
      break;
    case 1:
      g = 127 - WheelPos % 128; //green down
      b = WheelPos % 128; //blue up
      r = 0; //red off
      break;
    case 2:
      b = 127 - WheelPos % 128; //blue down
      r = WheelPos % 128; //red up
      g = 0; //green off
      break;
  }
  
  if (rainbowMode) {
    
    for (int i=0; i<384; i++) {
      analogWrite(red, r);
      analogWrite(green, g);
      analogWrite(blue, b);
      delay(stepDownDelay);
      
      if ( (i%20 == 0) && (readFromIR())) {
         return; 
      }
      
      WheelPos = (WheelPos+1) % 384;
      switch(WheelPos / 128)
      {
        case 0:
          r = 127 - WheelPos % 128; //Red down
          g = WheelPos % 128; // Green up
          b = 0; //blue off
          break;
        case 1:
          g = 127 - WheelPos % 128; //green down
          b = WheelPos % 128; //blue up
          r = 0; //red off
          break;
        case 2:
          b = 127 - WheelPos % 128; //blue down
          r = WheelPos % 128; //red up
          g = 0; //green off
          break;
      }
    }    
    
    
    
  }
  else {
  
    analogWrite(red, r);
    analogWrite(green, g);
    analogWrite(blue, b);
    delay(stepUpDelay);  
    
    r = r*2;
    g = g*2;
    b = b*2;
    analogWrite(red, r);
    analogWrite(green, g);
    analogWrite(blue, b);
    
    delay(mainDelay);
    
    for (int i=0; i<mainLoop; i++) {
      r = r*80/100;
      g = g*80/100;
      b = b*80/100;
      analogWrite(red, r);
      analogWrite(green, g);
      analogWrite(blue, b);
      delay(stepDownDelay);
    }
  
  }
  
  //delay(150);
  
  switch (useLed) {
    case 0:
      digitalWrite(LED1, HIGH);
      break;
    case 1:
      digitalWrite(LED2, HIGH);
      break;      
    case 2:
      digitalWrite(LED3, HIGH);
      break;      
    case 3:
      digitalWrite(LED4, HIGH);
      break;
  }  
  
  whichLed++;
}

boolean setParameters (int irReading) {
  
  int previousFactor = factor;
  
  if (irReading > 968) {
    factor = 0;
  }
  else if (irReading > 950) {
    factor = 1; 
  }
  else if (irReading > 900) {
    factor = 2; 
  }
  else if (irReading > 850) {
    factor = 3; 
  }  
  else if (irReading > 700) {
    factor = 4; 
  }  
  else if (irReading > 500) {
    factor = 5; 
  }
  else if (irReading > 400) {
    factor = 6;
  }
  else if (irReading > 300) {
    factor = 8; 
  }
  else if (irReading > 200) {
    factor = 10; 
  }
  else if (irReading > 100) {
    factor = 12; 
  }
  else if (irReading > 50) {
    factor = 15; 
  }
  else {
    factor = 20; 
  }
  
  
  if (factor < 13) {
    if (factor - previousFactor > 2) {
      lastKnownWaveInTime = millis(); 
    }
    
    if ( (previousFactor - factor > 2) && ( millis() - lastKnownWaveInTime < 800 )) {
      
      if (turnOffDaisy) {
        turnOffDaisy = false;
        rainbowJump = true;
      }
      else {
        turnOffDaisy = true;
        //turning off all 4 LEDs by setting them HIGH
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
        digitalWrite(LED3, HIGH);
        digitalWrite(LED4, HIGH);
        seqDelay = 0;        

      }
      return true;
    }
    
  }


  
  if (factor == 0 ) {

    stepUpDelay = 50;
    mainDelay = 200;
    mainLoop = 50;
    radIncrement = 4;    
    
    factor = 0; 
    rainbowMode = true;
    stepDownDelay = 5;
    seqDelay = 0;
   
  } else { //if (factor < 7) {

    rainbowMode = false;
    
    stepUpDelay = 50/(factor*2);
    mainDelay = 200/(factor*2);
    mainLoop = 50/(factor*2);
    stepDownDelay = 20/(factor*2);
    seqDelay = 400/(factor*2);
    //radIncrement = 4/(factor*2);
    
  }
  
  return false;
  
}

//boolean readFromIR() {
//  return readFromIR(false); 
//}

boolean readFromIR() {
  
  boolean retVal;
  
  digitalWrite(IRD, HIGH);  
  delay(IR_INITDELAY);
  
  int tcrt = analogRead(IRT);
  
  Serial.println(tcrt);
    
  retVal = setParameters(tcrt);

  digitalWrite(IRD, LOW);    
  
  return retVal;

}


void setColorWheel() {
  
  colorWheelMode++;
  if (colorWheelMode > 6) {
    colorWheelMode = 0;
  }
  
  rad = colorWheelMode * 55; // because 384 colors in wheel, and divide by 7 colors
  
  analogWrite(RED12, rainbowRGB[colorWheelMode][0]);
  analogWrite(RED34, rainbowRGB[colorWheelMode][0]);
  
  analogWrite(GREEN12, rainbowRGB[colorWheelMode][1]);
  analogWrite(GREEN34, rainbowRGB[colorWheelMode][1]);
  
  analogWrite(BLUE12, rainbowRGB[colorWheelMode][2]);
  analogWrite(BLUE34, rainbowRGB[colorWheelMode][2]);
  
  //turn on LEDs
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);         
  delay(900);
  
  //turn off LEDs
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);         
}



void loop()  {  
  
  if (MASTEROFF) {
   delay(5000);
   return; 
  }
  
  readFromIR();
  
  if (turnOffDaisy) {
    delay(30);
  }
  else if (rainbowJump) {
    rainbowJump = false;
    setColorWheel();
  }  
  else {
    displayColor(rad);
    delay(seqDelay);                           
    rad = (rad+radIncrement) % 384;
  }
}
