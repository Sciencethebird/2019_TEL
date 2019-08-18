


const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)

void setup() {
  // initialize serial communications at 9600 bps:
  pinMode(3, OUTPUT);
  pinMode(A0, INPUT);
  Serial.begin(9600);
}
int count = 0;
bool temp = false;
bool rotate = LOW;
void loop() {
  /*
  temp = digitalRead(A0);
  if (digitalRead(A0) != temp && !temp) {
    count++;
    Serial.println(count);
  }
  if (count > 44) {
    Serial.println(count);
    count = 0;
  }*/

  
    for(int i = 0; i<255; i++){
    analogWrite(analogOutPin, i);
    delay(5);
    }

    for(int i = 255; i>0; i--){
    analogWrite(analogOutPin, i);
    delay(5);
    }
    rotate = !rotate;
    digitalWrite(3, rotate);
  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:

}
