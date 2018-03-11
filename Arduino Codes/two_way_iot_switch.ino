#include <MySensor.h>
#include <SPI.h>
#include <Bounce2.h>

#define RELAY_PIN  4
#define ON 5
#define OFF 6
   
#define BUTTON_PIN  3  
 
#define CHILD_ID 1   

#define RELAY_ON 1
#define RELAY_OFF 0

Bounce debouncer = Bounce(); 

int oldValue=0;
bool state;
unsigned long previousMillis = 0;
const long blink_interval = 2000;

MySensor gw;
MyMessage msg(CHILD_ID,V_LIGHT);

void setup()  
{  
  gw.begin(incomingMessage, AUTO, true);

  gw.sendSketchInfo("Two Way Internet Connected Switch", "1.0");
  
  pinMode(BUTTON_PIN,INPUT);
  digitalWrite(BUTTON_PIN,LOW);
  
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  gw.present(CHILD_ID, S_LIGHT);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  pinMode(ON, OUTPUT);
  digitalWrite(ON, RELAY_ON);
  pinMode(OFF, OUTPUT);
  digitalWrite(ON, RELAY_ON);   
   
  state = gw.loadState(CHILD_ID);
  digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
}

void loop() 
{
  gw.process();
  debouncer.update();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blink_interval) 
  {
    previousMillis = currentMillis;
    if(digitalRead(RELAY_PIN)==HIGH)
    {
      digitalWrite(OFF,HIGH);
      digitalWrite(ON,LOW);
    }
    else
    {
      digitalWrite(ON,HIGH);
      digitalWrite(OFF,LOW);
    }
  }
  
  int value = debouncer.read();
  if (value != oldValue && value==0) 
  {
      gw.send(msg.set(state?false:true), true); 
  }
  oldValue = value;
} 
 
void incomingMessage(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }

  if (message.type == V_LIGHT) {
     // Change relay state
     state = message.getBool();
     digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     gw.saveState(CHILD_ID, state);
    
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
 }

