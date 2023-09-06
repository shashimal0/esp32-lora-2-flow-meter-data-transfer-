//Libraries for OLED Display
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2lib.h>

//define the pins used by the LoRa transceiver module
#define SCK               5
#define MISO              19
#define MOSI              27
#define SS                18
#define RST               23
#define DIO0              26

#define BAND              868E6

//OLED pins
#define OLED_SDA          21
#define OLED_SCL          22
#define OLED_RST          3 // RXD pin in Lora 32 V1.6 board
#define SCREEN_WIDTH      128 // OLED display width, in pixels
#define SCREEN_HEIGHT     64 // OLED display height, in pixels

#define BOARD_LED         25

//packet counter
int counter = 0;

//trigger and echo pin
const unsigned int TRIG_PIN=15;
const unsigned int ECHO_PIN=13;


//flow sensor 1
int FLOW_PIN =12;
volatile long pulse_freq;
unsigned long lastTime;
float flow, tflow, ftflow;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void increase () // Interrupt function

{
  pulse_freq++;
}

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  //ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  //flow sensor
  pinMode(FLOW_PIN, INPUT);
  attachInterrupt(12, increase, RISING);

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);

  digitalWrite(OLED_RST, LOW);  delay(20);
  digitalWrite(OLED_RST, HIGH); delay(20);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER ");
  display.display();
  
  Serial.println("LoRa Sender Test");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

void loop() 
{
//ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

 const unsigned long duration= pulseIn(ECHO_PIN, HIGH);
 int distance= (duration*0.034)/2;
 if(duration==0){
   Serial.println("Warning: no pulse from sensor");
   display.setCursor(0,70);
   display.println("Warning: no pulse from sensor");
   } 
  else{
      Serial.print("distance to nearest object: ");
      Serial.print(distance);
      Serial.println(" cm");
  }

//flow sensor
  flow = 2.25 * pulse_freq;
  tflow = flow/1000;
  ftflow += tflow;
  if (millis() - lastTime > 1000)
  {
    pulse_freq= 0;
    lastTime = millis();
  }
  Serial.print(flow);
  Serial.println(" mL/s");  

  Serial.print("Sending packet: ");
  Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("Water level   : ");
  if(distance<=50)
  {
    LoRa.println(distance);
  }
  else{
    LoRa.println("empty");
  }
  LoRa.print("Flowrate(mL/s): ");
  LoRa.println(flow,2);
  LoRa.print("Total Flow (L): ");
  LoRa.println(ftflow,2);
  LoRa.endPacket();
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LORA SENDER");
  display.setTextSize(1);
  display.setCursor(0,30);
  display.print("Water Level   :");
  display.setCursor(90, 30);
  if(distance>60){
    display.print("Empty");
  }
  else{
    display.print(distance);
  }
  display.setCursor(0, 40);
  display.print("Flowrate(mL/s):");
  display.setCursor(90, 40);
  display.print(flow,2); 
  display.setCursor(0, 50);  
  display.print("Total flow(L) :");
  display.setCursor(90, 50);
  display.print(ftflow,2);
  display.display();
  counter++;
  
  delay(1000);
}




