#include <SPI.h>
#include <MySensor.h>  

#include <DHT.h>
#include <Bounce2.h>  

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_AC 2

#define HUMIDITY_SENSOR_DIGITAL_PIN 4
#define RELAY_PIN  3  
#define BUTTON_PIN  2

#define ON 5
#define OFF 6

#define RELAY_ON 1
#define RELAY_OFF 0

Bounce debouncer = Bounce(); 
int oldValue=0;
bool state;
unsigned long previousMillis = 0;
unsigned long light_previousMillis = 0;
const long interval = 120000;
const long blink_interval = 2000;

MySensor gw;

DHT dht;
float lastTemp;
float lastHum;
boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msg(CHILD_ID_AC,V_LIGHT);
void tempsense();

void setup() {
  
  gw.begin(incomingMessage, AUTO, true);
  
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 

  gw.sendSketchInfo("Humidity & Temp with AC ctrl", "1.0");
  
  pinMode(BUTTON_PIN,INPUT);
  digitalWrite(BUTTON_PIN,HIGH);
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);
  
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_AC, S_LIGHT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  pinMode(ON, OUTPUT);
  digitalWrite(ON, RELAY_ON);
  pinMode(OFF, OUTPUT);
  digitalWrite(OFF, RELAY_ON);
    
  
  metric = gw.getConfig().isMetric;

  state = gw.loadState(CHILD_ID_AC);
  digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);

  delay(1000); 
}

void loop() 
{
  
  gw.process();
  debouncer.update();
  unsigned long light_currentMillis = millis();
  if (light_currentMillis - light_previousMillis >= blink_interval) 
  {
    light_previousMillis = light_currentMillis;
    if(state==HIGH)
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
  if (value != oldValue && value==0) {
      gw.send(msg.set(state?false:true), true); // Send new state and request ack back
  }
  oldValue = value;  

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis; 
    tempsense();
  }
}

void tempsense()
{
  //delay(dht.getMinimumSamplingPeriod());
  float temperature = dht.getTemperature();
  
  if (isnan(temperature))
  {
      Serial.println("Failed reading temperature from DHT");
  } 
  
  else if (temperature != lastTemp) 
  {
    lastTemp = temperature;
    if (!metric)
    {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  }
  
  float humidity = dht.getHumidity();
  if (isnan(humidity)) 
  {
      Serial.println("Failed reading humidity from DHT");
  } 
  
  else if (humidity != lastHum) 
  {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
      Serial.print("H: ");
      Serial.println(humidity);
  }
}

void incomingMessage(const MyMessage &message) {
  
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }

  if (message.type == V_LIGHT) {
     
     state = message.getBool();
     digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
     
     gw.saveState(CHILD_ID_AC, state);
    
     
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
}
