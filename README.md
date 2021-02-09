# Arduino-Projects-
This is a place where I put my Arduino/Atmega328 Projects


## Better Midi Drums

Better Midi Drums 
Written by Jacob Fifield in April 2020

A problem with many Arduino MIDI drumset tutorials is that they usually call a delay() function to make the midi note a reasonable length. The problem with that is when you try
to hit 2 drums at once, the later of the 2 inputs gets eaten completely due to the system pause in delay(). My goal was to create a MIDI drumset that would allow the user to hit multiple notes at once. Another goal was to make a velocity function that felt a bit more reactive. 

[Better MIDI Drums](Arduino-Projects/Better_Midi_Drums/Better-Midi-Drums.cpp)


## Multi Dimensional Tremolo

Multi-Dimensional Tremolo Guitar Pedal

by Jacob Fifield September 2020

A polyrhymic take on a tremolo pedal!

The core concept of this Guitar pedal is to bring polyrhythms and some higher frequency Amplitude Modulation into the world of tremolo.
Usually the pattern of a tremolo pedal is unchanging, and its dynamics are limited to only a single symmetrical on/off state.
The Multi-Dimensional Tremolo Pedal changes all of that, allowing the user to input custom patterns and play with complex polyrhythms.
This pedal has 2 overall modes that are broken down into sub modes:

### PolyMode:

#### Gridded PolyMode
This mode make sure that when you adjust the "Poly" Element, the signal change is slightly delayed until the next cycle starts. This is so that any poly change will always 
be aligned with the master timer
    
#### Free PolyMode
This mode lets the user adjust the "Poly" Element in real time, disregarding the syncronization of the lower division. This is so that the uiser can adjust settings freely
without being locked to a grid (free time).

### CustomMode:

#### Record/Playback CustomMode
This mode lets the user record a series of ON/OFF times and plays the sequence back according to speed of the Master timer. This mode will sync both channels in order 
showcase the user's pattern.

#### Killswitch CustomMode
This mode lets the user play the tremolo signal directly by passing control of the ON/OFF state of the transistors directly to the ON/OFF state of momentary Push Button 2
Should have a really nice button that's rugged, sensitive and fun to push.

The "interval" is the time of one half of on/off for the lower division, and the "poly_selector" is the of one half cycle of on/off for the ployrhythmic element
These ON/OFF states are used to drive a photo transistor to cut an audio signal on and off. The two audio channels being turned on and off create together create a more dynamic
and "tuplet aware" tremolo circuit as the circuit has a different volume level that is dependant on the overall logic state of the two transistors.
Putting these two outputs into a passive mixer circuit should let the user emphasize either the poly rhythmic element or the lower division.

Pin layout cheatsheet:

A0 - 2K Ohm Potentiometer (Rate knob)

A1 - 10K Ohm Potentiometer (Poly knob, Will be replaced with notched pot)

D12 - Push Button 2 (Tap Tempo/ Custom Pattern Input)

D11 - Push Button 1 (Sync+Unsync/ Record Custom Pattern/Play Tremolo)

D09 - Output Channel 2 (Output to phototransistor corrispoonding to channel two of the opamp)

D08 - Output Channel 1 (Output to phototransistor corrispoonding to channel one of the opamp)

D07 - Toggle Switch (Custom Mode/Poly Mode)

D06 - Toggle Switch (Utility)

[Multi Dimensional Tremolo](Arduino-Projects-/Multi_Dimensional_Tremolo/Multi-Dimensional-Tremolo.cpp)

## Cable Tester

Written By Jacob Fifield Febuary 2021
Description coming soon....
