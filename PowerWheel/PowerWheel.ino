#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <TaskScheduler.h>
#include <math.h>

//Wheel Encoder Settings
#define WHEEL_TEETH (12)
#define WHEEL_DIAMETER_CM  (66.0)
#define RPM2MPH (3.728226e-4) //6.21371e-6 * 60 (cm to mile to hour) 

//Board IO Settings
#define BATTERY_IN (A0)
#define LEFT_WHEEL_IN (D2)
#define RIGHT_WHEEL_IN (D1)
#define MOTOR_FWD_OUT (D5)
#define PEDAL_FWD_IN (D6)
#define PEDAL_REV_IN (D7)
#define MOTOR_REV_OUT (D8)

//Duty Cycle Ramp up and down
#define DC_STEP_DOWN (1.0);
#define DC_STEP_SLOW (5.0);
#define DC_STEP_FAST (15.0);

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
typedef enum eGear {GNEUTRAL, GFORWARD, GREVERSE, GPROTECT, GERROR}eGear;
eGear Gear = GNEUTRAL;

//Wheel metrics
typedef enum eWheels {W_LEFT, W_RIGHT, W_COUNT}eWheels;
unsigned long tickCount[W_COUNT] = {0};
unsigned long lastTickCount[W_COUNT] = {0};
int wheelSensor[W_COUNT] = {0,0};

//Speed Control
double MaxMph = 3.0;
double MaxSlip = 100.0;
volatile unsigned long previousTime[W_COUNT] = {0};
volatile unsigned long currentTime[W_COUNT] = {0};
volatile double RPM[W_COUNT] = {0};
volatile double MPH[W_COUNT] = {0};

//Traction Control
bool TCS = true;
bool TCS_Comfort = true;


//Battery
double BatteryVoltage;
int BatteryValue;

//Task Scheduler
void task1HzCallback();
void task5HzCallback();
void task1000HzCallback();
Task task1Hz(1000, TASK_FOREVER, &task1HzCallback);
Task task5Hz(200, TASK_FOREVER, &task5HzCallback);
Task task1000Hz(1, TASK_FOREVER, &task1000HzCallback);
Scheduler task;



//Calc info from sensor
void WheelSensor(int W)
{
  currentTime[W] = millis();
  tickCount[W]++;

  if (currentTime[W] > previousTime[W])
  {
    //x2 because this is called on falling and rising edge
    RPM[W] = 60000.0 / ((currentTime[W] - previousTime[W]) * WHEEL_TEETH * 2.0);
    MPH[W] = RPM[W] * RPM2MPH * WHEEL_DIAMETER_CM;
  }

  previousTime[W] = currentTime[W];
}


void task1HzCallback()
{
  BatteryValue = analogRead(BATTERY_IN);
  BatteryVoltage = (12.0 / 1023.0) * (double)BatteryValue * 3.3;

  for(int w = W_LEFT; w < W_COUNT; w++)
  {
    //reset if wheel is not moving, otherwise the last read RPM will stay forever
    if(lastTickCount[w] == tickCount[w])
    {
      RPM[w] = 0.0;
      MPH[w] = 0.0;
    }
    lastTickCount[w] = tickCount[w];
  }
}

void gearSelection()
{
    int pedalFwd = digitalRead(PEDAL_FWD_IN);
    int pedalRev = digitalRead(PEDAL_REV_IN);
    
    //pedal up
    if(pedalFwd == LOW && pedalRev == LOW && Gear != GPROTECT)
    {
        Gear = GNEUTRAL;
    }

    //Stay in protect until speed goes to 0ish
    else if(Gear == GPROTECT)
    {
      if(RPM[W_LEFT] < 1.0)
        Gear = GNEUTRAL;
    }
    
    //Error, both should never be high, kill power
    else if(pedalFwd == HIGH && pedalRev == HIGH)
    {
      Gear = GERROR;
    }

    //Forward Gear
    else if(pedalFwd == HIGH)
    {
      //Protect hard shift into forward from reverse
      if(Gear == GREVERSE)
      {
       Gear = GPROTECT;
      }
      else
      {
        Gear = GFORWARD;
      }
    }

    //Reverse Gear
    else if(pedalRev == HIGH)
    {
      //Protect hard shift into reverse from forward
      if(Gear == GFORWARD)
      {
        Gear = GPROTECT;
      }
      else
      {
        Gear = GREVERSE;
      }
    }
}


