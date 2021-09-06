
#include <PinChangeInt.h>
#include <Wire.h>

const int ZC_pins = 2;
const int dimmerPins[] = {9,10};
int delay_dim = 0;
int dimmerValue[]={4000,4000};
int last_CH1_state = 0;
const int PinCLK1 =5;
const int PinDT1 =4;
const int PinCLK2 =7;
const int PinDT2 =6;
int change = 0;

int Dim1 = 50;  
int Dim2 = 50;

void requestDim(){
  Wire.write(change);
  Wire.write(Dim1);
  Wire.write(Dim2);
  change=0;
}
void ZCsetup(){
  attachInterrupt(digitalPinToInterrupt(ZC_pins), zeroCrossing, CHANGE);
}
void dimmersetup(){
  //Configure dimmer pins
  for (int i=0;i<sizeof(dimmerPins)/sizeof(int);i++)
  {
    pinMode(dimmerPins[i], OUTPUT);
  }
  pinMode(PinCLK1,INPUT); // Set Pin 2 (Interrupt zero) to Input
  pinMode(PinCLK2, INPUT); // Set normal 8 pin to Input
  pinMode(PinDT1,INPUT);
  pinMode(PinDT2,INPUT);

  attachPinChangeInterrupt(PinCLK1, dimmer1, CHANGE); // interrupt 0 always connected to pin 2 on Arduino UNO
  attachPinChangeInterrupt(PinCLK2, dimmer2, CHANGE); // interrupt connected to pin 11
}
void dimmerFunction(int dim[]){
    if(dim[0]-dim[1]>0)
    {
      delayMicroseconds(dim[1]);
      digitalWrite(dimmerPins[1],HIGH);
      delayMicroseconds(dim[0]-dim[1]);
      digitalWrite(dimmerPins[0],HIGH);
      delayMicroseconds(100);
      digitalWrite(dimmerPins[0],LOW);
      digitalWrite(dimmerPins[1],LOW);
      delay_dim=0;
    }
    else
    {
      delayMicroseconds(dim[0]);
      digitalWrite(dimmerPins[0],HIGH);
      delayMicroseconds(dim[1]-dim[0]);
      digitalWrite(dimmerPins[1],HIGH);
      delayMicroseconds(100);
      digitalWrite(dimmerPins[0],LOW);
      digitalWrite(dimmerPins[1],LOW);
      delay_dim=0;
    }
}
void zeroCrossing(){
  if(digitalRead(ZC_pins)==HIGH)
  {
    if(last_CH1_state==0)
    {
      delay_dim=1;
    }
  }
  else if(last_CH1_state ==1)
  {
    delay_dim=1;
    last_CH1_state=0;
  }
}
void i2cSetup(){
  Wire.begin(4);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestDim);
}
void receiveEvent(){
  char c = Wire.read(); 
  int x = Wire.read(); 
  if(c=='F'){
     dimmerValue[1]=map(x,0,100,8000,10);
     Dim2=x;
  }
  if(c=='B'){
     dimmerValue[0]=map(x,0,100,8000,10);
     Dim1=x;
  }
}
void dimmer1() {
  delay(10); // delay for Debouncing Rotary Encoder
  if (digitalRead(PinCLK1)) {
    if (digitalRead(PinDT1)) {
      if (Dim1 < 96){
        Dim1 = Dim1+5;
        Serial.println(Dim1);
        dimmerValue[0] = map(Dim1,0,100,8000,10); 
        change=1;
      }
    }
    if (!digitalRead(PinDT1)) {
      if (Dim1 > 5){
        Dim1 = Dim1-5; 
        Serial.println(Dim1);
        dimmerValue[0] = map(Dim1,0,100,8000,10); 
        change = 1;
      }
    }
  } 
}
void dimmer2() {
  delay(10); // delay for Debouncing Rotary Encoder
  if (digitalRead(PinCLK2)) {
    if (digitalRead(PinDT2)) {
      if (Dim2 < 96){
        Dim2 = Dim2+5;
        Serial.println(Dim2);
        dimmerValue[1] = map(Dim2,0,100,8000,10); 
        change = 1;
      }
    }
    if (!digitalRead(PinDT2)) {
      if (Dim2 >5){
        Dim2 = Dim2-5; 
        Serial.println(Dim2);
        dimmerValue[1] = map(Dim2,0,100,8000,10); 
        change = 1;
      }
    }
  } 
}
void setup() {
  delay( 1000 ); // power-up safety delay
  ZCsetup();
  dimmersetup();
  i2cSetup();
}
void loop() {
  if(delay_dim==1){
    dimmerFunction(dimmerValue);
  }
}
