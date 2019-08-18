//#include <bitset>

int incomingByte = 0;  // 用來儲存收進來的 data byte
class servo_set {
  public:
    servo_set(int num_of_servo) {
      this->message_length = num_of_servo * 3 + 7; // total message length including 0x55 0x55
      this->num_of_servo = num_of_servo;
      servo_degrees = new int [num_of_servo];
      for(int i = 0; i< num_of_servo; i++){
        servo_degrees[i] = 0;
      }
    }
    void move_single_servo(int servo_num, int dt, int degree) {
      byte message[message_length] = {85, 85, message_length-2, 0x03, num_of_servo, dt, dt >> 8};
      servo_degrees[servo_num-1] = degree;
      int base_length = 7;
      
      for(int i = 0; i<num_of_servo; i++){
        message[base_length] = i+1;
        message[base_length+1] = servo_degrees[i];
        message[base_length+2] = servo_degrees[i] >>8;
        base_length+=3;
      }
      //message[7] = servo_num;
      //message[8] = degree;
      //message[9] = degree>>8;
      //byte message[] = {85, 85, message_length, 0x03, num_of_servo, 0xE8, 0x03, 0x01, 0x20, 0x03};
      for (int i = 0; i < message_length; i++) {
        Serial.write(message[i]);
      }
    }
  private:
    int message_length = 0;
    int num_of_servo = 0;
    int *servo_degrees;

};
void setup() {
  // 開啟 Serial port, 通訊速率為 9600 bps
  Serial.begin(9600);
  Serial.println("Hello Arduino");
  
}
servo_set test(6);
byte head = 0x55;
byte l = 0x08;
byte instr = 0x03;
byte variable[] = {85, 85, 0x08, 0x03, 0x01, 0xE8, 0x03, 0x01, 0x20, 0x03};
byte variable2[] = {0x55, 0x55, 0x08, 0x03, 0x01, 0xE8, 0x03, 0x01, 0x00, 0x01};
void loop() {
  /*
    if (Serial.available() > 0) {
    incomingByte = Serial.read();
    Serial.println(incomingByte);
    }*/
    /*
  int lll = 800;
  byte a = lll;
  byte b = lll >> 8;
  Serial.println(a, HEX);
  Serial.println(b);
  delay(2000);*/
  int degree = 0;
  for(int i = 0; i< 400; i+=5){
    test.move_single_servo(1, 5, i);
    //delay(200);
  }
  for(int i = 400; i>0; i-=5){
    test.move_single_servo(1, 5, i);
    //delay(200);
  }
  
  
  //test.move_single_servo(1, 500, 0);
  //delay(2000);
  /*
  test.move_single_servo(1, 500, 800);
  delay(2000);
  test.move_single_servo(2, 500, 0);
  delay(2000);
  test.move_single_servo(2, 500, 800);
  delay(2000);
  test.move_single_servo(3, 1000, 0);
  delay(2000);
  test.move_single_servo(3, 1000, 800);
  delay(2000);
  test.move_single_servo(4, 1000, 0);
  delay(2000);
  test.move_single_servo(4, 1000, 800);
  delay(2000);
  test.move_single_servo(5, 1000, 0);
  delay(2000);
  test.move_single_servo(5, 1000, 400);
  delay(2000);
  test.move_single_servo(6, 1000, 0);
  delay(2000);
  test.move_single_servo(6, 1000, 800);
  delay(2000);*/
  /*
    for (int i = 0; i < 10; i++) {
      Serial.write(variable[i]);
    }
    delay(2000);
    for (int i = 0; i < 10; i++) {
      Serial.write(variable2[i]);
    }
    delay(2000);*/
}
