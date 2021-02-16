#include <EEPROM.h>
/*
   Multi-Dimensional Tremolo Guitar Pedal For Atmega 32U4
   by Jacob Fifield February 2021
   A polyrhymic take on a tremolo pedal!
   
   This is the same Multi-Dimensional Tremolo unit, but modified for use on Atmega 32U4. Debugging the 328p on a breadboard with all the peripherals proved difficult and tedious 
   so I bought a pro Micro to put on the breadboard instead. Since it is so much smaller, I can fit all the peripherals and I/O ports on 2 of my breadboards in a much more 
   managable fashion. 
   
   
   After doing some code review here are the fixes/reworks that need to be done:
   
   -Don't drive the indicator LED's with the output signal. Instead use another digital port and control them seperately 
   -Use pullup mode on digital button inputs for better stability
   -Rework Gridded Poly Mode, it's a mess and doesn't work right when switching between polys 
   -More bug testing on Record(); and Playback(); because sometimes ghost inputs are collected and played back
   -camelCase, pls
   -EEPROM storage of patterns so that your saved pattern persists between powerups (if you save it!)
     
   Pin layout cheatsheet for 324U:
     A0 - 2K Ohm Potentiometer (Rate knob)
     A1 - 10K Ohm Potentiometer (Poly knob, Will be replaced with notched pot)
     D15 - Push Button 2 (Tap Tempo/ Custom Pattern Input)
     D14 - Push Button 1 (Sync+Unsync/ Record Custom Pattern/Play Tremolo)
     D09 - Output Channel 2 (Output to phototransistor corrispoonding to channel two of the opamp)
     D08 - Output Channel 1 (Output to phototransistor corrispoonding to channel one of the opamp)
     D07 - Toggle Switch (Custom Mode/Poly Mode)
     D06 - Utility Push Button
     D00 - Red RGB Led
     D01 - Blue RGB Led
     D02 - Green RGB Led
     D03 - Rate Indicator 
     D04 - Poly Indicator
*/

//Hardware Pin Constants
const int pot1 = 0;               //Potentiometer 1, to control the interval rate
const int pot2 = 1;               //Potentiometer 2, to control the poly selection
const int togglesw = 7;           //Toggle Switch, For Mode Selection
const int out1 = 8;               //Output Channel 1
const int out2 = 9;               //Output Channel 2
const int pushb1 = 11;            //Momentary Push Button 1 for Recording Custom Pattern and Internal Sync
const int pushb2 = 12;            //Momentary Push Button 2 for Inputting Custom Pattern, Playing in Killswitch Mode, and Tap Tempo
const int pushb3 = 6;             //Momentary Push Button 3 for Utility Functions: MIDI Sync, EEPROM R/W and
const int indicator_r = 4;        //Indicator LED red
const int indicator_g = 3;        //Indicator LED green
const int indicator_b = 2;        //Indicator LED blue

//State Variables
bool togglesw_state, pushb1_state, pushb2_state, pushb3_state = 0;                    //Boolean States for the 2 buttons, and switch
bool prev_pushb1_state = 0;
bool prev_pushb2_state = 0;
bool out1_state = 1;                                                                  //Output Channel 1 State
bool out2_state = 1;                                                                  //Output Channel 2 State
bool program_chg = 0;

//Timing and Polyrhythm Variables
int interval, poly_select = 0;                                                        //Interval (Half Note), Poly_select is the subdivision of the whole note
int prev_poly_select = 0;
unsigned long prev_Millis, current_Millis, prev_Millis2, current_Millis2 = 0;         //Different variables to hold the two times in FreePoly Mode
unsigned long current_Millis_B1, prev_Millis_B1 = 0;
float interval2;                                                                      //Better sig figs for dealing with polyrhythms
bool freeMode = 1;                                                                    //Freemode Flag
bool synced = 1;
int B1_db = 25;                                                                       //Button Debounce Time in ms
bool init_poly_sync = 0;

//Recording Variables
const int SIZE = 127;
unsigned long rec_Millis[SIZE] = {0};                                                   //Recording array set at size 50 because dynamic memory arrangment is not straightforward on arduino
unsigned long temp_Millis;                                                            //temporary Millis for loading the recording array
bool last_state;
bool started;
bool start_state;                                                                     //Start state of the button when in record mode
bool recording_state = 0;                                                             //Recording State Flag

