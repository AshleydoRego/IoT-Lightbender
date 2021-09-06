//Include required libraries
#include <FastLED.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <GyverOLED.h>

//Wifi Details
const char ssid[] = "";        
const char pass[] = "";   
int status = WL_IDLE_STATUS;

//MQTT Server Details
const char mqtt_server[] = "";
const char mqtt_username[] = "";
const char mqtt_password[] = "";

//For MQTT Connection
WiFiSSLClient client;
PubSubClient MQTTclient(client);
long lastMsgTime = 0;
char msg[64];
char topic[64];
int disconnectCounter = 0;
const char * deviceID = "Switchboard";

//Pin Assignment
const int buttonPins[] = {0,1,2,3,4,5,6};
const int commonPin = 7;
const int LED_Pin = 8;
const int resetPin = 9;
const int bulb1_RelayPin = 10;
const int fan1_RelayPin = 11;
const int bulb2_RelayPin = 12;
const int plug_RelayPin = 13;

//Button bounce time
unsigned long lastFire = 0;

//LED variables
const int num_LEDs = 60;
int LED_brightness = 150;
int colorIndex = 10;
long colorhex = strtol("#eb4034", NULL, 16);
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
CRGB leds[num_LEDs];

//Aditional Variables
int changeFlag = 0;
/* 1- Bulb1 power change
 * 2- Fan1 power change
 * 3- Bulb2 power change
 * 4- LED power change
 * 5- LED brightness change
 * 6- LED colour change
 * 7- Plug power change
 * 8- Dimmer change
 */
int led_Status = 1;
int Fan1_Status = 1;
int Bulb1_Status = 1;
int Bulb2_Status = 1;
int Plug_Status = 1;
/* 1 - OFF
 * 0 - ON
 */
int Dim1 = 50;
int Dim2 = 50;

//OLED Display
GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;

