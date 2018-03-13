#include <Arduino.h>

//changed pins
//wifi off
/*

 */
 //27/3/17


/////wifi
#include <ESP8266WiFi.h>
const char* ssid = "RepeaterWirelessNetwork";
const char* password = "ddeedddd";
boolean EnableWifi = true;
/////

////MQTT
#define MQTT_KEEPALIVE 60
#include <PubSubClient.h> //MQTT library
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
//float temp = 0;
//int inPin = 5;
const char* mqtt_server = "192.168.2.254";
const char* mqtt_channel1 ="temperature1";//mqtt topic used for dallas sensor
float mqttlastTemperature;  
const int mqttWait = 6;  //how many times to wait before sending if data is unchanged
int mqttNoChange =0; //number of times temperature has remained constant

/////////

////Pushetta
float maxPushTemp = 80; // message sent if above this temperature
float minPushTemp = -129; // message sent if below this temperature
char PushApiKey[] = "42faa2f4cc6c8a142c600d088b38e82937d0d684"; // Put here your API key. It's in the dashboard
char PushChannel[] = "col temp";                               // and here your channel name
char PushserverName[] = "api.pushetta.com";
boolean lastConnected = false;
unsigned long int lastPush = 0; //use to calculate interval from last message
/////////



///Display
#include <Adafruit_GFX.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Adafruit_ST7735_ESP.h> //https://github.com/nzmichaelh/Adafruit-ST7735-Library
#include <SPI.h>
#define cs   12 //D6
#define dc   4  //D2
#define rst 2  // D4 
//DIN            D7 / GPIO13 / MOSI
//CLK            D5 / GPIO14 / SCLK
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
#define brightnessPin 16 //GPIO16 = D0
int brightness = 300; //0-1024
////////////////

//graph
const int dispx = 128; //width of LCD
const int dispy = 160;// height of LCD (pixels)
float tempHistory[dispx];
const float graphHeight = 2;  //height of graph height in degrees
float ScaleMin  = 15; // x axis minimum
float ScaleMax = 25;   //x axis maximum
int ScaledTemperature1 = 0; //essentially the number of pixels
int ScaledTemperature2 = 0; //essentially the number of pixels
boolean fullPageGraph = true;
/////////////////////////////////////////////////////////////////////////////////////////


//// temperature sensor
#include <DallasTemperature.h>
#include <OneWire.h>
#define ONE_WIRE_BUS 5//d1
//#define TEMPERATURE_PRECISION 12 // high resolution
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
float maxtemperature = 0;
float mintemperature = 88;
float temperature;
const float temperatureOffSet = 0;// calibration of temeperature
//////////////////////////////////////////////////////////////////////////////////////////////


//Sleep
extern "C" {// esp8266 sleep functions
#include "user_interface.h"
  //uint16 readvdd33(void);
  bool wifi_set_sleep_type(sleep_type_t);
  sleep_type_t wifi_get_sleep_type(void);
}
/////////////////


unsigned long MainLoopCnt = 1;
unsigned long previousMillis =0;
unsigned long currentMillis =0;


void updateGraph (void);
void drawTemperatureLine(void); 
void drawGrid () ;
void testmultifillcircles(uint8_t radius) ;
void setup_wifi() ;
void reconnect() ;
void callback(char* topic, byte* payload, unsigned int length);
void sendToPushetta(char channel[], String text) ;