//Playback Variables
bool playback_state = 0;                                                              //Playback State Flag
bool PB_init = 0;
unsigned long prev_temp_Millis = 0;
unsigned long prev_P_Millis = 0;
unsigned long current_P_Millis = 0;
unsigned long erase_Millis = 2500;
int PB_ind = 0;
int z = 0;
float PB_rate = 0;

//Utility Handler Variables
unsigned long prev_H_Millis = 0;
const int Erase_time = 3000;


//EEPROM
const int starting_EEPROM_address = 17;


void Utility_Handler()
{
  pushb3_state = digitalRead(pushb3);
  if (pushb3_state == 1 && togglesw_state == 1)
  {
    //Do Utility function for Poly Mode (MIDI Clock Sync)
  }
  else if (pushb3_state == 1 && playback_state == 1)                                     //Do Utility function for Custom Mode (Erase Function)
  {
    prev_H_Millis = millis();
    while (pushb3_state == 1)                                                       //While the Utility Button is being pressed in a playback state
    {
      Playback();                                                                   //Continue to playback until the button has been held for the amount of time dictated by "Erase Time"
      if ((millis() - prev_H_Millis) >= Erase_time)
      {
        for (int x = 0; x < SIZE; x++)                                                //Reset the Array that holds the milliseconds between the state changes
        {
          rec_Millis[x] = 0;
          PB_ind = 0;                                                                   //Reset the Pointer that works inside of Record(); to keep track of array
        }
        playback_state = 0;                                                         //Reset the Playback state, since we have nothing to playback
        Erase_Animation();
        break;                                                                      //Break from the while loop, or else we are stuck here forever.
      }
      else
      {
        pushb3_state = digitalRead(pushb3);                                         //If the button is still being pressed, but we haven't reached the threshold we still need to check the state of the button
      }
    }
  }
}

void Erase_Animation()                                                              //Flashing purple animation on the indicator LED to denote that you have successfully erased the pattern.
{
  digitalWrite(indicator_r, LOW);
  digitalWrite(indicator_g, LOW);
  digitalWrite(indicator_b, LOW);
  delay(75);
  digitalWrite(indicator_b, HIGH);
  digitalWrite(indicator_r, HIGH);
  delay(75);
  digitalWrite(indicator_b, LOW);
  digitalWrite(indicator_r, LOW);
  delay(75);
  digitalWrite(indicator_b, HIGH);
  digitalWrite(indicator_r, HIGH);
  delay(75);
  digitalWrite(indicator_r, LOW);
  digitalWrite(indicator_g, LOW);
  digitalWrite(indicator_b, LOW);
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
}


void Playback()
{
  PB_rate = analogRead(pot2);
  PB_rate = map(PB_rate, 0, 1023, 5, 1);
  digitalWrite(indicator_g, HIGH);
  //Serial.println(PB_rate);
  if (PB_init == 0)                                                                      //If playback has not been initialized
  {
    digitalWrite(out1, start_state);                                                     //Write the start states to start playback
    digitalWrite(out2, start_state);
    out1_state = start_state;                                                            //Assign the start states to the output_states
    out2_state = start_state;
    PB_ind = 0;                                                                          //Reset the Plackback Index to 0
    PB_init = 1;                                                                         //Set the Playback initialize flag to true to indicate we have setup correctly. 
  }
  else
  {
    if (rec_Millis[z] != 0)
    {
      unsigned long temp_speed = rec_Millis[z] * PB_rate
                                 ;
      current_P_Millis = millis();                                                        //Get the number of milliseconds that has passed since turn on
      if (current_P_Millis - prev_P_Millis >= temp_speed) {                               //If we are outside the bounds of the interval
        if (out1_state == 0) {                                                            //If the state is low, set it high and vise versa
          out1_state = 1;
          z = z + 1;
        }
        else {
          out1_state = 0;
          z = z + 1;
        }
        prev_P_Millis = current_P_Millis;                                                  //Store the last time we changed states
      }
      out2_state = out1_state;
      digitalWrite(out1, out1_state);
      digitalWrite(out2, out2_state);
      PB_rate = analogRead(pot1);
      PB_rate = map(PB_rate, 0, 1024, 5, 1);
    }
    else
    {
      PB_rate = analogRead(pot1);
      PB_rate = map(PB_rate, 0, 1024, 5, 1);
      z = 0;
    }
  }

}

