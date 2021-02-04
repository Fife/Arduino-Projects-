/****** Pin Setup *******/

const int greenLED = 10;
const int yellowLED = 11;
const int redLED = 12;
const int buttonPin = 7;
const int outputPin = 9;
const int in0 = 0;
const int in1 = 1;

/****** Input Setup ******/
int input0 = 0;
int input1 = 0;
unsigned long currentMillis = 0;
unsigned long prevMillis = 0;
int prev_input0 = 0;
int current_input0 = 0;
int prev_input1 = 0;
int current_input1 = 0;
bool readingFlag = false;

/****** Frequency and sample rate ******/
float freq = 440.0;
const int sampleRate = 44100;

/****** Min/Max Variables ******/
int max1 = 0;
int max0 =0; 
int min0 = 500;
int min1 = 500;
int diff = 0;
unsigned long lastButtonPress = 0;


/* Setup pin i/o mode and play a startup LED animation */
void setup() {
pinMode(greenLED, OUTPUT);
pinMode(yellowLED, OUTPUT);
pinMode(redLED, OUTPUT);
pinMode(outputPin, OUTPUT);
pinMode(buttonPin, INPUT_PULLUP);
Animation (6);
Animation (7);
//Serial.begin(9600); // Serial Line for Debugging
}

void ClearLED() // This function turns off all the LEDs 
{
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
}

 /* This function takes an interger input and plays an animation based on the input
 0 Red ON
 1 Red and Yellow ON
 2 Yellow ON
 3 Yellow and Green
 4 Green
 5 Flashing Yellow and Red
 6 LED chase
 7 All lights on 
 */
void Animation (int selection) 
{
  ClearLED;
  switch(selection){
    case 0:
      digitalWrite(redLED, HIGH);
      break;
    case 1:
      digitalWrite(yellowLED, HIGH);
      digitalWrite(redLED, HIGH);
      break;
    case 2:
      digitalWrite(yellowLED, HIGH);
      break;
    case 3:
      digitalWrite(yellowLED, HIGH);
      digitalWrite(greenLED, HIGH);
      break;
    case 4:
      digitalWrite(greenLED, HIGH);
      break;
    case 5:
      for (int i=0; i<3; i++)
      {
       digitalWrite(redLED, HIGH);
       delay(100);
       digitalWrite(redLED, LOW); 
       delay(100);
       digitalWrite(yellowLED, HIGH);
       delay(100);
       digitalWrite(yellowLED, LOW); 
       delay(100);
      } 
      digitalWrite(redLED, HIGH);
      digitalWrite(yellowLED, HIGH);
      break;
    case 6:
      digitalWrite(redLED, HIGH);
      delay(100);
      digitalWrite(yellowLED, HIGH);
      delay(100);
      digitalWrite(greenLED, HIGH);
      delay(100);
      digitalWrite(redLED, LOW);
      delay(100);
      digitalWrite(yellowLED, LOW);
      delay(100);
      digitalWrite(greenLED, LOW);
      break; 
    case 7:
      digitalWrite(redLED, HIGH);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(greenLED, HIGH);
      break;
  }
}

// This function takes the amplitude difference between input 1 and input0 and does some range checks on it.
void checkDiff(int diff2) 
{
  if (diff2< 10) //Less Than 10, it passes
  {
    ClearLED();
    Animation(4);
  }
  if (diff2 < 20 && diff2 > 10) //Less than 20 but greater than 10, re-test
  {
    ClearLED();
    Animation(2);
  }
  if (diff2 < 50 && diff2 > 20) //Less than 50 but greater than 20, check connection Cable Fails
  {
    ClearLED();
    Animation(1);
  }
  if (diff2 > 50) // Cable has significant amplitude loss, Cable Fails
  {
    ClearLED();
    Animation(0);
  }
}

/* 
checkBadGround checks to see if there's a major short. A characteristic of a major short is a distorted signal with a decreased amplitude on the input0 pin
However there is no difference between the amplitude of the distorted signal on input0 and input1, so it would otherwise pass the test. This is a problem.
My solution is that since the signal is of a known amplitude, it is easy to check for a short by comparing the amplitude of input 0 against the known
signal's amplitude 
*/

void checkBadGround(int internalSignalAmp) 
{
  if (internalSignalAmp <200) //If signal amplitude is less than 200 do a special animation
  {
    Animation(1);
    Animation(5);
  }
  else 
  {
    Animation(4);
  }
}


void loop() { 
    tone(outputPin, freq); //Generate 440hz tone on pin 9
    bool buttonstate = digitalRead(buttonPin); //Read the button connected to the button pin
    
    if (buttonstate == false) //Input mode is PULLUP so when the button gets pressed it sends a LOW or FALSE
    {
      ClearLED(); //Clear LED function to turn all LEDs OFF
      max1=0;max0=0; min0 = 0;min1 = 0;diff = 0;  //Reset the maximum, minimum and amplitude difference as we are starting a new test
      lastButtonPress = millis(); //Remember the last time the button was pressed 
      readingFlag = true; //Set the reading flag to true. This means that we are currently sampling the tone from pin 9.
    }
    
    if (readingFlag== true) // If the reading flag is true
    {
      Animation(6); // Do an animation to let the user know that the cable is being tested 
      int i = 0;
      while (i<2000) // We will pull 2000 samples at our sample rate
      {
        currentMillis = millis();
        if (currentMillis - prevMillis  >= 1/sampleRate) // If we are within a frame of our sample rate
        {
          prevMillis = currentMillis; // Take note of the current time
          current_input0 = analogRead(in0); // Read the internal input
          current_input1 = analogRead(in1); // Read the external input
          if (current_input0 > max0) max0 = current_input0; //max calculation for internal 
          if (current_input0 < min0) min0 = current_input0; //min calculation for internal
          if (current_input1 > max1) max1 = current_input1; //max calculation for external
          if (current_input1 < min1 && current_input1>10) min1 = current_input1; //mincalculation for external, minimum can't be zero.
          i++;
        }
      }
      int amp0 = max0 - min0; //amplitude calculation for internal 
      int amp1 = max1 - min1; //amplitude calculation for external connection
      diff = amp0-amp1; //difference between the amplitudes
      if (diff <0) {diff =diff*(-1);} //if the difference between the amplitudes is negative (I don't know why but this happens unreliably) turn it positive.
      if (diff == 0) //if there is no difference in amplitude, there may be a bad ground problem 
      {
       checkBadGround(amp0); //Check to see if it's a bad ground problem
      }
      else //There is some difference in amplitude
      {
        checkDiff(diff); //Check how bad the difference in amplitude is.  
      }
      readingFlag = false; // We are no longer in a reading state anymore
    }
    prevMillis = currentMillis; //Used for timekeeping 
    //Serial.println(diff); //Used for Debugging
    //Serial.println(" ");
}
