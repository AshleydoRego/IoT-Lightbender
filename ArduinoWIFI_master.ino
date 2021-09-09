//Include required libraries
#include <FastLED.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <GyverOLED.h>

//Wifi Details - Enter your credentials
const char ssid[] = "";        
const char pass[] = "";   
int status = WL_IDLE_STATUS;

//MQTT Server Details - Enter your credentials
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
int LED_brightness = 50;
int colorIndex = 10;
long colorhex;
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
CRGB leds[num_LEDs];
uint8_t red;
uint8_t green;
uint8_t blue;

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
int Fan1_Status =1;
int Bulb1_Status =1;
int Bulb2_Status =1;
int Plug_Status =1;
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
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while ( WiFi.status() != WL_CONNECTED ) 
    {
      WiFi.begin( ssid, pass );
      Serial.print(".  ");
      delay( 1000 );
    }
    Serial.println("");
    Serial.println( "Connected" );
    disptext("WIFI CONNECTED");
    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    printMacAddress(mac);

    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print the MAC address of the router you're attached to:
    byte bssid[6];
    WiFi.BSSID(bssid);
    Serial.print("BSSID: ");
    printMacAddress(bssid);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.println(rssi);

    // print the encryption type:
    byte encryption = WiFi.encryptionType();
    Serial.print("Encryption Type:");
    Serial.println(encryption, HEX);
    Serial.println();
}
void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
void pressInterrupt() {
  if (millis()-lastFire < 200) 
  {
    return;
  }
  lastFire = millis();
  
  configureDistinct();

  for (int i=0;i<sizeof(buttonPins)/sizeof(int);i++) 
  {
    if (!digitalRead(buttonPins[i])) 
    {
      buttonAction(i);
    }
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
      Led_Power("ab");
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
  Wire.beginTransmission(4); // transmit to device #4
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
  if (led_Status == 1) return;
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
  if(led_Status == 1 || strcmp(message,"ON")==0)
  {
    FillLEDsFromPaletteColors(colorIndex);
    FastLED.show();
    led_Status = 0;    
    changeFlag = 4;
  }
  else if(led_Status == 0 || strcmp(message,"ON")==0){
    for (int i=0;i<num_LEDs;i++)
    {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    led_Status = 1; 
    changeFlag = 4;  
  }
  
}
void Led_fun(char message){
  if(strcmp(message,"ON")==0){
  }
  else if(strcmp(message,"OFF")==0){
  }
}
void HexToRgb(String hex){
  if(hex[0] == '#')  colorhex = strtol(&hex[1], NULL, 16);
  else colorhex = strtol(&hex[0], NULL, 16);
  red = colorhex >> 16;
  green = colorhex >> 8 & 0xFF;
  blue = colorhex & 0xFF;
}
void LED_SetColor(char* incoming){
  String colour = String(incoming);
  HexToRgb(colour);
  for( int i = 0; i < num_LEDs; ++i) 
  {
      leds[i] = CRGB(red, green, blue);
  }
  FastLED.show();
  changeFlag = 6;
}
void mqttCallback(char* topic, byte* payload, unsigned int length){
  memcpy(msg,payload,length);
  msg[length] = '\0';
  Serial.print("Message recieved in topic [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(msg);
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
  if(strcmp(topic,"ToDevice/Switchboard/LED/STATUS")==0){
    Led_Power(msg);
  }
  if(strcmp(topic,"ToDevice/Switchboard/LED/BRIGHTNESS")==0){
    FastLED.setBrightness(map(msg1.toInt(),0,100,0,255));
    LED_brightness = msg1.toInt();
    FastLED.show();
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
  if(strcmp(topic,"ToDevice/Switchboard/LED/FUN")==0){
    Led_fun(msg);
  }   
}
void reconnectMQTT(){
  disptext("CONNECTING TO SERVER");
  while (!MQTTclient.connected()){
    if(WiFi.status() != WL_CONNECTED) {
      Serial.print("Wifi Status code = ");
      Serial.println(WiFi.status());
      connectWifi();
    }
    Serial.print("Wifi Status code = ");
    Serial.println(WiFi.status());
    Serial.println("Attempting to connect to MQTT broker at HiveMQ");
    if(MQTTclient.connect(mqtt_server,mqtt_username,mqtt_password)){
      disptext("CONNECTED TO SERVER");
      disconnectCounter = 0;
      Serial.print("Connected to MQTT broker at ");
      Serial.println("HiveMQ"); 
      snprintf(topic,64,"ToHost/%s",deviceID);
      snprintf(msg,64,"CONNECT");
      bool conn = MQTTclient.publish(topic,msg);
      if (conn){
        Serial.print("Succesfuly published to [");
        Serial.print(topic);
        Serial.print("]: ");
        Serial.println(msg);
      }
      else{
        Serial.println("Response Not Published");
      }
      
      snprintf(topic,32,"ToDevice/%s/#",deviceID);
      conn = MQTTclient.subscribe(topic,1);
      if (conn){
        Serial.print("Subscribed to Topic [");
        Serial.print(topic);
        Serial.println("]");
      }
      else{
        Serial.println("Not Subscribed");
      }
      mqttPublishAll();
    }
    
    else{
      Serial.print("Connection to the MQTT server failed, rc = ");
      disptext("DISCONNECTED FROM SERVER");
      Serial.println(MQTTclient.state());
      Serial.println("Trying again in 1 seconds");
      delay(1000);
      disconnectCounter++;
      if(disconnectCounter>2){
        Serial.println("Software disconnect");
        resetArduino();
      }
    }
  }
}
void checkForChange(){
  if (changeFlag == 1){
    snprintf(topic,64,"ToDevice/App/BULB1");
    if(Bulb1_Status==1) snprintf(msg,64,"OFF");
    else snprintf(msg,64,"ON");
    mqttPublish(msg, topic);
    changeFlag=0;        
  }
  if (changeFlag == 2){
    snprintf(topic,64,"ToDevice/App/FAN1");
    if(Fan1_Status==1) snprintf(msg,64,"OFF");
    else snprintf(msg,64,"ON");
    mqttPublish(msg, topic);
    changeFlag=0;
  }
  if (changeFlag == 3){
    snprintf(topic,64,"ToDevice/App/BULB2");
    if(Bulb2_Status==1) snprintf(msg,64,"OFF");
    else snprintf(msg,64,"ON");
    mqttPublish(msg, topic);
    changeFlag=0;
  }        
  if (changeFlag == 4){
    snprintf(topic,64,"ToDevice/App/LED/STATUS");
    if(led_Status==1) snprintf(msg,64,"OFF");
    else snprintf(msg,64,"ON");
    mqttPublish(msg, topic);
    changeFlag=0;
  }   
  if (changeFlag == 5){
    snprintf(topic,64,"ToDevice/App/LED/BRIGHTNESS");
    snprintf(msg,64,"%d",LED_brightness);
    mqttPublish(msg, topic);
    changeFlag=0;
  }   
  if (changeFlag == 6){
    snprintf(topic,64,"ToDevice/App/LED/COLOUR");
    String myString = String(colorhex);
    myString.toCharArray(msg, 64);
    //snprintf(msg,64,colorhex);
    mqttPublish(msg, topic);
    changeFlag=0;    
  }
  if (changeFlag == 7){
    snprintf(topic,64,"ToDevice/App/PLUG");
    if(Plug_Status==1) snprintf(msg,64,"OFF");
    else snprintf(msg,64,"ON");
    mqttPublish(msg, topic);
    changeFlag=0;       
  }
  if (changeFlag == 8){
    snprintf(topic,64,"ToDevice/App/BULB1/DIM");
    snprintf(msg,64,"%d",Dim1);
    mqttPublish(msg, topic);
    snprintf(topic,64,"ToDevice/App/FAN1/DIM");
    snprintf(msg,64,"%d",Dim2);
    mqttPublish(msg, topic);
    changeFlag=0;        
  }
}
void mqttPublish(char* message, char* TOPIC){
  bool success;
  success = MQTTclient.publish(TOPIC,message);
  if (success){
    Serial.print("Succesfuly published to [");
    Serial.print(TOPIC);
    Serial.print("] :");
    Serial.println(message);
  }
  else{
    Serial.println("Response Not Published");
  }
}
void mqttPublishAll(){
  snprintf(topic,64,"ToDevice/App/BULB1");
  if(Bulb1_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  mqttPublish(msg,topic);
  
  snprintf(topic,64,"ToDevice/App/FAN1");
  if(Fan1_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  mqttPublish(msg,topic);
    
  snprintf(topic,64,"ToDevice/App/BULB2");
  if(Bulb2_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  mqttPublish(msg,topic);
  
  snprintf(topic,64,"ToDevice/App/LED/STATUS");
  if(led_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  mqttPublish(msg,topic);
  
  snprintf(topic,64,"ToDevice/App/PLUG");
  if(Plug_Status==1) snprintf(msg,64,"OFF");
  else snprintf(msg,64,"ON");
  mqttPublish(msg,topic);
  
  snprintf(topic,64,"ToDevice/App/BULB1/DIM");
  snprintf(msg,64,"%d",Dim1);
  mqttPublish(msg,topic);
  
  snprintf(topic,64,"ToDevice/App/FAN1/DIM");
  snprintf(msg,64,"%d",Dim2);
  mqttPublish(msg,topic);

  snprintf(topic,64,"ToDevice/App/LED/BRIGHTNESS");
  snprintf(msg,64,"%d",LED_brightness);
  mqttPublish(msg,topic);

  snprintf(topic,64,"ToDevice/App/LED/COLOUR");
  String myString = String(colorhex);
  myString.toCharArray(msg, 64);
  //snprintf(msg,64,colorhex);
  mqttPublish(msg,topic);
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
  Serial.begin(115200);
  Wire.begin();
  setupPeripherals();
  connectWifi();
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