void Record()
{
  digitalWrite(indicator_r, HIGH);                                  //Write to the indicator LED
  pushb2_state = digitalRead(pushb2);                             //Read the button states
  pushb1_state = digitalRead(pushb1);
  if (prev_pushb2_state != pushb2_state)
  {
    rec_Millis[PB_ind] = (millis() - prev_temp_Millis);
    prev_temp_Millis = millis();
    PB_ind = PB_ind + 1;
  }
  if (pushb2_state == 1)                                          //If the Custom input button is HIGH
  {
    digitalWrite(out1, HIGH);                                     //Write both outputs to HIGH
    digitalWrite(out2, HIGH);
  }
  else if (pushb1_state == 0)                                     //If the Recording Button has stopped being pressed, but the recording flag has not been turned off yet
  {
    recording_state = 0;                                          //Turn the Recording Flag to 0
    playback_state = 1;
    digitalWrite(indicator_r, LOW);                                 //Write the indicator LED to LOW
  }
  else                                                            //If the Custom input button is LOW
  {
    playback_state = 1;
    digitalWrite(out1, LOW);                                      //Write both outputs to LOW
    digitalWrite(out2, LOW);
  }
  prev_pushb2_state = pushb2_state;
}

void customMode()
{
  if (program_chg == 1)                                               //If the program is being launched either for the first time or from switching over from the other mode
  {
    digitalWrite(indicator_r, LOW);                                   //Write indicator Purple
    digitalWrite(indicator_b, LOW);
    program_chg = 0;                                                  //Reset Sync and program change flags
    init_poly_sync = 0;
  }
  if (rec_Millis[0] != 0)
  {
    playback_state = 1;
  }
  if (playback_state == 0 && recording_state == 0)                    //Since the arduino main code runs in a loop, we end up calling this function many times per second, we have to check if we are in playback or recording mode.
  {
    digitalWrite(out1, LOW);                                          //If we are not in playback or recording mode, that means there has been no recording, and no recording has been loaded from the EEPROM. Odds are, we've switched over from the other mode
    digitalWrite(out2, LOW);                                          //Just in case the switch has flipped while the other mode had a pin high, we will reset both the output pins to low, because the user is not recording or playing back
  }
  if (playback_state == 1)
  {
    Playback();
  }
  pushb2_state = digitalRead(pushb2);                                 //Read the current state of both buttons and write value to their respective boolean variable memory spaces
  pushb1_state = digitalRead(pushb1);
  if (pushb1_state == 1 && recording_state == 0)                      //The "record" button has been pushed but the recording state flag has not been set yet
  {
    start_state = digitalRead(pushb2);                                //Take note of the starting state
    recording_state = 1;                                              //Set the recording state flags
    prev_pushb2_state = start_state;
    prev_temp_Millis = millis();
    while (recording_state == 1)                                      //While the recording state flag is true: Recording Mode Activated
    {
      Record();
    }
  } // end of if
  else {                                                              //In this case, the user is just inputting a pattern to be played live, so control is given in the same fashion as above (Killswitch Mode)
    if (pushb2_state == 1)                                            //If the Custom input button is HIGH
    {
      digitalWrite(out1, HIGH);                                       //Write both outputs to HIGH
      digitalWrite(out2, HIGH);
    }
  }
} // end of void customMode();


void B1_toggle() {
  pushb1_state = digitalRead(pushb1);                               //This function takes the input from Momentary PB_1 and turns it into toggle information
  if (pushb1_state == 1 && prev_pushb1_state == 0)
  {
    //started pressing the button
    prev_pushb1_state = 1;
    if (freeMode == 1)
    {
      synced = 0;                                                   //If we are in freeMode and we've toggled the button, we will mark the synced and freeMode flag as false to
      freeMode = 0;                                                 //trigger the grid initialization next time we want to go into griddedMode.
    }
    else
    { //Otherwise we toggled the button while in GriddedMode, so change the freeMode flag to true
      freeMode = 1;
    }
  }
  if (pushb1_state == 0 && prev_pushb1_state == 1)                  //If the current state is false and the last recorded state is true, then we know we've stopped pressing the button
  {
    //stopped pressing the button                                   //Therefore we need to flip the flag
    prev_pushb1_state = 0;
  }
}