void setup()   {
  /* pinMode(5, OUTPUT);// power to sensor
     digitalWrite(5, LOW);// power to sensor
  pinMode(6, OUTPUT);// power to sensor
     digitalWrite(6, HIGH);// power to sensor*/
  pinMode(brightnessPin,OUTPUT);
  analogWrite(brightnessPin, brightness);
  Serial.begin(115200);
  
  
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  delay(100);
  tft.fillScreen(ST7735_BLACK);
  delay(100);
  testmultifillcircles(60);
  delay(500);
 tft.fillScreen(ST7735_BLACK);
 
   if (EnableWifi) {
 //display mqqt commands
 tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextWrap(true);
    tft.setTextSize(2);
    tft.setCursor(1, 10);
    tft.println("MQTT");
     tft.setTextSize(1);
     
  tft.println("brightness 0-9");
  delay(200);
  tft.println("alert-low 0-99");
  tft.println("alert-high 0-99");
  delay(10000);
   }
  
  sensors.begin(); // DAllas IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
  sensors.setResolution(12);
  
  if (EnableWifi) {
   Serial.println("setting up wifi");
  setup_wifi();
  wifi_set_sleep_type(MODEM_SLEEP_T);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);}
  else  Serial.println("not setting up wifi");
 //delay(10000);

 
 ///show status on display
  tft.fillScreen(ST7735_BLACK);

    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextWrap(true);
    tft.setTextSize(1);
    tft.setCursor(1, 10);
    tft.print(WiFi.SSID());
    tft.setCursor(1, 32);
    tft.print("RSSI= ");tft.print(WiFi.RSSI()); tft.print(" dBm");
    tft.setCursor(1, 42);
    tft.print(WiFi.localIP()); 
    tft.setCursor(1, 62);
    tft.print("brightness="); tft.print(brightness/10.24);tft.print("%");
    tft.setCursor(1, 72);
    tft.print("mqtt server="); tft.print(mqtt_server);
    tft.print("pushetta min max"); tft.print(minPushTemp);tft.print(" "); tft.print(maxPushTemp);
	 delay(15000);
     tft.fillScreen(ST7735_BLACK);

delay(1000);
 if (!EnableWifi) {
WiFi.mode(WIFI_OFF);
 WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
 
 }  

}


void loop() {
 if (EnableWifi) {
  if (!client.connected()) {// connect to mqtt server
    reconnect();
  }
 }
  client.loop();//mqtt check subcriptions
  
  analogWrite(brightnessPin, brightness);
  delay(1);//tim
  
    
    
    while(millis() - previousMillis< 1000) {//Start of main loop (restricted the loop to 1Hz)  was 999
  //Serial.println(millis()-previousMillis);
  delay(50);
  //currentMillis = millis();
}
   //Serial.print(millis()); Serial.print(" previous=");Serial.println(previousMillis);
  previousMillis = millis();
   Serial.print("MainLoopCnt=");Serial.println(MainLoopCnt);


//Read Sensor
if ((MainLoopCnt % 20) == 1) {   ///every 20 times thru loop.  (Loop is every second)
    delay(1);// allow the esp8266 to catch up on house keeping
    //sensors.setResolution(12);
    delay(1);// allow the esp8266 to catch up on house keeping
    sensors.requestTemperatures(); // Send the command to get temperatures
    delay(2);// allow the esp8266 to catch up on house keeping
    temperature = sensors.getTempCByIndex(0) + temperatureOffSet;    
  }


//update temperature on display
  if ((MainLoopCnt % 20) == 1) {   ///every 20 times thru loop.  (Loop is every second)
    drawGrid ();//refresh grid
    tft.setTextColor(ST7735_RED, ST7735_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5, 120);
    tft.print(temperature, 3);
    delay(1);
    tft.setTextSize(1);
    tft.print("C");
    delay(1);

  }


  //draw temperature graph
  if ((MainLoopCnt % 20) == 1) {   ///every 20 times thru loop.  (Loop is every  second) was 1440
    // therefore once every 30 s
    
    updateGraph();
  }

if (EnableWifi) {
 //send MQTT
  if ((MainLoopCnt % 20) == 1) {   ///every 20 times thru loop.  (Loop is every  second) was 1440
    //Serial.println("about to publish");
    delay(1);
    
    if (mqttlastTemperature != temperature){
      client.publish(mqtt_channel1, String(temperature).c_str(), TRUE);//mqtt
       mqttNoChange =0; //reset counter
       Serial.println("temp changed");
       }
      else {
        mqttNoChange++;
        Serial.println("temp not changed");
         Serial.print("mqttnochange "); Serial.println(mqttNoChange);
      if (mqttNoChange>mqttWait) {//time to send a message (even though nothing has changed)
        client.publish(mqtt_channel1, String(temperature).c_str(), TRUE);//mqtt
        mqttNoChange =0; //reset counter
        Serial.println(mqttlastTemperature);
        Serial.println(temperature);
        
        }
     }
        
    mqttlastTemperature=temperature;
  }


 
  // send pushetta message
  if ((temperature > maxPushTemp) or (temperature < minPushTemp)){
   if (millis()-lastPush > 120000) {// minimum interval is 2 minutes
  sendToPushetta(PushChannel, String(temperature).c_str());
  Serial.print("Pushetta");Serial.println(temperature);
  lastPush =millis();
  delay(1);
   }
  }
}
  MainLoopCnt++;
  delay(50);
  //ESP.deepSleep(60000000, WAKE_RF_DEFAULT); microseconds, max 2^32, GPIO16 needs to be tied to RST to wake from deepSleep.
}
//}


