#include <MySensor.h>
#include <SPI.h>
#include <Bounce2.h>

#define CHILD_ID_GAS 1
#define CHILD_ID_FLAME 2
#define CHILD_ID_BATTERY 3

#define GAS_PIN  2
#define FLAME_PIN  3
#define ALARM_PIN  4
#define reset_pin 5
#define INTERRUPT_GAS  2
#define INTERRUPT_FLAME  1

int BATTERY_SENSE_PIN = A0;
int oldBatteryPcnt=-1;

const long TIME = 60000;
unsigned long previousMillis = 0;
MySensor gw;

Bounce debouncer = Bounce();

int oldValue_gas=-1;
int oldValue_flame=-1;


MyMessage msgGas(CHILD_ID_GAS,V_TRIPPED);
MyMessage msgFlame(CHILD_ID_FLAME,V_ARMED);
MyMessage msg(CHILD_ID_BATTERY, V_LIGHT_LEVEL);

void gas_check();
void flame_check();
void alert_user();

void setup()  
{  
  gw.begin();

  gw.sendSketchInfo("Gas & Flame with battery level", "1.0");

  pinMode(GAS_PIN,INPUT);
  pinMode(FLAME_PIN,INPUT);
  pinMode(reset_pin,INPUT);
  pinMode(ALARM_PIN,OUTPUT);

  delay(300);
  
  digitalWrite(GAS_PIN,LOW);
  digitalWrite(FLAME_PIN,LOW);
  digitalWrite(reset_pin,HIGH);
  
  debouncer.attach(reset_pin);
  debouncer.interval(5);

  delay(300);
    
  gw.present(CHILD_ID_GAS, S_MOTION);
  gw.present(CHILD_ID_FLAME, S_SMOKE);
  gw.present(CHILD_ID_BATTERY, S_LIGHT_LEVEL);  

  #if defined(__AVR_ATmega2560__)
   analogReference(INTERNAL1V1);
#else
   analogReference(INTERNAL);
#endif

  delay(400);
}

void loop() 
{
  
  gas_check();
  
  flame_check();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= TIME) 
  {
    previousMillis = currentMillis;
    int sensorValue = analogRead(BATTERY_SENSE_PIN);
    //float batteryV  = sensorValue * 0.0043105446;
    int batteryPcnt = map(sensorValue,460,1023,0,100);
    if (oldBatteryPcnt != batteryPcnt) 
    {
      gw.send(msg.set(batteryPcnt));
      oldBatteryPcnt = batteryPcnt;
    }
  }
} 

void gas_check()
{
   int value_gas = digitalRead(GAS_PIN);
   delay(50);
   if (value_gas != oldValue_gas) 
   {
     Serial.println(value_gas);
     gw.send(msgGas.set(value_gas==HIGH ? 1 : 0));
     if (value_gas == HIGH) 
     {
        oldValue_gas = value_gas;
        alert_user();
     }
  }
  oldValue_gas = value_gas;
  delay(50);
}

void flame_check()
{
  int value_flame = digitalRead(FLAME_PIN);
  delay(50);
  if (value_flame != oldValue_flame) 
  {
     Serial.println(value_flame);
     gw.send(msgFlame.set(value_flame==HIGH ? 1 : 0));
     if (value_flame == HIGH) 
     {
        oldValue_flame = value_flame;
        alert_user();
     }
  }
  oldValue_flame = value_flame;
  delay(50);
}

void alert_user()
{
    debouncer.update();
    int value = debouncer.read();
    while(value==HIGH)
    {
        digitalWrite(ALARM_PIN, HIGH);
        delay(200);
        digitalWrite(ALARM_PIN, LOW);
        delay(100);
        debouncer.update();
        value = debouncer.read();
        delay(100);
     }

     gas_check();
  
     flame_check();
}
