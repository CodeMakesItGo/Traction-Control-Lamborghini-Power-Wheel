#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <TaskScheduler.h>
#include <math.h>

//Board IO Settings
#define BATTERY_IN (A0)
#define LEFT_WHEEL_IN (D2)
#define RIGHT_WHEEL_IN (D1)
#define MOTOR_FWD_OUT (D5)
#define PEDAL_FWD_IN (D6)
#define PEDAL_REV_IN (D7)
#define MOTOR_REV_OUT (D8)

//Duty Cycle Stepper
#define DC_STEP (10.0)
#define GEAR_COUNT 10

/* Set these to your desired credentials. */
const char *ssid = "Lambo";
const char *password = "CodeMakesItGo";

//Wifi
ESP8266WebServer server(80);
IPAddress ip;

//Web and AJAX
String webSite, XML;

//Motor Control
int DutyCycle = 0;
int MinDutyCycle = 30;
int MaxDutyCycle = 100;

//Gear Selection
typedef enum {GNEUTRAL, GFORWARD, GREVERSE} eGear;
eGear GearDebounce[GEAR_COUNT] = {GNEUTRAL}; //1 second of neutral
eGear Gear = GNEUTRAL;
int GearIndex = 0;

//Battery
double BatteryVoltage;
int BatteryValue;

//Task Scheduler
void task1HzCallback();
void task5HzCallback();

Task task1Hz(1000, TASK_FOREVER, &task1HzCallback);
Task task5Hz(200, TASK_FOREVER, &task5HzCallback);
Scheduler task;


void task1HzCallback()
{
  BatteryValue = analogRead(BATTERY_IN);
  BatteryVoltage = (12.0 / 1023.0) * (double)BatteryValue * 3.3;
}

void gearSelection()
{
    int pedalFwd = digitalRead(PEDAL_FWD_IN);
    int pedalRev = digitalRead(PEDAL_REV_IN);
     
    //pedal up
    if(pedalFwd == LOW && pedalRev == LOW)
    {
      GearDebounce[GearIndex] = GNEUTRAL;
    }

    //Error, both should never be high, kill power
    else if(pedalFwd == HIGH && pedalRev == HIGH)
    {
      GearDebounce[GearIndex] = GNEUTRAL;
    }

    //Forward Gear
    else if(pedalFwd == HIGH)
    {
      GearDebounce[GearIndex] = GFORWARD;
    }

    //Reverse Gear
    else if(pedalRev == HIGH)
    {
      GearDebounce[GearIndex] = GREVERSE;
    }

    GearIndex++;
    GearIndex = GearIndex % GEAR_COUNT;

    //If gear does not match for a half second then set to neutral
    for(int i = 0; i < GEAR_COUNT; ++i)
    {
      if(GearDebounce[0] != GearDebounce[i])
      {
         Gear = GNEUTRAL;
         return;
      }
    }

    Gear = GearDebounce[0];
}


void task5HzCallback()
{
  
  gearSelection();
  
  if(Gear == GFORWARD)
  {
    DutyCycle += DC_STEP;
   
    DutyCycle = _max(DutyCycle, MinDutyCycle);
    DutyCycle = _min(DutyCycle, MaxDutyCycle);
  
    analogWrite(MOTOR_REV_OUT,0);
    analogWrite(MOTOR_FWD_OUT, map(DutyCycle, 0, 100, 0, 1023));
  }
  else if(Gear == GREVERSE)
  {
    DutyCycle += DC_STEP;
   
    DutyCycle = _max(DutyCycle, MinDutyCycle);
    DutyCycle = _min(DutyCycle, MaxDutyCycle);
    
    analogWrite(MOTOR_FWD_OUT,0);
    analogWrite(MOTOR_REV_OUT, map(DutyCycle, 0, 100, 0, 1023));
  }
  else
  {
    DutyCycle -= DC_STEP;
   
    DutyCycle = _max(DutyCycle, MinDutyCycle);
    DutyCycle = _min(DutyCycle, MaxDutyCycle);
    
    analogWrite(MOTOR_REV_OUT,0);
    analogWrite(MOTOR_FWD_OUT,0);
  }
}


void setup() 
{
  pinMode(MOTOR_FWD_OUT, OUTPUT);
  pinMode(MOTOR_REV_OUT, OUTPUT);
  
  analogWrite(MOTOR_FWD_OUT,0);
  analogWrite(MOTOR_REV_OUT,0);
  
  pinMode(LEFT_WHEEL_IN, INPUT_PULLUP);
  pinMode(RIGHT_WHEEL_IN, INPUT_PULLUP);
  pinMode(PEDAL_REV_IN, INPUT_PULLUP);
  pinMode(PEDAL_FWD_IN, INPUT_PULLUP);
  

  //Start Wireless
  WiFi.softAP(ssid, password);
  ip = WiFi.softAPIP();
  
  //enable lambo.local web address
  MDNS.begin("lambo");
  
  //start scheduled tasks
  task.init();
   
  task.addTask(task1Hz);
  task1Hz.enable();
  
  task.addTask(task5Hz);
  task5Hz.enable();
 
  //Set web sites
  server.on("/Data", handleLamboData);
  server.on("/", handleSettings);
  
  server.begin();
}


void loop() 
{
  server.handleClient();
  task.execute();
}
