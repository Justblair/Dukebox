// Microcontroller firmware to interface with a RockOlla 477 dukebox
// Written by JustBlair started 02/06/2024, ended ???
// The idea of the project is to create a microcontroller firmware that emulates a keyboard and 
// serial controller. Based on a pro-micro, but using PCF8575 I2C port expanders to drive relays to 
// emulate button presses as well as read the 0-9 and reset buttons.

// ToDo List:
// Interface with the keyboard
// 

// includes:
#include <Arduino.h>
#include "Wire.h"
#include "I2CKeyPad8x8.h"
// #include "LedControl.h"
#include "Keyboard.h"
#include "PCF8575.h"
#include <TM1637Display.h>


// function declarations:
void switchAudio(bool);
void serialAction();
void checkKeypad();
void scrollDigits();
void sendTrackNo_Rockolla();
void sendTrackNo_Pi();

// global constants:
# define KEYPAD_ADDRESS 0x21
# define debounceTime 400
# define rockolaDelay 250

// global variables:
uint32_t lastKeyPressed = 0;
uint8_t lastIndex;
uint8_t index;
uint32_t now = 0; // timer for debounce etc. Run once per loop
char incomming = 0;

uint8_t selection[3];
uint8_t selIndex = 0;

const int IRQPIN = 7;

volatile bool flag = false;
uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
const uint8_t SEL[] = {
	SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,           // S
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,            // E
	SEG_D | SEG_E | SEG_F,                           // L
  0x00
	};

// Module connection pins (Digital Pins) TM1637
#define CLK 5
#define DIO 6

/* Used in controlling LED segmnent display we always wait a bit between updates of the display */
unsigned long delaytime=250;

// pin definitions:
int leftAudioPin = PIN3;
int rightAudioPin = PIN4;
int mutePin = PIN5;
int addCredit = PIN6;

// Macros

// Key defines. These are needed because the keyboard is multiplexed
// and we don't want to alter the PCB etc. So we are kinda stuck with 
// whatever keys are stuck

#define RESET 23
#define key1  4
#define key2  20
#define key3  14
#define key4  6
#define key5  13
#define key6  5
#define key7  21
#define key8  15
#define key9  7
#define key0  12

#define PCF8575_INITIAL_VALUE 0xFFFF


#define rockola true
#define pi false

PCF8575 relays(0x20);  //  add relays to lines (used as output)
I2CKeyPad8x8 keyPad(KEYPAD_ADDRESS);

TM1637Display display(CLK, DIO);


void pcf_irq()
{
  flag = true;
      // Serial.print("f: ");
}

void setup() {
  // put your setup code here, to run once:

  // initiate serial connection
  Serial.begin(115200);
  Serial.println(__FILE__);

  Wire.begin();
  Wire.setClock(400000);

  display.setBrightness(0x0f);
  display.setSegments(SEL);

  Keyboard.begin();  // Start Keyboard emulation
  

  // double check that we have keyboard access, return an error if not
  while (keyPad.begin() == false)
  {
    Serial.println("\nERROR: cannot communicate to keypad.\nPlease reboot.\n");
  }

  keyPad.setDebounceThreshold(debounceTime);

  pinMode(IRQPIN, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(IRQPIN), pcf_irq, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:

  now = millis();  // set the value of now to timer for use in functions
  serialAction();  // Checks for and actions serial commands
  checkKeypad();
}

void switchAudio(bool OnOff){
  // function to switch the audio from pi to record player.
  digitalWrite (leftAudioPin, OnOff);
  digitalWrite (rightAudioPin, OnOff);

  
  relays.write(15, OnOff);  
  if(OnOff == rockola){
    Serial.println("Rockola On");
  } else {
    Serial.println("Pi On");
  }
}

void serialAction(){
// Serial read and take action block
  if (Serial.available() > 0){
    incomming = Serial.read();

    if (incomming == 'p'){
      switchAudio(pi);
    } else if (incomming == 'r');{
      switchAudio(rockola);
    }
  }
// end of Serial read and take action block
}

void checkKeypad(){


    // the 

    index = keyPad.getKey();
    Serial.println (index);

    // check to see if the key is reset
    //delay (100);
    if (index == RESET){
      memset(selection,0,3);
      selIndex = 0;
      Serial.println("reset");
      lastIndex = index;
      display.setSegments(SEL);
      memset(data, '\0', sizeof data);


    } else if ((index == 64) or (index == 255))
    {
      lastIndex = index;
    } else if (index != lastIndex){
      // do something with the new key value!
      // Serial.print(index);

      // Load the selection into the selection list...
      // unless it's a forbidden choice 

      // first of all the selection needs to be converted to the 
      // actual values

      int8_t selDigit; 

      switch (index)
      {
        case key0:
          selDigit = 0;
          break;
        case key1:
          selDigit = 1;
          break;
        case key2:
          selDigit = 2;
          break;
        case key3:
          selDigit = 3;
          break;
        case key4:
          selDigit = 4;
          break;
        case key5:
          selDigit = 5;
          break;
        case key6:
          selDigit = 6;
          break;
        case key7:
          selDigit = 7;
          break;
        case key8:
          selDigit = 8;
          break;
        case key9:
          selDigit = 9;
          break;
      }

      if (selIndex == 2 && (selDigit == 8 || selDigit == 9)){
        // discount the selection
      } else {

        // load the selection list with the converted value
        selection[selIndex] = selDigit;
        data[selIndex] =  display.encodeDigit(selDigit);

        // print the vale to the LED display
        display.setSegments(data);

        // lc.setDigit(0,selIndex,selection[selIndex],false);

        Serial.print ("digit: ");
        Serial.print (selection[selIndex], DEC);
        Serial.print ("  pos: ");
        Serial.println(selIndex, DEC);

        selIndex = selIndex + 1;
      }

      if (selIndex > 2){
        // all three numbers are now selected lets update the rockolla etc.

        // first check to see if the number is in the 100s or 200s
        if (selection[0] == 1 || selection[0] == 2 ){
          sendTrackNo_Rockolla();
        } else {
          sendTrackNo_Pi();
        }

        for (int i = 0; i<3; i++) {
          display.setSegments(data);
          delay(250);
          display.clear();
          delay(250);
        }
        display.setSegments(SEL);
        // clear the display data
        memset(data, '\0', sizeof data);
        selIndex = 0;
      }
      index = lastIndex;
    }

}


void sendTrackNo_Rockolla(){

  Serial.println("Sending TrackNo to Rockolla");

  // for safety, send a reset signal first

  // relays.write(10, LOW);
  // delay(rockolaDelay);
  // relays.write(10, HIGH);

  for (int i=0; i<3; i++){
    relays.write(selection[i], LOW);
    delay (rockolaDelay);
    relays.write(selection[i], HIGH);
    delay (rockolaDelay);
  }
  switchAudio(rockola);
  // checkKeypad();
}

void sendTrackNo_Pi(){
  switchAudio(pi); 

  Serial.print("Sending TrackNo to Pi: ");
  // not going to bother with a loop, it's only sending 3 digits
  Keyboard.print(selection[0]);
  Keyboard.print(selection[1]);
  Keyboard.print(selection[2]);
  

  Serial.print(selection[0]);
  Serial.print(selection[1]);
  Serial.println(selection[2]);


}