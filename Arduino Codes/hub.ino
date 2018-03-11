#include <DHT.h>

#define GOOD 5
#define MED 4
#define BAD 3
#define PI_RELAY 10
#define RECHARGE_RELAY 9
#define FAN_RELAY 8

#define CHARGE_ON
#define CHARGE_OFF

#define TEMPERATURE_PIN 2
#define BATTERY_PIN A0

DHT dht;

float lastTemp;
float lastBattery;
float temperature;
int sensorValue=0;

unsigned long previousMillis_temperature = 0;
const long temperature_interval = 120000;
unsigned long previousMillis_battery = 0;
const long battery_interval = 13000;

void temperature_check();
void battery_check();
void rgb_led();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  dht.setup(TEMPERATURE_PIN);
  delay(200);
  
  pinMode(GOOD,OUTPUT);
  pinMode(MED,OUTPUT);
  pinMode(BAD,OUTPUT);
  pinMode(PI_RELAY,OUTPUT);
  pinMode(RECHARGE_RELAY,OUTPUT);
  pinMode(FAN_RELAY,OUTPUT);
  delay(200);
  
  digitalWrite(GOOD,HIGH);
  digitalWrite(MED,HIGH);
  digitalWrite(BAD,HIGH);
  digitalWrite(PI_RELAY,LOW);
  digitalWrite(RECHARGE_RELAY,HIGH);
  digitalWrite(FAN_RELAY,HIGH);
  delay(200);
  
  #if defined(__AVR_ATmega2560__)
   analogReference(INTERNAL1V1);
#else
   analogReference(INTERNAL);
#endif
  delay(200);
  
  battery_check();
  temperature_check();
  delay(200);
}

void loop() 
{
  unsigned long currentMillis_temperature = millis();
  if (currentMillis_temperature - previousMillis_temperature >= temperature_interval) 
  {
    previousMillis_temperature = currentMillis_temperature;
    temperature_check();
    battery_check();
  }

  unsigned long currentMillis_battery = millis();
  if(digitalRead(RECHARGE_RELAY)==LOW)
  if (currentMillis_battery - previousMillis_battery >= battery_interval) 
  {
     previousMillis_battery = currentMillis_battery;
     for(int i=3;i<6;i++)
     {
        digitalWrite(i,HIGH);
     }   
     for(int i=3;i<6;i++)
     {
        digitalWrite(i,LOW);
        delay(500);
        digitalWrite(i,HIGH);
     }
  }   
}

void temperature_check()
{
  delay(dht.getMinimumSamplingPeriod());
  temperature = dht.getTemperature();
  if (isnan(temperature)) 
  {
      Serial.println("Failed reading temperature from DHT");
  } 
  else if (temperature != lastTemp) 
  {
    lastTemp = temperature;
    Serial.print("T: ");
    Serial.println(temperature);

    if(temperature>35)
    {
      digitalWrite(FAN_RELAY,LOW);
    }
    if(temperature<=26)
    {
      digitalWrite(FAN_RELAY,HIGH);
    }
  }
}

void battery_check()
{
   sensorValue = analogRead(BATTERY_PIN);
   float batteryV  = sensorValue * 0.004134897361;
   int batteryPcent = map(sensorValue,700,1000,0,100);
   if(batteryPcent!=lastBattery)
   {
      lastBattery=batteryPcent;
      Serial.println(batteryV);
      Serial.println(sensorValue);
      Serial.println(batteryPcent);
      if(batteryPcent<25)
      {
          digitalWrite(GOOD,HIGH);
          digitalWrite(MED,HIGH);
          digitalWrite(BAD,LOW);
      }
      else if((batteryPcent>25)&(batteryPcent<75))
      {
          digitalWrite(GOOD,HIGH);
          digitalWrite(BAD,HIGH);
          digitalWrite(MED,LOW);
      }
      else
      { 
          digitalWrite(MED,HIGH);
          digitalWrite(BAD,HIGH);
          digitalWrite(GOOD,LOW);
      }

      if(batteryPcent<20)
      {
          digitalWrite(RECHARGE_RELAY,LOW);
      }
      if(batteryPcent==107)
      {
          digitalWrite(RECHARGE_RELAY,HIGH);
      }
   }
}