void task5HzCallback()
{
    double change = DC_STEP_SLOW;
 
    if(TCS)
    {
      double slip = RPM[W_LEFT] - RPM[W_RIGHT];
 
      //bound slip
      slip = _max(0, slip);
      slip = _min(MaxSlip, slip);

      if(TCS_Comfort)
      {
        change = DC_STEP_SLOW;
      }
      
      else //Sport
      {
        change = DC_STEP_FAST;
      }
      
      //Percentage of power based on slip
       change *= (-2.0 * slip + 100.0) / 100.0;
    }
 
   if(MPH[W_RIGHT] > MaxMph)
   {
    change = DC_STEP_DOWN;
   }
  
    //Change is based on how close we are to target speed
    DutyCycle += change * (1.0 - (MPH[W_RIGHT] / MaxMph));
   

  DutyCycle = _max(DutyCycle, MinDutyCycle);
  DutyCycle = _min(DutyCycle, MaxDutyCycle);

  gearSelection();
  
  if(Gear == GFORWARD)
  {
    analogWrite(MOTOR_REV_OUT,0);
    analogWrite(MOTOR_FWD_OUT, map(DutyCycle, 0, 100, 0, 1023));
  }
  else if(Gear == GREVERSE)
  {
    analogWrite(MOTOR_FWD_OUT,0);
    analogWrite(MOTOR_REV_OUT, map(DutyCycle, 0, 100, 0, 1023));
  }
  else
  {
    DutyCycle = MinDutyCycle;
    analogWrite(MOTOR_REV_OUT,0);
    analogWrite(MOTOR_FWD_OUT,0);
  }
}

void task1000HzCallback()
{
    int wheelLeft = digitalRead(LEFT_WHEEL_IN);
    int wheelRight = digitalRead(RIGHT_WHEEL_IN);

    if(wheelSensor[W_LEFT] != wheelLeft)
    {
      WheelSensor(W_LEFT);
    }

    if(wheelSensor[W_RIGHT] != wheelRight)
    {
      WheelSensor(W_RIGHT);
    }

    wheelSensor[W_LEFT] = wheelLeft;
    wheelSensor[W_RIGHT] = wheelRight;
}

void handleSettings() 
{
  buildHomePage();

  server.send(200, "text/html", webSite);
}

void handleGauge()
{
  buildGaugePage();

  if (server.hasArg("SPD"))
  {
    MaxMph = (double)server.arg("SPD").toInt();
  }

  if (server.hasArg("TCS"))
  {
    int tcs = server.arg("TCS").toInt();
    TCS = tcs > 0;
    TCS_Comfort = tcs == 2;
  }

  if (server.hasArg("MAXDC"))
  {
    MaxDutyCycle = server.arg("MAXDC").toInt();
  }

  if (server.hasArg("MINDC"))
  {
    MinDutyCycle = server.arg("MINDC").toInt();
  }
  
  server.send(200, "text/html", webSite);
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
  
 // attachInterrupt(digitalPinToInterrupt(LEFT_WHEEL_IN), LeftWheelSensor, FALLING); 
 // attachInterrupt(digitalPinToInterrupt(RIGHT_WHEEL_IN), RightWheelSensor, FALLING);


  //Init times
  currentTime[0] = currentTime[1] = millis();
  previousTime[0] = previousTime[1] = millis();

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

  task.addTask(task1000Hz);
  task1000Hz.enable();

 
  //Set web sites
  server.on("/", handleGauge);
  server.on("/lamboData", handleLamboData);
  server.on("/settings", handleSettings);
  
  server.begin();
}


void loop() 
{
  server.handleClient();
  task.execute();
}
