//These are the two libraries that I am using.
#include <Time.h>
#include <TimeLib.h>

//Variables to declare the input pins. pressurePin is reading a force sensitive resistor.
  const int buttonPin = 11;
  const int pressurePin = A0;

//Variables the inputs are being assigned
  int buttonState = 0;
  int pressureVal = 0;

//Variables to declare the output pins
  const int rLed = 10;
  const int bLed = 9;

//Long variables to be used to flash the red LED light to get the attention of the client;
//Interval is about half a second
  const long sitInterval = 500;
  const long standInterval = 200;
  long pTimer;

/*Sets the state of the red LED, which is being used 
 *to let the client know the timer has gone off.*/
  int rLedState = LOW;
  int bLedState;

/*Checking the time passed. 
 *Hour check is filled by default for the client whereas min check will be
 *assigned for the snooze functionality*/
  int hourCheck = 1;
  int minCheck = 0;

// Will be used as a comparision between two values.
  int currentSec = 0;

//My Booleans, which check if snooze is on or the client is sitting down.
  bool snooze = false;
  bool sit = false;
  bool away = false;

//My Booleans for checking if the serial write has been run for the notification.
  bool introHasRun = false;
  bool timeUpHasRun = false;
  bool snoozeHasRun = false;
  bool standingHasRun = false;
  bool awayHasRun = false;
  bool sitTimeSet = false;
  bool standTimeSet = false;

//Nothing too special here, just setting the pin modes
void setup() {
  Serial.begin(9600);
  pinMode(rLed, OUTPUT);
  pinMode(bLed,OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {

  //There are a couple of these throughout the code. They are there so the print only runs once instead of continously. These send messages to the serial port.
  if(introHasRun != true){
    Serial.println("Hey! Before we get started make sure the force sensor is down where you will be sitting and you are not sitting on it now so it can calibrate");
    Serial.println("When you are ready to begin, sit down. When it's working the blue LED should turn on");
    introHasRun = true;
  }

  /* This code here is reading the force sensitive resistor placed on the chair of the
   * client. It is detecting to see if the client is sitting down or not.
   * 600 was the value chosen as it was high enough to avoid false positives
   * but not too high to have issues triggering.*/
  pressureVal = analogRead(pressurePin);
  if(pressureVal >= 600){
    sit = true;
    away = false;

    //This sets the time for when the client is sitting. It only will do it once.
    if(sitTimeSet != true){
      setTime(0,0,0,0,0,0);
      sitTimeSet = true;
    }
  }else{
    sit = false;
    if(standTimeSet != true){
      setTime(0,0,0,0,0,0);
      standTimeSet = true;  
    }
  }

  /* This will do everything (set the time, check the time and trigger the LED when
   * the client has sat continuously for 1 hour).
   * This code will continue until the client has gotten up */
  do{

    standTimeSet = false;
    //This lets the client know that the program is active.
    digitalWrite(bLed, HIGH);

    //This writes the current time that has passed
    //The reason for 2 digital reads is that the other one is contained in a loop.
    buttonState = digitalRead(buttonPin);
    if(buttonState == HIGH){
      
      //This bit of code stops it from printing out the same line multiple times when the client hold the button.
      if(second() != currentSec){
        //To write the number in a sentence, I have to convert the values into strings.
        Serial.println(String(minute()) + " minute(s) and " + String(second()) + " second(s) have passed");
        
        //Inputs the current second so it can be compare to later.
        currentSec = second();
      }
    }
    
    //This checks if the hour has passed, but also checks anything.
    if(hour() == hourCheck && minute() == minCheck){
      //Turns off snooze, if it was on.
      snooze = false;

      /* This code flashes the red LED continuously until either 
       * snooze is activated of the client gets up */
       while(snooze == false){

        if (timeUpHasRun != true){
          Serial.println(String(hourCheck) + " hour(s) and " + String(minCheck) + " minute(s) has passed. Time to get up!");
        }
        //Sets the time the Arduino has run in milliseconds
        unsigned long cTimer = millis();

        //Checks if the current time and past time is greater or equal to the interval of 500 milliseconds
        if(cTimer - pTimer >= sitInterval){
          pTimer = cTimer;
          //This checks the state of the LED. If it's on, turn it off and vice versa.
          if(rLedState == LOW){
            rLedState = HIGH;
          }else{
            rLedState = LOW;
          }
        }

        // +Snooze Function
        /* Reading the state of the button. The reason it's down here is it needs
         * to have something activated as soon as it is read. If it's higher up, it
         * does everything else beforehand*/
        buttonState = digitalRead(buttonPin);
        if(buttonState == HIGH){
          //If the light (aka alarm) is on, it turns it off.
          rLedState = LOW;
          //This stops the while loop turning the red LED on and off.
          snooze = true;
          //The client wants to be able to snooze for 10 minutes, so here we go.
          minCheck = minute()+5;
         
          /* If minCheck is over 60 it removes the 60 and adds 1 to the hour.
           * This allows the user to snooze forever if needed*/
           
          if (minCheck >= 60){
            minCheck = minCheck-60;
            hourCheck = hourCheck + 1;
          }
          
          if (snoozeHasRun != true){
            Serial.println("Not right now? OK, we'll remind you in 5 minutes, starting now");
            snoozeHasRun = true;
          }
          
          /* This takes the current seconds count, then subtacts that from the current 
           * second it is up to, leaving it back at 0.
           * Heres how I found that out: https://www.pjrc.com/teensy/td_libs_Time.html */
           
          adjustTime(-second());
        }  
        //Finally, light up the red LED.
        digitalWrite(rLed, rLedState);
      }
    }
  }while(sit == true);

  //This will run when the user stands up or leaves the sensor.
  while(sit == false){
    digitalWrite(bLed, LOW);
    hourCheck = 1;
    minCheck = 0;

    sitTimeSet = false;
    
    
    if(standingHasRun != true){
    Serial.println("Hey, just letting you know I think you have gotten up. If you're taking a break, have a good one.");
    standingHasRun = true;
    }
    // ++ Away Reminder
    if(minute() >= 30){
      away = true;
      do{
        if(awayHasRun != true){
          Serial.println("Hey, come back! It's been 30 minutes;");
          awayHasRun = true;
        }
        
        //Sets the time the Arduino has run in milliseconds
        unsigned long cTimer = millis();

        //Checks if the current time and past time is greater or equal to the interval of 500 milliseconds
        if(cTimer - pTimer >= standInterval){
          pTimer = cTimer;
          //This checks the state of the LED. If it's on, turn it off and vice versa.
          if(bLedState == LOW){
            bLedState = HIGH;
          }else{
            bLedState = LOW;
          }
        }
        digitalWrite(bLed, bLedState);
      }while(away == true);
    }
  }
}
