/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "e:/aacrs/02_12/markvisser/git_clone/Timers/timers/src/timers.ino"

/*
 * TimeAlarmExample.pde
 *
 * This example calls alarm functions at 8:30 am and at 5:45 pm (17:45)
 * and simulates turning lights on at night and off in the morning
 * A weekly timer is set for Saturdays at 8:30:30
 *
 * A timer is called every 15 seconds
 * Another timer is called once only after 10 seconds
 *
 * At startup the time is set to Jan 1 2011  8:29 am
 */
 //Argon Timers demo	e00fce6867ccb25034dbe6c5	
 //30:ae:a4:d1:c1:f8



#include "Particle.h"
#include "TimeAlarms.h"
#include <blynk.h>
#include "Ubidots.h"
#include <string.h>
void Name_of_Device(const char *topic, const char *data);
inline void softDelay(uint32_t msDelay);
void setup();
void shutterOpenAlarm();
void shutterCloseAlarm();
void PostToUbidot(void);
float setControl(float value);
void myHandler(const char *event, const char *data);
void loop();
void checkCurrentShutterStatus();
void digitalClockDisplay();
void printDigits(int digits);
void blinkGreenRGB(void);
void blinkRedRGB(void);
void blinkBlueRGB(void);
void blinkYellowRGB(void);
#line 24 "e:/aacrs/02_12/markvisser/git_clone/Timers/timers/src/timers.ino"
#define UBIDOTS_TOKEN "BBFF-eZNmK5AdVusSXvaVxKYlxkt1N1Oedf"
#define DATA_SOURCE_TAG_DEVICE_API VARIABLE_LABEL// API of the gateway device
//variables supplied by Ubidots via get function
#define DATA_SOURCE_TAG_SHUTTER_HOUR_CLOSE "shutterHourClose"
#define DATA_SOURCE_TAG_SHUTTER_MINUTE_CLOSE "shutterMinuteClose"
#define DATA_SOURCE_TAG_SHUTTER_OVERIDE_CLOSE "shutterOverideClose"

#define DATA_SOURCE_TAG_SHUTTER_HOUR_OPEN "shutterHourOpen"
#define DATA_SOURCE_TAG_SHUTTER_MINUTE_OPEN "shutterMinuteOpen"
#define DATA_SOURCE_TAG_SHUTTER_OVERIDE_OPEN "shutterOverideOpen"

const int RELAY1 = D4;//detonation relay
const int RELAY2 = D5;//Spare 
const int PCH1= A1;//detonation relay
const int NCH1 = A0;//NPN outout drive
const int LOAD = D3;//PNO output drive
const int CLEAR = D2;//Clear Becons
const int led_green = D8;
const int led_red = D6;
const int led_blue= D7;
int buttonLOAD;//hold button load value
int buttonCLEAR;//hold button clear value

float shuttterOpencontrol = 0;
float shuttterClosecontrol = 0;
float valueShutterHourOpen = 0;
float valueShutterHourClose = 0;
float valueShutterMinuteOpen = 0;
float valueShutterMinuteClose = 0;
float shuttercontrol = 0;//control overide of the shutter
char  shutterIndicator[17] = "shutterindicator";//variable name to show shutter open or close in ubidots
int i = 0;
bool shutterStatus = 0;//flag to check shutter status on start up


/********************************************************************************************/
//Ubidots



//const char* WEBHOOK_NAME = "timerwebhook";
//Ubidots ubidots(WEBHOOK_NAME, UBI_PARTICLE);
//Ubidots ubidots(UBIDOTS_TOKEN, UBI_TCP); // Comment this line to use another protocol.
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP); // Uncomment this line to use HTTP protocol.
//Ubidots ubidots(UBIDOTS_TOKEN, UBI_UDP); // Uncomment this line to use UDP protocol

char VARIABLE_LABEL[32] = "";//keeps the device name
char dev_name[32] = "";//keeps the device name
bool publishName = false;//to publich the name



