

#include <math.h>
#include <PS2X_lib.h>
#include <Servo.h>

#define PI 3.14159265

#define PS2_DAT        8   //14
#define PS2_CMD        9   //15
#define PS2_SEL        10  //16
#define PS2_CLK        11  //17

//************************** LeArm Servos *******************************
class servo_set {
  public:
  
    servo_set(int num_of_servo, int* degrees_of_servo) { // constructor
      this->message_length = num_of_servo * 3 + 7; // total message length including 0x55 0x55
      this->num_of_servo = num_of_servo;
      servo_degrees = degrees_of_servo;

    }
    
    void move_single_servo(int servo_num, int dt, int &degree) { // move to certain degree

      if (degree < 0) degree = 0;
      if (degree > 1000) degree = 1000;
      servo_degrees[servo_num - 1] = degree;
      send_message(dt);
    }

    void move_radii(float dr, int dt) {   // input amount of distance you want to move
      float k = (float)(1000 - servo_degrees[0]) / 1000;
      servo_degrees[0] += int (4 * k * dr);
      servo_degrees[1] -= dr;
      servo_degrees[2] += dr;
      servo_degrees[3] += int (4 * k * dr) - 2 * dr;
      for (int i = 0; i < 4; i++) {
        if (servo_degrees[i] < 0) servo_degrees[i] = 0;
        if (servo_degrees[i] > 1000) servo_degrees[i] = 1000;
      }
      send_message(dt);
    }

    void move_height(float dh, int dt) {  // input amount of distance you want to move

      servo_degrees[0] += dh;
      servo_degrees[2] -= dh;
      servo_degrees[3] += 2 * dh;
      for (int i = 0; i < 4; i++) {
        if (servo_degrees[i] < 0) servo_degrees[i] = 0;
        if (servo_degrees[i] > 1000) servo_degrees[i] = 1000;
      }
      send_message(dt);
    }

    void move_to_origin() {
      int degree[6] = {120, 850, 150, 500, 500, 500};
      for (int i = 0; i < 6; i++) {
        servo_degrees[i] = degree[i];
      }
      move_single_servo(1, 1500, degree[0]);
    }

    void send_message(int dt) {
      byte message[message_length] = {85, 85, message_length - 2, 0x03, num_of_servo, dt, dt >> 8};
      int base_length = 7;

      for (int i = 0; i < num_of_servo; i++) {
        message[base_length] = i + 1;
        message[base_length + 1] = servo_degrees[i];
        message[base_length + 2] = servo_degrees[i] >> 8;
        base_length += 3;
      }
      for (int i = 0; i < message_length; i++) {
        Serial.write(message[i]);
      }
    }
  private:
    int message_length = 0;
    int num_of_servo = 0;
    float radii = 0;
    float height = 0;
    int *servo_degrees; // this pointer links an array of degrees outside of this class, not a good practice but that's what it is right now
};

//*********************** function declaration *****************************//
void ps2_setup();

float Polar_Angle(float, float);  //y, x
float Polar_Length(float, float);

int error = 0; // ps2 controller status
byte type = 0;
byte vibrate = 0;

PS2X ps2x;
Servo myservo;  // claw servo

int claw_pos = 0;
int degree[6] = {120, 850, 150, 500, 500, 500}; // stores degrees of servos, links to "servo_set" class

servo_set test(6, degree); // numbers of servo, the array to store degrees of servos

//*********************** setup *****************************//
void setup() {

  Serial.begin(9600);

  ps2_setup();

  myservo.attach(3);
  myservo.write(0);
  //*********************PIN MODE*************************
  for (int i = 0; i < 6; i++)  test.move_single_servo(i + 1, 1000, degree[i]);
}




