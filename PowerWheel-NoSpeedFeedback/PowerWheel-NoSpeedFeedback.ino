#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <TaskScheduler.h>
#include <math.h>
#include <EEPROM.h>

//Board IO Settings
#define PEDAL_FWD_IN (D2)
#define PEDAL_REV_IN (D1)
#define MOTOR_FWD_OUT (D5)
#define MOTOR_FWD_LED_OUT (D6)
#define MOTOR_REV_LED_OUT (D7)
#define MOTOR_REV_OUT (D8)

//Duty Cycle Stepper
#define DC_STEP 10
#define EEPROM_KEY 33

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
uint8_t MinDutyCycle = 30;
uint8_t MaxDutyCycle = 70;

//Gear Selection
typedef enum {GNEUTRAL, GFORWARD, GREVERSE} eGear;
eGear Gear = GNEUTRAL;

//Task Scheduler
void task5HzCallback();

//eeprom token
uint8_t eepromKey = EEPROM_KEY;

Task task5Hz(200, TASK_FOREVER, &task5HzCallback);
Scheduler task;


void gearSelection()
{
    int pedalFwd = digitalRead(PEDAL_FWD_IN);
    int pedalRev = digitalRead(PEDAL_REV_IN);
     
    //pedal up
    if(pedalFwd == LOW && pedalRev == LOW)
    {
      Gear = GNEUTRAL;
    }

    //Error, both should never be high, kill power
    else if(pedalFwd == HIGH && pedalRev == HIGH)
    {
      Gear = GNEUTRAL;
    }

    //Forward Gear
    else if(pedalFwd == HIGH)
    {
      Gear = GFORWARD;
    }

    //Reverse Gear
    else if(pedalRev == HIGH)
    {
      Gear = GREVERSE;
    }
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

    digitalWrite(MOTOR_FWD_LED_OUT, HIGH);
    digitalWrite(MOTOR_REV_LED_OUT, LOW);
  }
  else if(Gear == GREVERSE)
  {
    DutyCycle += DC_STEP;
   
    DutyCycle = _max(DutyCycle, MinDutyCycle);
    DutyCycle = _min(DutyCycle, MaxDutyCycle);

    analogWrite(MOTOR_REV_OUT, map(DutyCycle, 0, 100, 0, 1023));
    analogWrite(MOTOR_FWD_OUT,0);

    digitalWrite(MOTOR_FWD_LED_OUT, LOW);
    digitalWrite(MOTOR_REV_LED_OUT, HIGH);
  }
  else
  {
    DutyCycle -= DC_STEP;
   
    DutyCycle = _max(DutyCycle, MinDutyCycle);
    DutyCycle = _min(DutyCycle, MaxDutyCycle);
    
    analogWrite(MOTOR_REV_OUT,0);
    analogWrite(MOTOR_FWD_OUT,0);

    digitalWrite(MOTOR_FWD_LED_OUT, LOW);
    digitalWrite(MOTOR_REV_LED_OUT, LOW);
  }
}


void setup() 
{
  pinMode(MOTOR_FWD_OUT, OUTPUT);
  pinMode(MOTOR_REV_OUT, OUTPUT);

  pinMode(MOTOR_FWD_LED_OUT, OUTPUT);
  pinMode(MOTOR_REV_LED_OUT, OUTPUT);
  
  analogWrite(MOTOR_FWD_OUT,0);
  analogWrite(MOTOR_REV_OUT,0);
  
  pinMode(PEDAL_REV_IN, INPUT_PULLUP);
  pinMode(PEDAL_FWD_IN, INPUT_PULLUP);

  digitalWrite(MOTOR_FWD_LED_OUT, HIGH);
  digitalWrite(MOTOR_REV_LED_OUT, HIGH);
  
  //get settings
  EEPROM.begin(32);
  eepromKey = EEPROM.read(0);
  
  if(eepromKey != EEPROM_KEY)
  {
    eepromKey = EEPROM_KEY;
    EEPROM.write(0, eepromKey);
    EEPROM.write(1, MinDutyCycle);
    EEPROM.write(2, MaxDutyCycle);
    EEPROM.commit();
  }
  
  MinDutyCycle = EEPROM.read(1);
  MaxDutyCycle = EEPROM.read(2);
  
  //Start Wireless
  WiFi.softAP(ssid, password);
  ip = WiFi.softAPIP();
  
  //enable lambo.local web address
  MDNS.begin("lambo");
  
  //start scheduled tasks
  task.init();
  
  task.addTask(task5Hz);
  task5Hz.enable();
 
  //Set web sites
  server.on("/Data", handleLamboData);
  server.on("/", handleSettings);
  
  server.begin();

  digitalWrite(MOTOR_FWD_LED_OUT, LOW);
  digitalWrite(MOTOR_REV_LED_OUT, LOW);
}


void loop() 
{
  server.handleClient();
  task.execute();
}