void updateGraph (void) {
	
	    for (int j = 0; j < dispx; j++) { //erase old line
      ScaledTemperature1 = (((tempHistory[j]) - ScaleMin) * dispy) / (ScaleMax - ScaleMin); //essentially the number of pixels
      ScaledTemperature2 = (((tempHistory[j + 1]) - ScaleMin) * dispy) / (ScaleMax - ScaleMin);
      if ((dispy - ScaledTemperature1) <= (1000 * dispy)) { //dispy
        tft.drawLine(j, dispy - ScaledTemperature1, j + 1, dispy - ScaledTemperature2, ST7735_BLACK);
        //Serial.println(dispy-ScaledTemperature);
      }
    }
    for (int j = 1; j <= 127; j++) { //Pushing the newest value to the end of the array
      tempHistory[j - 1] = tempHistory[j];
      if (tempHistory[j - 1] > maxtemperature) {
        maxtemperature = tempHistory[j - 1];
      }// finding max temp
    }
    tempHistory[dispx-1] = temperature;
    maxtemperature = 0;
    for (int j = 0; j <= 127; j++) { //find max temp in array
      if (tempHistory[j - 1] > maxtemperature) {
        maxtemperature = tempHistory[j - 1];
      }// finding max temp
    }
    //  for (int j = 0; j < 128; j++){ //debug
    //  Serial.println(tempHistory[j],4);
    //  }
    //  for (int j = 1; j < 128; j++){ //draw new line
    //  tft.drawPixel(j, 160 - (3 * int(tempHistory[j])), ST7735_RED);
    //  }

    // calculate axis range
    if (temperature >= (maxtemperature - graphHeight)) { //can show both peak and current values
      ScaleMin = maxtemperature - graphHeight;
      ScaleMax = maxtemperature;
      /*Serial.println("if");
      Serial.println(temperature, 4);
      Serial.println(maxtemperature, 4);
      Serial.println(graphHeight);
      Serial.println(ScaleMax, 4);
      Serial.println(ScaleMin, 4);
      Serial.println();*/
    }
    else {
      ScaleMin = temperature - graphHeight + graphHeight / 2; // centre current temp on T axis
      ScaleMax = temperature + graphHeight / 2;

      /*Serial.println("else");
      Serial.println(temperature);
      Serial.println(maxtemperature);
      Serial.println(graphHeight);
      Serial.println(ScaleMax);
      Serial.println(ScaleMin);
      Serial.println();*/
    }
    //Serial.println(temperature);
    if (MainLoopCnt > 1) {
      drawTemperatureLine();
    }
}


void drawTemperatureLine(void) {
 // char buffer[16];

  if (fullPageGraph) {
    for (int j = 0; j < dispx; j++) { //draw graph
      ScaledTemperature1 = (((tempHistory[j]) - ScaleMin) * dispy) / (ScaleMax - ScaleMin);
      //ScaledTemperature1=ScaledTemperature2;//essentially the number of pixels
      ScaledTemperature2 = (((tempHistory[j + 1]) - ScaleMin) * dispy) / (ScaleMax - ScaleMin);
      //  if ((dispy - ScaledTemperature1) <=dispy){
      if (abs(ScaledTemperature2  - ScaledTemperature1) <= dispy) {

        tft.drawLine(j, dispy - ScaledTemperature1, j + 1, dispy - ScaledTemperature2, ST7735_WHITE);
        
        //delay(2); Serial.print(j); Serial.print(" ");
        //Serial.print(tempHistory[j], 2); Serial.print(" ");
        //Serial.print(ScaledTemperature1, DEC); Serial.print(" ");
        //Serial.print(ScaledTemperature2, DEC); Serial.print(" ");
        //Serial.print(abs(ScaledTemperature2  - ScaledTemperature1), DEC); Serial.print(" ");
        //Serial.println();
        delay(2);
        //Serial.println(dispy-ScaledTemperature);
      }
      
    }
  }


}