void setupPeripherals(){
  //OLED setup
  oled.init();
  oled.clear();   
  oled.update();  

  //LED Setup
  FastLED.addLeds<WS2812B, LED_Pin, GRB>(leds, num_LEDs);
  FastLED.setBrightness(LED_brightness);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  
  // Configure buttons
  pinMode(commonPin, INPUT_PULLUP);
  configureCommon();
  attachInterrupt(digitalPinToInterrupt(commonPin), pressInterrupt, FALLING);

  //Relay Setup
  for (int i=10;i<14;i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
  }

  //MQTT Setup
  MQTTclient.setServer(mqtt_server,8883);
  MQTTclient.setCallback(mqttCallback);
}
void connectWifi(){
    disptext("CONNECTING WIFI");
    while ( WiFi.status() != WL_CONNECTED ) 
    {
      WiFi.begin( ssid, pass );
      delay( 1000 );
    }
    disptext("WIFI CONNECTED");
}
void pressInterrupt() {
  if (millis()-lastFire < 200) return;
  
  lastFire = millis();
  configureDistinct();
  for (int i=0;i<sizeof(buttonPins)/sizeof(int);i++) 
  {
    if (!digitalRead(buttonPins[i])) buttonAction(i);
  }
  configureCommon();
}
void configureCommon() {
  pinMode(commonPin, INPUT_PULLUP);
  for (int i=0;i<sizeof(buttonPins)/sizeof(int);i++) 
  {
    pinMode(buttonPins[i], OUTPUT);
    digitalWrite(buttonPins[i], LOW);
  }
}
void configureDistinct() {
  pinMode(commonPin, OUTPUT);
  digitalWrite(commonPin, LOW);
  for (int i=0;i<sizeof(buttonPins)/sizeof(int);i++) 
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}
void relaySwitch(int pin, char message){
  if(strcmp(message,"ON")==0){
    digitalWrite(pin,LOW);
  }
  else if(strcmp(message,"OFF")==0){
    digitalWrite(pin,HIGH);
  }
  else{
    digitalWrite(pin,!digitalRead(pin));
  }
  switch(pin){
    case bulb1_RelayPin:
      Bulb1_Status = digitalRead(pin);
      changeFlag = 1;
      break;
    case fan1_RelayPin:
      Fan1_Status = digitalRead(pin);
      changeFlag = 2;
      break;
    case bulb2_RelayPin:
      Bulb2_Status = digitalRead(pin);
      changeFlag = 3;
      break;   
    case plug_RelayPin:
      Plug_Status = digitalRead(pin);
      changeFlag = 7;
      break;
  }
}
void buttonAction(int button) {
  switch(button)
  {
    case 0:
      relaySwitch(bulb2_RelayPin,"a");
      break;
    case 1:
      relaySwitch(bulb1_RelayPin,"a");
      break;
    case 2:
      relaySwitch(fan1_RelayPin,"a");
      break;
    case 3:
      Led_Power("a");
      break;
    case 4:
      Led_CycleBrightness();
      break;
    case 5:
      Led_CycleColour();
      break;
    case 6:
      relaySwitch(plug_RelayPin,"a");
      break;      
  }
}
void setDim(char c, byte i){
  Wire.beginTransmission(4); 
  Wire.write(c);        
  Wire.write(i);              
  Wire.endTransmission(); 
  requestDim();
  changeFlag = 7; 
}
void requestDim(){
  Wire.requestFrom(4, 3);
  int a = Wire.read();
  Dim1 = Wire.read();
  Dim2 = Wire.read();
  if (a == 1){
    changeFlag = 8;
  }
}
void Led_CycleColour(){
  colorIndex += 25;
  if (colorIndex > 255)
  {
    colorIndex=2;
  }
  FillLEDsFromPaletteColors(colorIndex);
  FastLED.show();
  changeFlag = 6;
}
void FillLEDsFromPaletteColors( uint8_t colorIndex){    
    for( int i = 0; i < num_LEDs; ++i) 
    {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, LED_brightness, currentBlending);
    }
}
void Led_CycleBrightness(){
  LED_brightness+=10;
  if (LED_brightness > 100)
  {
    LED_brightness=10;
  }
  FastLED.setBrightness(map(LED_brightness,0,100,0,255));
  FastLED.show();
  changeFlag = 5;  
}
void Led_Power(char message){
  if(led_Status==0||strcmp(message,"OFF"))
  {
    for (int i=0;i<num_LEDs;i++)
    {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    led_Status = 1; 
    changeFlag = 4;  
  }
  else if(led_Status==1||strcmp(message,"ON"))
  {
    FillLEDsFromPaletteColors(colorIndex);
    FastLED.show();
    led_Status = 0;    
    changeFlag = 4;
  }
}
void LED_SetColor(char incoming){
  colorhex = strtol(incoming, NULL, 16);
  for( int i = 0; i < num_LEDs; ++i) 
  {
      leds[i] = colorhex;
  }
  FastLED.show();
  changeFlag = 6;
}
void mqttCallback(char* topic, byte* payload, unsigned int length){
  memcpy(msg,payload,length);
  msg[length] = '\0';
  String msg1 = String(msg);
    if(strcmp(topic,"ToDevice/Switchboard/BULB1")==0){
    relaySwitch(bulb1_RelayPin,msg);
  }
  if(strcmp(topic,"ToDevice/Switchboard/BULB2")==0){
    relaySwitch(bulb2_RelayPin,msg);
  }
  if(strcmp(topic,"ToDevice/Switchboard/FAN1")==0){
    relaySwitch(fan1_RelayPin,msg);
  }
  if(strcmp(topic,"ToDevice/Switchboard/PLUG")==0){
    relaySwitch(plug_RelayPin,msg);
  }
  if(strcmp(topic,"ToDevice/Switchboard/LED")==0){
    Led_Power(msg);
  }
  if(strcmp(topic,"ToDevice/Switchboard/LED/BRIGHTNESS")==0){
    FastLED.setBrightness(map(msg1.toInt(),0,100,0,255));
    LED_brightness = msg1.toInt();
    changeFlag=5;
  }
  if(strcmp(topic,"ToDevice/Switchboard/LED/COLOUR")==0){
    LED_SetColor(msg);
    changeFlag=6;
  }
  if(strcmp(topic,"ToDevice/Switchboard/FAN1/DIM")==0){
    setDim('F',(msg1.toInt()));
    changeFlag=8;
    Dim2=msg1.toInt();
  }
  if(strcmp(topic,"ToDevice/Switchboard/BULB1/DIM")==0){
    setDim('B',(msg1.toInt()));
    changeFlag=8;
    Dim1=msg1.toInt();
  }   
}
void reconnectMQTT(){
  disptext("CONNECTING TO SERVER");
  while (!MQTTclient.connected()){
    if(WiFi.status() != WL_CONNECTED) {
      connectWifi();
    }
    if(MQTTclient.connect(mqtt_server,mqtt_username,mqtt_password)){
      disptext("CONNECTED TO SERVER");
      disconnectCounter = 0;
      snprintf(topic,64,"ToHost/%s",deviceID);
      snprintf(msg,64,"CONNECT");
      MQTTclient.publish(topic,msg);

      snprintf(topic,32,"ToDevice/%s/#",deviceID);
      MQTTclient.subscribe(topic,1);
      mqttPublishAll();
    }
    else{
      disptext("DISCONNECTED FROM SERVER");
      delay(1000);
      disconnectCounter++;
      if(disconnectCounter>2){
        resetArduino();
      }
    }
  }
}
void checkForChange(){
  if (changeFlag!=0){
    switch(changeFlag){
      case 1:
        snprintf(topic,64,"ToDevice/App/BULB1");
        if(Bulb1_Status==1) snprintf(msg,64,"OFF");
        else snprintf(msg,64,"ON");
        MQTTclient.publish(topic,msg);
        changeFlag=0;        
        break;
      case 2:
        snprintf(topic,64,"ToDevice/App/FAN1");
        if(Fan1_Status==1) snprintf(msg,64,"OFF");
        else snprintf(msg,64,"ON");
        MQTTclient.publish(topic,msg);
        changeFlag=0;        
        break;
      case 3:
        snprintf(topic,64,"ToDevice/App/BULB2");
        if(Bulb2_Status==1) snprintf(msg,64,"OFF");
        else snprintf(msg,64,"ON");
        MQTTclient.publish(topic,msg);
        changeFlag=0;        
        break;  
      case 4:
        snprintf(topic,64,"ToDevice/App/LED/STATUS");
        if(led_Status==1) snprintf(msg,64,"OFF");
        else snprintf(msg,64,"ON");
        MQTTclient.publish(topic,msg);
        changeFlag=0;    
        break;  
      case 5:
        snprintf(topic,64,"ToDevice/App/LED/BRIGTHNESS");
        snprintf(msg,64,"%d",LED_brightness);
        MQTTclient.publish(topic,msg);
        changeFlag=0;    
        break; 
      case 6:
        snprintf(topic,64,"ToDevice/App/LED/COLOUR");
        String myString = String(colorhex);
        myString.toCharArray(msg, 64);
        //snprintf(msg,64,colorhex);
        MQTTclient.publish(topic,msg);
        changeFlag=0;    
        break;
      case 7:
        snprintf(topic,64,"ToDevice/App/PLUG");
        if(Plug_Status==1) snprintf(msg,64,"OFF");
        else snprintf(msg,64,"ON");
        MQTTclient.publish(topic,msg);
        changeFlag=0;       
        break;   
      case 8:
        snprintf(topic,64,"ToDevice/App/BULB1/DIM");
        snprintf(msg,64,"%d",Dim1);
        MQTTclient.publish(topic,msg);
        snprintf(topic,64,"ToDevice/App/FAN1/DIM");
        snprintf(msg,64,"%d",Dim2);
        MQTTclient.publish(topic,msg);
        changeFlag=0;        
        break;  
    }
  }
}
void mqttPublishAll(){
  snprintf(topic,64,"ToDevice/App/BULB1");
  if(Bulb1_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  MQTTclient.publish(topic,msg);
  
  snprintf(topic,64,"ToDevice/App/FAN1");
  if(Fan1_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  MQTTclient.publish(topic,msg);
    
  snprintf(topic,64,"ToDevice/App/BULB2");
  if(Bulb2_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  MQTTclient.publish(topic,msg);
  
  snprintf(topic,64,"ToDevice/App/LED/STATUS");
  if(led_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  MQTTclient.publish(topic,msg);
  
  snprintf(topic,64,"ToDevice/App/PLUG");
  if(Plug_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  MQTTclient.publish(topic,msg);
  
  snprintf(topic,64,"ToDevice/App/BULB1/DIM");
  snprintf(msg,64,"%d",Dim1);
  MQTTclient.publish(topic,msg);
  
  snprintf(topic,64,"ToDevice/App/FAN1/DIM");
  snprintf(msg,64,"%d",Dim2);
  MQTTclient.publish(topic,msg);

  snprintf(topic,64,"ToDevice/App/LED/BRIGHTNESS");
  snprintf(msg,64,"%d",LED_brightness);
  MQTTclient.publish(topic,msg);

  snprintf(topic,64,"ToDevice/App/LED/COLOR");
  String myString = String(colorhex);
  myString.toCharArray(msg, 64);
  //snprintf(msg,64,colorhex);
  MQTTclient.publish(topic,msg);
}
void resetArduino(){
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
}
void disptext(String str){
  oled.clear();
  oled.home();            
  oled.setScale(2);
  oled.autoPrintln(true);   
  oled.print(str);   
  oled.update();
}
void setup() {
  Wire.begin();
  setupPeripherals();
  connectWifi();
  reconnectMQTT();
  delay(1500);
}
void loop() {  
  if(!MQTTclient.connected()) {
    reconnectMQTT();
  }
  MQTTclient.loop();
  checkForChange();
  requestDim();  
  delay(10);
}
