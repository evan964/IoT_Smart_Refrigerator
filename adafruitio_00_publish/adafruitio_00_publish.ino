
#include "config.h"
#include "time.h"
#include <ESP32Servo.h>
#define INTERVAL 100000 //5min
static uint64_t send_interval_ms;

//button
#define button 14
boolean buttonState;


#include <Wire.h>

#include "SparkFunBME280.h"
BME280 mySensor;

int sensorValue;

// this int will hold the current count for our sketch
float fridgeTemp  = 0;
float room_temp = 0;
bool door_open = 0;

bool closed = true;
int door_counter = 0;

int nbopen = 0;

//sonar
#include <NewPing.h>
#define TRIGGER_PIN 5
#define ECHO_PIN 18
#define MAX_DISTANCE 200

// NewPing setup of pins and maximum distance
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 

//servo
Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position

//button
int chabbatButton = 0;
int chabbatState = 0;

// set up the 'counter' feed
AdafruitIO_Feed *DO = io.feed("dooropen");
//AdafruitIO_Feed *Temperature = io.feed("temp");
AdafruitIO_Feed *RT = io.feed("roomtemp");
AdafruitIO_Feed *SH = io.feed("Shabbat");

AdafruitIO_Feed *NB = io.feed("nbopen");

void setup() {

  //led
  pinMode(4, OUTPUT); 

  // start the serial connection
  Serial.begin(9600);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  
//bme
  Serial.println("Reading basic values from BME280");

  Wire.begin();
  mySensor.setI2CAddress(0x76); //Connect to a second sensor

  if (mySensor.beginI2C() == false) //Begin communication over I2C
  {
    Serial.println("The sensor did not respond. Please check wiring.");
    while (1); //Freeze
  }
  Serial.print("REF:   ");
  Serial.println(mySensor.getReferencePressure());
  //  http://tlv-weather.net/
  mySensor.setReferencePressure(1011 * 100);
  
  //servo
  myservo.attach(26);

  //button
  pinMode(button,INPUT_PULLUP);
  
  send_interval_ms = millis();

}

void loop() {

  io.run();
  
    Serial.print("chabbat state ");
    Serial.println(chabbatState);
    
  //button 
  chabbatButton = digitalRead(button);
  Serial.print("button ");
  Serial.println(chabbatButton);
  if (chabbatButton==0 && chabbatState==0){
    chabbatState = 1;
    digitalWrite(4, HIGH);
    SH ->save(chabbatState);
  }
  else if (chabbatButton==0 && chabbatState==1){
    chabbatState = 0;
    digitalWrite(4, LOW);
    SH ->save(chabbatState);
  }

if (chabbatState == 0)
{
  //sonar
  unsigned int distance = sonar.ping_cm();
  Serial.print(distance);
  Serial.println("cm");

  //servo
  if (distance<=6){
    for (pos = 0; pos <= 20; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 70; pos >= 10; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
  }
  }
  
  

  //photo resistance
  sensorValue = analogRead(32);
  if (sensorValue <= 10){
    closed = true;
  }
  if (sensorValue>10 && closed == true){
    door_counter = door_counter+1;
    closed = false;
  }

  if ((int)(millis() - send_interval_ms) >= INTERVAL)
    {
      sensorValue = analogRead(32); // read analog input pin 0
      Serial.print(" open nb: ");
      Serial.print(door_counter, DEC); // prints the value read
      Serial.print(" \n"); // prints a space between the numbers
      nbopen = nbopen + door_counter;
      NB ->save(nbopen);
      DO ->save(door_counter);
      door_counter = 0; //remmettre sans comm
      
      //bme
      fridgeTemp = mySensor.readTempC();
      Serial.print(" Temp: ");
      Serial.print(fridgeTemp, 2);
      RT ->save(fridgeTemp);

      
      send_interval_ms = millis();
   
    }
}
  
  

  /*
  Temperature ->save(temp);
  RT ->save(room_temp);
  */

  // (1000 milliseconds == 1 second) during each loop.
  delay(10);

}