/********************************************************************************************/
// used for getting the device name
void Name_of_Device(const char *topic, const char *data){
 strncpy(dev_name, data, sizeof(dev_name)-1);
  Serial.printlnf("received in supscribe/publish request%s: %s", topic, dev_name);
  publishName = true;
  strncpy(VARIABLE_LABEL, dev_name, sizeof(dev_name));
  Serial.printlnf("variable label  that is copied from dev_name : %s", VARIABLE_LABEL);
 }

 inline void softDelay(uint32_t msDelay){
      for (uint32_t ms = millis(); 
            millis() - ms < msDelay;  
           Particle.process());
}
void setup() {

  Serial.begin(115200);
  Particle.subscribe("particle/device/name", Name_of_Device);
  softDelay(3000);
  Particle.publish("particle/device/name");//ask the cloud for the name to be sent to you
  softDelay(3000);
  Time.zone(+2);
  pinMode(RELAY1, OUTPUT);//output for triggered led
  pinMode(RELAY2, OUTPUT);//output for triggered led
  pinMode(led_red, OUTPUT);//output for triggered led
  pinMode(led_green, OUTPUT);//output for triggered led
  pinMode(led_blue, OUTPUT);//output for triggered led
  pinMode(PCH1, OUTPUT);//output for triggered led
  pinMode(NCH1, OUTPUT);//output for triggered led
  pinMode(LOAD, INPUT);//output for triggered led
  pinMode(CLEAR, INPUT);//output for triggered led
  pinMode(BATT, INPUT); // argon battery 

  // set initial IO states
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (RELAY1, LOW);//relay off
  digitalWrite (RELAY2, LOW);//relay off
  digitalWrite (LOAD, LOW);//off
  digitalWrite (CLEAR, LOW);//off
  pinMode(BATT, INPUT);
  // Subscribe to the integration response event
  //Particle.subscribe("UbidotsWebhook", shutterOpenHandler);
  //Particle.subscribe("UbidotsWebhook", shutterCloseHandler);
  valueShutterHourOpen = ubidots.get(DATA_SOURCE_TAG_DEVICE_API,DATA_SOURCE_TAG_SHUTTER_HOUR_OPEN);//device api label,variable api label
  valueShutterMinuteOpen = ubidots.get(DATA_SOURCE_TAG_DEVICE_API,DATA_SOURCE_TAG_SHUTTER_MINUTE_OPEN);//device api label,variable api label
  valueShutterHourClose = ubidots.get(DATA_SOURCE_TAG_DEVICE_API,DATA_SOURCE_TAG_SHUTTER_HOUR_CLOSE);//device api label,variable api label
  valueShutterMinuteClose = ubidots.get(DATA_SOURCE_TAG_DEVICE_API,DATA_SOURCE_TAG_SHUTTER_MINUTE_CLOSE);//device api label,variable api label
  Serial.printf("shutter OPEN %.f:%.f CLOSE %.f:%.f\n",valueShutterHourOpen,valueShutterMinuteOpen,valueShutterHourClose,valueShutterMinuteClose);

  // create the alarms 
  Alarm.alarmRepeat(valueShutterHourOpen,valueShutterMinuteOpen,0, shutterOpenAlarm);  // 8:30am every day
  Alarm.alarmRepeat(valueShutterHourClose,valueShutterMinuteClose,0,shutterCloseAlarm);  // 5:45pm every day
  ubidots.setDebug(true);//Uncomment this line for printing debug messages

 //Particle.subscribe("timerwebhook", handlerShutterHourOpen);
 //Particle.subscribe("timerwebhook", handlerShutterHourClose);
 //Particle.subscribe("timerwebhook", handlerShutterMinuteOpen);
 //Particle.subscribe("timerwebhook", handlerShutterMinuteClose);
 Particle.subscribe("timerwebhook", myHandler);
}


// functions to be called when an alarm triggers:
void shutterOpenAlarm(){
  Serial.println("Alarm: - OPEN SHUTTER");   
  digitalWrite (led_green, LOW);//led on
  digitalWrite (led_red, HIGH);//led off
  ubidots.add(shutterIndicator, 1);//show ubidots shutter is open
  PostToUbidot(); // send to ubidot
}

void shutterCloseAlarm(){
  Serial.printf("Alarm: - CLOSE shutter\n");  
  digitalWrite (led_green, HIGH);//led off    
  digitalWrite (led_red, LOW);//led off    
  ubidots.add(shutterIndicator, 0);//show ubidots shutter is close
  PostToUbidot(); // send to ubidot
}

