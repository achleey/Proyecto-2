#include <SoftwareSerial.h>
int VRx = A0;
int xPosition = 0;
int mapX = 0;
String mapXS;

const byte rxPin = 0;
const byte txPin = 1;

SoftwareSerial mySerial(rxPin, txPin); // RX, TX

void setup() {
  //joystick
  pinMode(VRx, INPUT);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
}

void loop() { // run over and over

  //Joystick 
   xPosition = analogRead(VRx);
   mapX = map(xPosition, 0, 1023, 0, 320);


if(mapX < 156){
  mapXS = "L";
  } else if (mapX > 157){
    mapXS = "R";
    }else{
      mapXS = "";
      }

   Serial.print(mapXS);
   
 delay(30);

}