void drawGrid () {
  for (int j = 1; j < 128;) {
    j = j + 19;
    tft.drawPixel(j, 20, ST7735_GREY);
    tft.drawPixel(j, 40, ST7735_GREY);
    tft.drawPixel(j, 60, ST7735_GREY);
    tft.drawPixel(j, 80, ST7735_GREY);
    tft.drawPixel(j, 100, ST7735_GREY);
    tft.drawPixel(j, 120, ST7735_GREY);
    tft.drawPixel(j, 140, ST7735_GREY);

    j = j + 1;
    tft.drawPixel(j, 20, ST7735_GREY);
    tft.drawPixel(j, 40, ST7735_GREY);
    tft.drawPixel(j, 60, ST7735_GREY);
    tft.drawPixel(j, 80, ST7735_GREY);
    tft.drawPixel(j, 100, ST7735_GREY);
    tft.drawPixel(j, 120, ST7735_GREY);
    tft.drawPixel(j, 140, ST7735_GREY);
  }
}

void testmultifillcircles(uint8_t radius) {
  uint8_t x = tft.width() / 2;
  uint8_t y = tft.height() / 2;
  for (uint8_t i = radius; i > 1; i -= 1) {
    tft.fillCircle(x, y, i, tft.Color565 (i, 255 - (i * 2), 0));
  }
}

void setup_wifi() {
  int attempts =1;
  WiFi.mode(WIFI_STA);
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while ((WiFi.status() != WL_CONNECTED) and (attempts <=2))
  {
    delay(500);
    attempts++;
    Serial.print(".");
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {
	   Serial.println("WiFi connected");}
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  int attempts =1;
  // Loop until we're reconnected
  while (!client.connected()and (attempts <=2)) {
    Serial.print("Attempting MQTT connection...");
    attempts++;
    // Attempt to connect
    if (client.connect("arduinoClient_temperature_sensor", mqtt_channel1, 1, 1, "Disconnected")) { //connect (clientID, willTopic, willQoS, willRetain, willMessage)
      Serial.println("connected");
      client.subscribe("inTopic");
      client.subscribe("brightness");
      delay(10);
      client.subscribe("maxpushtemp");
      delay(10);
      client.subscribe("minpushtemp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {//mqtt check subs
  
  //String brightString;
  //int brightInt;
  Serial.print("Message arrived ");
  Serial.print(topic); Serial.print(" ");
  delay(1);
    //Serial.print("] ");
  for (unsigned int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.print("length=");Serial.println(length);
  Serial.println();

   if (String(topic)=="brightness") {
 /* String value = String((char*)payload);
  brightString = value.charAt(0); 
  brightInt = brightString.toInt();
  *///Serial.print(" brightInt=");Serial.print(brightInt);
  
    //convert payload to int
  payload[length] = '\0';   //add a line return so atof can parse correctly
  float mymsg = atof( (const char *) payload);
  
  //if ((length==1) and (brightInt>=0)) {
  //brightness=brightInt*100;}
  
    if ((mymsg<=9) and (mymsg>=0)) {
  brightness=mymsg*113;}
  //brightness=brightString.toInt()*100;
  // for (int i=0;i<length;i++) {
      //Serial.print((char)payload[i]);
       //}
      // brightnesstest= int(payload[0]);
      //brightness=brightnesstest;
       //Serial.println();Serial.print("brightness= ");Serial.print(brightness);
       delay(1);
       //Serial.println();
       //Serial.println(value);
       //Serial.println(length);
  }
  
   if (String(topic)=="minpushtemp") {
	    payload[length] = '\0';
  String s = String((char*)payload);
  minPushTemp = s.toFloat();
  Serial.print("minpushtemp=");Serial.println(minPushTemp);
  }
  
  if (String(topic)=="maxpushtemp") {
	    payload[length] = '\0';
  String s = String((char*)payload);
  maxPushTemp = s.toFloat();
  Serial.print("maxpushtemp=");Serial.println(maxPushTemp);
  }
  
}


//Function for sending the request to Pushetta
void sendToPushetta(char channel[], String text) {
  WiFiClient client;
  client.stop();

  if (client.connect(PushserverName, 80))
  {
    client.print("POST /api/pushes/");
    client.print(channel);
    client.println("/ HTTP/1.1");
    client.print("Host: ");
    client.println(PushserverName);
    client.print("Authorization: Token ");
    client.println(PushApiKey);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(text.length() + 46);
    client.println();
    client.print("{ \"body\" : \"");
    client.print(text);
    client.println("\", \"message_type\" : \"text/plain\" }");
    client.println();
    delay(2);
  }
}

//void send2thingspeak(){}