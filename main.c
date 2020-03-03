#include<stdlib.h>
#include <stdio.h>
#include <pic32mx.h>
volatile int* pe = (volatile int*) 0xbf886110;    //led counter, set as global
int counter = 0;
int topRight = 0;
int topLeft = 0;
int botRight = 0;
int botLeft = 0;
int topAverage;
int botAverage;
int rightAverage;
int leftAverage;
int clockwiseSpeed = 3500;          //3500 -> 5% duty cycle -> 1ms for 50hz -> clockwise spin
int botcounterclockwiseSpeed = 3890;   //3890 -> 10% duty cycle -> 2 ms for 50hz ->counter clockwise spin
int topcounterclockwiseSpeed = 4000;   //4000 -> 10% duty cycle -> 2 ms for 50hz ->counter clockwise spin

void picInit(){
  volatile int* trise = (volatile int*)0xbf886100;    //setting leds as output
  *trise = *trise & 0xFF00;
  TRISD = TRISD & 0x0FE0;
  *pe = 0x0;                  //configures the lights

  //Motor initialization
  T2CONSET = 0x50;    //sets prescale to 1:32
  T2CONSET = 0x8;
  PR2 = 49999;        //80Mhz/32/50
  TMR2 = 0;           //timer starts at 0 and goes to PR2
  //Top motor
  OC1CON = 0x8026;    //configures the OC1 pins settings  (top motor)
  OC1RS = 0;
  OC1R = 0;        //sets duty cycle = (PR2+1)/OC1R = 50hz = 20 ms
  //Bottom motor
  OC2CON = 0x8026;    //configures the OC2 pins settings   (bottom motor)
  OC2RS = 0;
  OC2R = 0;
  T2CONSET = 0x8000;    //Start the timer

  //LDR initialization
  AD1CON1 = 0x4E0;    //Configure Analog Digital Converter
  AD1CON2 = 0x0;
  AD1CON3 |= (0x1 << 15); //Sets the clock source to the ADC Internal clock
  AD1CON1 |= (0x1 << 15); //turns it on
}

adcRead(int aPin){    //Function to read the value of the selected Analog pin
  AD1PCFG = ~(1 << aPin);         //Sets the pin as analog input
  TRISBSET = (1 << aPin);         //Sets the pin as input
  AD1CHS = (1 << (15 + aPin));    //Selects the pin to read from

  AD1CON1 |= (0x1 << 1);          //Starts the sampling
  while(!(AD1CON1 & (0x1 << 1))); //Checks if the sample
  while(!(AD1CON1 & 0x1));        //Checks if convertion is done

  return ADC1BUF0;
}

void main(){
  picInit();    //initialization of the pic and all the pins
while(1){       //feedback loop
    if(TMR2 >= PR2)           //Check timer
    {
      TMR2 = 0;               //Reset timer
      counter++;              //Counter to measure length of spinning
      if (counter == 3){  //stops the motors
        OC1RS = 0;
        OC2RS = 0;
      }
      if(counter >= 5)       //ish counter/10 = x seconds
      {
        counter = 0;
        //Check lightlevels for each sensor
        topLeft = adcRead(3);
        topRight = adcRead(4);
        botLeft = adcRead(5);
        botRight = adcRead(6);

        rightAverage = ((topRight + botRight) / 2);
        leftAverage = ((botLeft + topLeft) / 2)+100;    //Due to difference between the sensors we have added, 100
        topAverage = ((topLeft + topRight) / 2)+160;    //values to make them closer to each other, 230
        botAverage = ((botLeft + botRight) / 2);

        (*pe) = botAverage;                //led counter

        if(rightAverage > leftAverage+20){        //turns right if the right average is greater
          OC2RS = botcounterclockwiseSpeed;
          }
        else if(leftAverage > rightAverage+20){ //turns left if the left average is greater
          OC2RS = clockwiseSpeed;
          }
        else            //if the right and left are withing +-20 of eachother then it remains
        {
          OC2RS = 0;
        }

        if(topAverage > botAverage+20){     //angles clockwise if top is greater
          OC1RS = clockwiseSpeed; //towards blue
        }
        else if(botAverage > topAverage+20){  //angles counterclockwise if bottom is greater
          OC1RS = topcounterclockwiseSpeed;
          }
        else      //remains if they are within +-30
        {
          OC1RS = 0;
        }
      }
    }
  }
}