void freePoly() {
  digitalWrite(indicator_r, LOW);
  digitalWrite(indicator_b, LOW);
  digitalWrite(indicator_g, LOW);
  synced = 0;
  init_poly_sync = 0;
  interval = analogRead(pot1);                                      //Read the Analog Inputs to get interval and poly_select values
  poly_select = analogRead(pot2);
  interval = map(interval, 0, 1023, 1000, 50);                      //Map interval and poly_select to correct value ranges
  poly_select = map(poly_select, 0, 1023, 2, 16);
  interval2 = (interval * 2) / poly_select;                         //interval2 represents is the poly rhythmic element based on the entire length of the whole note
  current_Millis = millis();                                        //Get the number of milliseconds that has passed since turn on
  current_Millis2 = millis();
  if (current_Millis - prev_Millis >= interval) {                   //If we are outside the bounds of the interval
    if (out1_state == 0) {                                          //If the state is low, set it high and vise versa
      out1_state = 1;
    }
    else {
      out1_state = 0;
    }
    prev_Millis = current_Millis;                                   //Store the last time we changed states
  } //end interval if statement
  if (current_Millis2 - prev_Millis2 >= interval2) {                //Same thing as channel 1, but with the polyrhythm channel
    if (out2_state == 0) {
      out2_state = 1;
    }
    else {
      out2_state = 0;
    }
    prev_Millis2 = current_Millis2;                                //Store the last time we changed states
  } //end interval2 if statement
  digitalWrite(out1, out1_state);                                  //Write output states
  digitalWrite(out2, out2_state);
}

void gridPoly()
{
  digitalWrite(indicator_r, HIGH);                              //Now we are
  digitalWrite(indicator_g, LOW);                                   //Light the indicator purple to indicate that we are in gridded mode
  digitalWrite(indicator_b, HIGH);
  interval = analogRead(pot1);                                      //Read the Analog Inputs to get interval and poly_select values
  poly_select = analogRead(pot2);
  interval = map(interval, 0, 1023, 1000, 50);                      //Map interval and poly_select to correct value ranges
  poly_select = map(poly_select, 0, 1023, 2, 16);
  interval2 = (interval * 2) / poly_select;                         //interval2 represents is the poly rhythmic element based on the entire length of the whole note
  if (synced == 0 || prev_poly_select != poly_select || init_poly_sync == 0)
  {
    out1_state = 1;                                                 //If the poly select is different than the last time it was scanned, or out of sync due to program change/first launch
    out2_state = 1;                                                 //Reset the states and millis so that we can ensure that we are in sync
    prev_Millis = 0;
    prev_Millis2 = 0;
    synced = 1;
    init_poly_sync = 1;
    prev_poly_select = poly_select;
    current_Millis = millis();                                        //Get the number of milliseconds that has passed since turn on
    current_Millis2 = millis();
    gridPoly();
  }
  if (current_Millis - prev_Millis >= interval) {                   //If we are outside the bounds of the interval
    if (out1_state == 0) {                                          //If the state is low, set it high and vise versa
      out1_state = 1;
    }
    else {
      out1_state = 0;
    }
    prev_Millis = current_Millis;                                   //Store the last time we changed states
  } //end interval if statement
  if (current_Millis2 - prev_Millis2 >= interval2) {                //Same thing as channel 1, but with the polyrhythm channel
    if (out2_state == 0) {
      out2_state = 1;
    }
    else {
      out2_state = 0;
    }
    prev_Millis2 = current_Millis2;                                //Store the last time we changed states
  } //end interval2 if statement
  if (current_Millis - prev_Millis < interval)
  {
    current_Millis = millis();
  }
  if (current_Millis2 - prev_Millis2 < interval2)
  {
    current_Millis2 = millis();
  }
  digitalWrite(out1, out1_state);                                  //Write output states
  digitalWrite(out2, out2_state);
  prev_poly_select = poly_select;
}

void polyMode() {
  program_chg = 1;                                                  //Set the Program Change Flag so that
  recording_state = 0;                                              //Set the flags for playback/recording mode back to zero, as we are no longer in that mode
  playback_state = 0;
  B1_toggle();
  if (freeMode == 0)
  {
    freePoly();
  }
  else
  {
    gridPoly();
  }
}


void setup()
{
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(togglesw, INPUT);
  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(pushb1, INPUT);
  pinMode(pushb2, INPUT);
  pinMode(pushb3, INPUT);
  pinMode(indicator_r, OUTPUT);
  pinMode(indicator_b, OUTPUT);
  pinMode(indicator_g, OUTPUT);
}



void loop()
{
  Utility_Handler();                                                  //Run Utility Handler for Utility Button
  togglesw_state = digitalRead(togglesw);                             //Read the state of the Toggle Switch
  if (togglesw_state == 1)
  {
    polyMode();                                                       //Call polyMode();
  }
  else
  {
    customMode();                                                     //Call customMode();
  }
}
