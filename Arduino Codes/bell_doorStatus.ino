#include <MySensor.h>
#include <SPI.h>
#include <Bounce2.h>

#define CHILD_ID_BELL 1
#define CHILD_ID_DOOR 2

#define BUTTON_PIN  2
#define DOOR_PIN  8  

#define RELAY_PIN  3
#define ON 4
#define OFF 5
#define MED 6
 
MySensor gw;

Bounce debouncer = Bounce(); 
int oldValue_door=-1;
int oldValue_bell=-1;

MyMessage msgDoor(CHILD_ID_DOOR,V_TRIPPED);
MyMessage msgBell(CHILD_ID_BELL,V_TRIPPED);

void doorOpCl();

void setup()  
{  
  gw.begin();

  gw.sendSketchInfo("Door Bell & Door Status Indicator", "1.0");
 
  pinMode(BUTTON_PIN,INPUT);
  digitalWrite(BUTTON_PIN,HIGH);

  pinMode(DOOR_PIN,INPUT);
  digitalWrite(DOOR_PIN,HIGH);

  pinMode(RELAY_PIN,OUTPUT);
  digitalWrite(RELAY_PIN,LOW);
  pinMode(ON,OUTPUT);
  digitalWrite(ON,HIGH);
  pinMode(MED,OUTPUT);
  digitalWrite(MED,HIGH);
  pinMode(OFF,OUTPUT);
  digitalWrite(OFF,HIGH);
  
  
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);
  
  gw.present(CHILD_ID_DOOR, S_DOOR);
  gw.present(CHILD_ID_BELL, S_DOOR);  
}

void loop() 
{
  
  doorOpCl();
  
  debouncer.update();
  
  int value_bell = debouncer.read();
  
  if (value_bell != oldValue_bell) 
  {
     oldValue_bell = value_bell; 
     Serial.println(value_bell);
     gw.send(msgBell.set(value_bell==HIGH ? 0 : 1));
     if (value_bell == 0) 
     {
        Serial.println("activating rely for 1 sec");
        for(int j=0;j<2;j++)
        {
          digitalWrite(RELAY_PIN, HIGH);
          digitalWrite(OFF,HIGH);
          digitalWrite(MED,HIGH);
          digitalWrite(ON,LOW);
          doorOpCl();
          delay(200);
          doorOpCl();
          digitalWrite(RELAY_PIN, LOW);
          doorOpCl();
          delay(300);
          doorOpCl();
          digitalWrite(OFF,HIGH);
          digitalWrite(ON,HIGH);
          digitalWrite(MED,LOW);
          doorOpCl();
          delay(500);
          doorOpCl();
        }
        
        for(int i=0;i<12;i++)
        {
          if(digitalRead(ON)==LOW)
          {
            digitalWrite(ON,HIGH);
            digitalWrite(MED,LOW);
          }
          else
          {
            digitalWrite(MED,HIGH);
            digitalWrite(ON,LOW);
          }
          delay(500);
          doorOpCl();
        }
     }

     digitalWrite(ON,HIGH);
     digitalWrite(MED,HIGH);
     digitalWrite(OFF,LOW);
  }
} 

void doorOpCl()
{
    int value_door = digitalRead(DOOR_PIN);
  
    if (value_door != oldValue_door) 
    {
      oldValue_door = value_door;
      Serial.println(value_door);
      gw.send(msgDoor.set(value_door==HIGH ? 0 : 1));
    }
}
