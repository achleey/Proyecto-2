
void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
//  Serial2.begin(9600);
  Serial3.begin(9600);
}

void loop() {
  // read from port 1, send to port 0:
  //Joystick - Arduino 1
//  if (Serial2.available()) {
//    
//    int inByte = Serial2.read();
//    Serial.write(inByte); 
//    
//  }

  //Joystick - Arduino 2
  if (Serial3.available()){

    int inByte2 = Serial3.read();
    Serial.write(inByte2);
    
    }
}