void PostToUbidot(void){

 char* context = (char *) malloc(sizeof(char) * 50);
 //Builds the context with the coordinates to send to Ubidots
 ubidots.getContext(context);
 bool bufferSent = false;
 bufferSent = ubidots.send(VARIABLE_LABEL, PRIVATE);//Will use particle webhooks to send data

 if(bufferSent){
   //Do something if values were sent properly
   Serial.println("Values sent by the device");
  }
 //frees memory
 free(context);
 delay(2000);
}

float setControl(float value) {
    if (value == 3) {
        return 3;
    }
    if (value == 1) {
        return 1;
    }
    return 2;
}

void myHandler(const char *event, const char *data) {
  // Handle the integration response
  i++;
  Serial.println(i);
  Serial.print(event);
  Serial.print(", data: ");
  if (data) {
   // Serial.println(data);
    shuttercontrol = setControl(atof(data));
    Serial.printf("%f\n",shuttercontrol);
  } else {
    Serial.println("NULL");
  }
}

void loop() {    
  digitalClockDisplay();
  if (shutterStatus == 0)  checkCurrentShutterStatus();
  shutterStatus = 1;
  Alarm.delay(1000);
  if (shuttercontrol == 3) {
      digitalWrite(NCH1, HIGH);//Open the shutter n channel
      digitalWrite(PCH1, LOW);//close the shutter
  }
  else if (shuttercontrol == 1) {
     digitalWrite(PCH1, HIGH);//close the shutter p channel
     digitalWrite(NCH1, LOW);
   }
  else if (shuttercontrol == 2) {
     
    digitalWrite(NCH1, LOW);//do Nothing
    digitalWrite(PCH1, LOW);//do nothing 
  } 
      Serial.printf("shutter contrll %f \n",shuttercontrol);
}



void checkCurrentShutterStatus(){
//&&(valueShutterMinuteOpen >= Time.minute() <=valueShutterMinuteClose )
//

if (((valueShutterHourOpen) < Time.hour()) && (Time.hour() < valueShutterHourClose ) ){
  digitalWrite (led_green, LOW);//led on
  digitalWrite (led_red, HIGH);//led off
  ubidots.add(shutterIndicator, 1);//show ubidots shutter is open
  PostToUbidot(); // send to ubidot
}

else {
  digitalWrite (led_green, HIGH);//led off    
  digitalWrite (led_red, LOW);//led on
  ubidots.add(shutterIndicator, 0);//show ubidots shutter is close
  PostToUbidot(); // send to ubidot
}


}
void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(Time.hour());
  printDigits(Time.minute());
  printDigits(Time.second());
  Serial.println(); 
}
void printDigits(int digits)
{
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
//blink green when a beacon is sucessfully enrolled
void blinkGreenRGB(void){
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  digitalWrite (led_green, LOW);//led on
  delay(80);
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  return;
 
}
void blinkRedRGB(void){
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  digitalWrite (led_red, LOW);//led on
  delay(50);
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  return;
}
void blinkBlueRGB(void){
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  digitalWrite (led_blue, LOW);//led on
  delay(50);
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  return;
}

void blinkYellowRGB(void){
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  digitalWrite (led_green, LOW);//led on
  digitalWrite (led_red, LOW);//led on
  delay(80);
  digitalWrite (led_red, HIGH);//led off
  digitalWrite (led_blue, HIGH);//led off
  digitalWrite (led_green, HIGH);//led off
  return;
}


/*
float setShutterOpenControl(float valueOpen) {
    if (valueOpen == 1) {
        return 1;
    }
    return 0;
}
float setShutterCloseControl(float valueClose) {
    if (valueClose == 2) {
        return 2;
    }
    return 0;
}

void shutterOpenHandler(const char *event, const char *data) {
  // Handle the integration response
  i++;
  Serial.println(i);
  Serial.print(event);
  Serial.print(", dataopen: ");
  if (data) {
    Serial.println(data);
    shuttterOpencontrol = setShutterOpenControl(atof(data));
    Serial.printf("shutteropencontrol %f\n",shuttterOpencontrol);
  } else {
    Serial.println("NULL");
  }
}


void shutterCloseHandler(const char *event, const char *data) {
  // Handle the integration response
 i++;
  Serial.println(i);
  Serial.print(event);
  Serial.print(", dataclose: ");
  if (data) {
    Serial.println(data);
    shuttterClosecontrol = setShutterCloseControl(atof(data));
     Serial.printf("shutterCLOSEcontrol %f\n",shuttterClosecontrol);
  } else {
    Serial.println("NULL");
  }
}
*/