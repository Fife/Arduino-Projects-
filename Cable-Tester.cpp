


/****** LED Pin Setup *******/

const int greenLED = 10;
const int yellowLED = 11;
const int redLED = 12;
const int buttonPin = 7;

/****** Input Setup ******/
const int in0 = 0;
const int in1 = 1;
int input0 = 0;
int input1 = 0;
unsigned long currentMillis = 0;
unsigned long prevMillis = 0;
const int sampleRate = 44100;
int prev_input0 = 0;
int current_input0 = 0;
int prev_input1 = 0;
int current_input1 = 0;
bool readingFlag = false;

/****** Min/Max Variables ******/
int max1=0;
int max0 =0; 
int min0 = 500;
int min1 = 500;
int diff = 0;
unsigned long lastButtonPress = 0;

void setup() {

pinMode(greenLED, OUTPUT);
pinMode(yellowLED, OUTPUT);
pinMode(redLED, OUTPUT);
pinMode(9, OUTPUT);
pinMode(7, INPUT_PULLUP);
//Serial.begin(9600);
Animation (6);
Animation (7);
}

void ClearLED()
{
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
}

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

void checkDiff(int diff2)
{
  if (diff2< 10)
  {
    ClearLED();
    Animation(4);
  }
  if (diff2 < 20 && diff2 > 10)
  {
    ClearLED();
    Animation(2);
  }
  if (diff2 < 50 && diff2 > 20)
  {
    ClearLED();
    Animation(1);
  }
  if (diff2 > 50)
  {
    ClearLED();
    Animation(0);
  }
}

void checkBadGround(int internalSignalAmp)
{
  if (internalSignalAmp <200)
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
    tone(9, 440); //Generate 440hz tone on pin 9
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
      Animation(6);
      int i = 0;
      while (i<2000) // We will pull 2000 samples at our sample rate
      {
        currentMillis = millis();
        if (currentMillis - prevMillis  >= 1/sampleRate) //If we are in our sample rate will read from both our inputs and do some min/max math
        {
          prevMillis = currentMillis;
          current_input0 = analogRead(in0);
          current_input1 = analogRead(in1);
          if (current_input0 > max0) max0 = current_input0;
          if (current_input0 < min0) min0 = current_input0;
          if (current_input1 > max1) max1 = current_input1;
          if (current_input1 < min1 && current_input1>10) min1 = current_input1;
          i++;
        }
      }
      int amp0 = max0 - min0;
      int amp1 = max1 - min1;
      diff = amp0-amp1;
      if (diff <0) {diff =diff*(-1);}
      if (diff == 0)
      {
       checkBadGround(amp0); 
      }
      else 
      {
        checkDiff(diff);
      }
      readingFlag = false;
    }
    prevMillis = currentMillis; 
    //Serial.println(diff);
    //Serial.println(" ");
}
