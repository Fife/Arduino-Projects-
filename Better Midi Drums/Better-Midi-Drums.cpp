/*
 * Written by Jacob Fifield on April 4th 2020
 * MIDI Drum Extra Curricular Submission
 * Here is the code for my MIDI drum pad that I designed over Spring Break
 *
 * Note: This will not compile on Clion because it requires Arduino Libraries! However since the
 * Arduino IDE uses C++ this code is pastable and runnable on an Arduino.
*/


#include <MIDI.h>                                                                                                       //Include the Arduino MIDI library, which gives access to musical instrument digital interface functions.

MIDI_CREATE_DEFAULT_INSTANCE();                                                                                         // Create a MIDI I/O session at the default BAUD Rate

//------------- Constants

const int InputCount = 4;                                                                                               //How many piezoelectric inputs are present.
const int threshold = 600;                                                                                              //Raw Threshold for fine tuning in the field
int note[InputCount] = {51, 50, 52, 53};                                                                                //Note Numbers

//------------ Variables

int last_piezoReading [InputCount];                                                                                     //Used to compare piezo states in the function piezoState
int current_piezoReading[InputCount];                                                                                   //Also Used to compare piezo states in the function piezoState
int piezoReadings [InputCount];                                                                                         //Stores the Raw 10-bit integer readings from the piezo electric electric element
bool notestate[InputCount];                                                                                             //An array of true/false notestates Used to trigger the MIDI function
bool prev_notestate[InputCount];                                                                                        //An array of true/false notestates used to check for the last trigger
bool passed_thresh[InputCount];                                                                                         //An array of true/false statemenents used to guard against double triggers
unsigned long startMillis[InputCount];                                                                                  //An array of unsigned long integers, used to count Microseconds 
unsigned long currentMillis[InputCount];                                                                                //An array of unsigned long integers, used to compare against start microseconds, and figure out how much time has passed since last note event
const unsigned long period = 22730;                                                                                     //Number of microseconds. This determines how long before the same midi message can fire again (To reduce jitter in high cycles) 22730 microseconds is the equivelent of 32nd note triplets at 220 bpm
double velocity[InputCount];                                                                                            //Velocity information array as double for transformation purposes
int ivelocity[InputCount];                                                                                              //Velocity information array as as integer for midi message

//------------Functions used in loop

void piezoRead()                                                                                                        //piezoRead is a function that reads each piezo elements and stores the current value into an array of ints (piezoReadings).
{
    for (int i = 0; i < InputCount; i++)
    {
        piezoReadings[i] = analogRead(i);                                                                               //analogRead(); is a function that is on the Arduino native library that returns a 10 bit integer value (0-1023) from a voltage reading (0V- 4.9V) on the device.
    }
}

void piezoState()                                                                                                       //piezoState is a function that processes the state of each piezo electric element from raw 10-bit data into a boolean array.
{                                                                                                                       //As the piezo element is pressed (hit w/ drumstick) it generates a small voltage within our range. This voltage is not instantaneous and follows a "velocity curve" of sorts
    for (int i = 0; i < InputCount; i++)                                                                                //This "velocity curve" is also well within the time it takes to read multiple inputs
    {                                                                                                                   //What this function does is compare the last reading in the ith slot of the previous array with the current reading in the ith slot of the current reading array
        currentMillis[i] = micros(); 
        current_piezoReading[i] = piezoReadings[i];
        if(current_piezoReading[i] >= threshold && current_piezoReading[i] > last_piezoReading[i] && notestate[i]==false && passed_thresh[i]==false)
        {
            last_piezoReading[i] = current_piezoReading[i];                                                             //While the current reading is greater than the last reading, the voltage (velocity spike) is still increasing.
            current_piezoReading[i] = analogRead(i);                                                                    //That means that the notestate should be false, no note has been triggered yet
            notestate[i] = false;
            passed_thresh[i] =true;                                                                                     //We want to mart that we have passed the threshold, even though no note has been triggered yet.
            prev_notestate[i]= notestate[i];
            break;
        }
        if(last_piezoReading[i] > current_piezoReading[i] && last_piezoReading[i] > threshold && notestate[i]==false && passed_thresh[i]==true)
        {
            notestate[i]=true;                                                                                          //If the last notestate is greater than the current notestate that means the voltage has started decreasing.
            velocity[i] = ((last_piezoReading[i]/8)-1);                                                                 //Bit reduction math, from 10 bit integer to a 7 bit, and subtracting one to get a value from 0-127                                                                                                  
            velocity[i] = velocity[i]*(log(velocity[i]+1)+(velocity[i]/5+1/10));                                        //Map to custom velocity function: (ln(x+1)+x/5+1/10)
            ivelocity[i]= int(velocity[i]);
            if(notestate[i] == true && last_piezoReading[i] > current_piezoReading[i]&& prev_notestate[i]!=true && currentMillis[i] - startMillis[i] >= period)
            {
                MIDI.sendNoteOn(note[i], ivelocity[i], 1);                                                               //If the last notestate was false and the current notestate is true: We can trigger a MIDI.sendNoteOn function with the velocity associated from the last step
                prev_notestate[i]= notestate[i];                                                                         //In order to make jitter above the threshold less of a problem in test enviornments, MIDI.sendNoteOn only gets called every "period" of microseconds
                startMillis[i] = currentMillis[i];
            }
            prev_notestate[i] = notestate[i];
            break;
        }
        if(current_piezoReading [i] < threshold && notestate[i]== true && prev_notestate[i]!=false && currentMillis[i] - startMillis[i] >= period)                    
        {
            notestate[i] = false;                                                                                        //Here we are no longer above the threshold, and have already triggered a note.
            prev_notestate[i] = notestate[i];                                                                            
            passed_thresh[i]=false;                                                                                      //Mark that we are under the threshold
            MIDI.sendNoteOff(note[i], velocity[i], 1);                                                                   //Send MIDI note off
            break;
        }
        last_piezoReading[i] = current_piezoReading[i];
        prev_notestate[i] = notestate[i];
    }
}


void laststate()                                                                                                         //laststate(); Is a function that simply assigns the last piezo array to the current piezo array. Used at the end of the main loop.
{
    for (int i= 0; i< InputCount; i++)
    {
        last_piezoReading [i] = piezoReadings[i];
    }
}

//------------ Setup
/*
 * The Arduino works by running a setup function once, and then looping another function
 */

void setup() {
    MIDI.begin(1);                                                                                                       //Begin MIDI session at the default rate
    MIDI.sendControlChange(123,0,1);                                                                                     //All notes off message. Boot up on the arduio is electrically noisy and can trigger phantom MIDI messages!
    for(int i; i< InputCount; i++){                                                                                       
      startMillis[i] = micros();                                                                                         //Initialize the start time 
    }
    
}

//------------ Loop

void loop()                                                                                                              //The loop is relatively straight forward.
{
  piezoRead();                                                                                                           //First, the output of the piezo electric elements get read into memory. 
  piezoState();                                                                                                          //Then, piezoState(); sends a midi message if the conditions are right.
  laststate();                                                                                                           //Finally, the "last state" loop gets updated to the array read at the beginning of the loop.
}
