#include <MySensor.h>
#include <SPI.h>
#include <Bounce2.h>

#define RELAY_PIN  8 
 
#define BUTTON_PIN  5 
#define ENERGY_SAVING 4

#define ENERGY_SAVING_ON_OFF 6
#define ON 3
#define OFF 2
#define LIGHT_SENSOR_ANALOG_PIN A0

#define RELAY_ON 1
#define RELAY_OFF 0

#define CHILD_ID_LIGHT 0
#define CHILD_ID_LIGHT_SENSOR 1

int lightLevel;
int lastLightLevel;
unsigned long previousMillis = 0;
unsigned long sense_previousMillis = 0;
const long interval = 1000;
const long sense_interval = 30000;

Bounce debouncer_light = Bounce();

int oldValue_light=0;
int oldValue_energy=0;
bool state_light;
bool state_energy_saving;
bool old_state;

MySensor gw;

MyMessage msgLight(CHILD_ID_LIGHT,V_LIGHT);
MyMessage msgLightSensor(CHILD_ID_LIGHT_SENSOR, V_LIGHT_LEVEL);
void incomingMessage(const MyMessage &message);

void setup() {
  
    gw.begin(incomingMessage, AUTO, true);
    
    gw.sendSketchInfo("Energy Saver Switch", "1.0");

    pinMode(BUTTON_PIN,INPUT);
    digitalWrite(BUTTON_PIN,LOW);
    pinMode(ENERGY_SAVING,INPUT);
    digitalWrite(ENERGY_SAVING,LOW);
    
    debouncer_light.attach(BUTTON_PIN);
    debouncer_light.interval(5);

    gw.present(CHILD_ID_LIGHT, S_LIGHT);
    gw.present(CHILD_ID_LIGHT_SENSOR, S_LIGHT_LEVEL);

    pinMode(RELAY_PIN, OUTPUT); 
    pinMode(ENERGY_SAVING_ON_OFF, OUTPUT);
    digitalWrite(ENERGY_SAVING_ON_OFF,HIGH);
    pinMode(ON,OUTPUT);
    pinMode(OFF,OUTPUT);
    digitalWrite(ON,HIGH);
    digitalWrite(OFF,HIGH);
        
    state_light = gw.loadState(CHILD_ID_LIGHT);
    digitalWrite(RELAY_PIN, state_light?RELAY_ON:RELAY_OFF);

    digitalWrite(ON, state_light?RELAY_OFF:RELAY_ON);
       
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long sense_currentMillis = millis();
  gw.process();

  if (sense_currentMillis - sense_previousMillis >= sense_interval) 
  {
    sense_previousMillis = sense_currentMillis;
    lightLevel = (1023-analogRead(LIGHT_SENSOR_ANALOG_PIN))/10.23; 
  
    if ((lightLevel - lastLightLevel>5)|(lastLightLevel - lightLevel>5)) 
    {
      gw.send(msgLightSensor.set(lightLevel));
      lastLightLevel = lightLevel;
    }
  }
  
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;
    if(digitalRead(ENERGY_SAVING)==HIGH)
    {
      if(state_light==LOW)
      {
        digitalWrite(ON,HIGH);
        digitalWrite(ENERGY_SAVING_ON_OFF,HIGH);
        digitalWrite(OFF,LOW);
        delay(200);
        digitalWrite(OFF,HIGH);
        digitalWrite(ENERGY_SAVING_ON_OFF,LOW);
        delay(200);
        digitalWrite(ENERGY_SAVING_ON_OFF,HIGH);
      }
      else
      {
        digitalWrite(OFF,HIGH);
        digitalWrite(ENERGY_SAVING_ON_OFF,HIGH);
        digitalWrite(ON,LOW);
        delay(200);
        digitalWrite(ON,HIGH);
        digitalWrite(ENERGY_SAVING_ON_OFF,LOW);
        delay(200);
        digitalWrite(ENERGY_SAVING_ON_OFF,HIGH);
      }
    }
    else
    {
      if(state_light==HIGH)
      {
        digitalWrite(ENERGY_SAVING_ON_OFF,HIGH);
        digitalWrite(OFF,HIGH);
        if(digitalRead(ON)==LOW) 
        {
          digitalWrite(ON,HIGH);
        }
        else{
          digitalWrite(ON,LOW);
        }
      }
      else
      {
        digitalWrite(ENERGY_SAVING_ON_OFF,HIGH);
        digitalWrite(ON,HIGH);
        if(digitalRead(OFF)==LOW)
        {
          digitalWrite(OFF,HIGH);
        }
        else
        {
          digitalWrite(OFF,LOW);
        }
     }
    }
  }
  
  
  debouncer_light.update();
  
  int value_light = debouncer_light.read();
  if (value_light != oldValue_light && value_light==0) {
    if((digitalRead(ENERGY_SAVING)==HIGH)&(lightLevel>60))
    {
      gw.send(msgLight.set(false), true); // Send new state and request ack back
    }
    else
    {
     gw.send(msgLight.set(state_light?false:true), true); 
    }
  }
  oldValue_light = value_light;
  
  

}

void incomingMessage(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }

  if (message.type == V_LIGHT) {
     // Change relay state
     state_light = message.getBool();
     if((digitalRead(ENERGY_SAVING)==HIGH)&(state_light==true)&(lightLevel>60))
     {
     //   state_light=!state_light;
        digitalWrite(RELAY_PIN, LOW);
        delay(2000);
        state_light = false;
        delay(2000);
        gw.send(msgLight.set(false), true);
        delay(6000);
     }
     else
     {
        digitalWrite(RELAY_PIN, state_light?RELAY_ON:RELAY_OFF);
     }
     gw.saveState(CHILD_ID_LIGHT, state_light);
    
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
}
