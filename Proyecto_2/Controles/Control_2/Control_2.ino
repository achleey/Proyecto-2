#include <SoftwareSerial.h>
uint8_t VRx = A0;
uint8_t VRy = A1;
uint8_t SW = 2;

uint8_t xPosition = 0;
uint8_t yPosition = 0;
uint8_t SW_state = 0;
uint8_t mapX = 0;
uint8_t mapY = 0;

const byte rxPin = 0;
const byte txPin = 1;

SoftwareSerial mySerial(rxPin, txPin); // RX, TX

void setup() {
  //joystick
  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);
  pinMode(SW, INPUT_PULLUP);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  Serial.println("PRUEBA");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
}

void loop() { // run over and over

  //Joystick 
   xPosition = analogRead(VRx);
   yPosition = analogRead(VRy);
   SW_state = digitalRead(SW);

   mapX = map(xPosition, 0, 1023, -512, 512);
   mapY = map(yPosition, 0, 1023, -512, 512);

   Serial.print("X:");
   Serial.print(mapX);
   Serial.print(" | Y:");
   Serial.print(mapY);
   Serial.println("Button 1:");
   Serial.println(SW_state);
   
 delay(750);

}