void loop() {

  bool motorstate = 0;

  float left_joystick_angle = 0;
  float left_joystick_length = 0;

  float right_joystick_angle = 0;
  float right_joystick_length = 0;

  ps2x.read_gamepad(false, vibrate);
  delay(20);

  if (ps2x.ButtonPressed(PSB_SQUARE) && ps2x.ButtonPressed(PSB_CIRCLE)) return; /// if square is pressed means noise

  int ly = (int)ps2x.Analog(PSS_LY);
  int lx = (int)ps2x.Analog(PSS_LX);
  int ry =  (int)ps2x.Analog(PSS_RY);
  int rx =  (int)ps2x.Analog(PSS_RX); // rx control spin


  lx = (float)map(lx, 0, 255, -200, 200);
  ly = (float)map(ly, 0, 255, 200, -200);
  rx = (float)map(rx, 0, 255, -200, 200);
  ry = (float)map(ry, 0, 255, 200, -200);

  lx == 0 ? lx = 1 : 1;
  ly == 0 ? ly = 1 : 1;
  rx == 0 ? rx = 1 : 1;
  ry == 0 ? ry = 1 : 1;
  //rx == 1 ? rx = 0 : 1; // if rx == 1 then rx = 0, else do nothing


  if (lx == 1 && ly == 1 && rx == 1 && ry == 1) {
    motorstate = 0;
  } else {
    motorstate = 1;
    left_joystick_angle  = Polar_Angle(static_cast<float>(lx), static_cast<float>(ly));
    left_joystick_length = Polar_Length(static_cast<float>(lx), static_cast<float>(ly));

    right_joystick_angle  = Polar_Angle(static_cast<float>(rx), static_cast<float>(ry));
    right_joystick_length = Polar_Length(static_cast<float>(rx), static_cast<float>(ry));
  }

  if (motorstate == 0) {
    //motorstop();
  } else {

    if (left_joystick_angle < 22.5 || left_joystick_angle >= 337.5) {
      // nothing right now
      return;

    } else if (left_joystick_angle >= 67.5 && left_joystick_angle < 112.5) {

      test.move_height(-2, 10);
      return;

    } else if (left_joystick_angle >= 157.5 && left_joystick_angle < 202.5) {

      // nothing right now
      return;

    } else if (left_joystick_length > 200 && left_joystick_angle >= 247.5 && left_joystick_angle < 292.5) {

      test.move_height(2, 10);
      return;
    }


    /// right joystick
    if (right_joystick_angle < 22.5 || right_joystick_angle >= 337.5) {  // right-right
      if (degree[0] > 600) degree[5] -= 2;
      else degree[5] -= 5;
      test.move_single_servo(6, 1, degree[5]); // base_rotation
      return;

    } else if (right_joystick_angle >= 67.5 && right_joystick_angle < 112.5) { // right-up

      if (degree[0] > 500) test.move_radii(2, 10); 
      else test.move_radii(4, 10);
      return;

    } else if (right_joystick_angle >= 157.5 && right_joystick_angle < 202.5) { // right-left
      if (degree[0] > 600) degree[5] += 2;
      else degree[5] += 5;
      test.move_single_servo(6, 1, degree[5]); // base_rotation
      return;

    } else if (right_joystick_angle >= 247.5 && right_joystick_angle < 292.5) { // right-down
      if (degree[0] > 500) test.move_radii(-2, 10);
      else test.move_radii(-4, 10);

      return;
    }
  }


  if (ps2x.ButtonPressed(PSB_SQUARE)) {
    test.move_to_origin();
    delay(25);
  }


  if (ps2x.Button(PSB_R2)) {

    claw_pos += 5;
    if (claw_pos > 60) claw_pos = 60;
    myservo.write(claw_pos);
    delay(50);

  }  else if (ps2x.Button(PSB_L2)) {
    claw_pos -= 5;
    if (claw_pos < 0) claw_pos = 0;
    myservo.write(claw_pos);
    delay(50);
  }

  if (ps2x.Button(PSB_R1)) { // claw rotation

    degree[4] += 5;
    test.move_single_servo(5, 1, degree[4]); 
    return;

  }  else if (ps2x.Button(PSB_L1)) {

    degree[4] -= 5;
    test.move_single_servo(5, 1, degree[4]);
    return;
  }
}



//****************** function implementation ********************//

// helper functions
float Polar_Angle(float x, float y) {
  if (x > 0 && y > 0) return atan((float)y / (float)x) * 180 / PI;
  else if (x < 0) return atan((float)y / (float)x) * 180 / PI + 180;
  else return atan((float)y / (float)x) * 180 / PI + 360;
};
float Polar_Length(float x, float y) {
  return sqrt(x * x + y * y);
};


void ps2_setup() {

  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  //error = ps2x.config_gamepad(false, false);
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);

  if (error == 0) {
    Serial.print("Found Controller, configured successful ");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }
  else if (error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");

  else if (error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if (error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");

  type = ps2x.readType();
  switch (type) {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
    case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
  }
}

//message[7] = servo_num;
//message[8] = degree;
//message[9] = degree>>8;
//byte message[] = {85, 85, message_length, 0x03, num_of_servo, 0xE8, 0x03, 0x01, 0x20, 0x03};
/// mecanum wheel control code was written by eason27271563

/*
  this->radii = r;
  double mid_line_square = pow(radii, 2) + pow(height, 2);
  double a1_square = pow(arm_len_1, 2);
  double a2_square = pow(arm_len_2, 2);
  double theta_base = (a1_square + mid_line_square - a2_square)/(2*arm_len_1*sqrt(mid_line_square))  + atan(height/radii);
  double theta_top = 0.5*PI - (a1_square + a2_square -  mid_line_square)/(2*arm_len_1*arm_len_2);*/
