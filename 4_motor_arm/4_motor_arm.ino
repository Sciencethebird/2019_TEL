

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

    servo_set(int num_of_servo, int* init_degree) { // constructor, init #servo and starting degree
      this->message_length = num_of_servo * 3 + 7; // total message length including 0x55 0x55
      this->num_of_servo = num_of_servo;
      this->servo_degrees = new int[num_of_servo]; // stores current degree of servo;
      this->zero_points = new int[num_of_servo];  // stores origin of each srvo
      for (int i = 0; i < num_of_servo; i++) {
        servo_degrees[i] = init_degree[i];
        zero_points[i] = init_degree[i];
      }
      send_message(2);
    }

    void move_single_servo(int servo_num, int dt, int dtheta) { // move to certain degree
      servo_degrees[servo_num] += dtheta; // servo_num starts from zero
      if (servo_degrees[servo_num] < 0) servo_degrees[servo_num] = 0;
      if (servo_degrees[servo_num] > 1000) servo_degrees[servo_num] = 1000;
      send_message(dt);
    }

    void move_radii(float dr, int dt) {   // input amount of distance you want to move
      float k = (float)(1000 - servo_degrees[0]) / 1000;
      //servo_degrees[0] += int (4 * k * dr);
      servo_degrees[1] += dr;
      servo_degrees[2] += dr;
      servo_degrees[3] += dr;
      for (int i = 0; i < 4; i++) {
        if (servo_degrees[i] < 0) {
          Serial.write('no');
          servo_degrees[i] = 0;
        }
        if (servo_degrees[i] > 1000) {
          Serial.write('no');
          servo_degrees[i] = 1000;
        }
      }
      send_message(dt);
    }

    void move_height(float dh, int dt) {  // input amount of distance you want to move

      servo_degrees[1] -= dh;
      servo_degrees[2] += dh;
      servo_degrees[3] -= 1.5 * dh;
      for (int i = 0; i < num_of_servo; i++) {
        if (servo_degrees[i] < 0) servo_degrees[i] = 0;
        if (servo_degrees[i] > 1000) servo_degrees[i] = 1000;
      }
      send_message(dt);
    }

    void move_to_origin() {

      for (int i = 0; i < num_of_servo; i++) {
        servo_degrees[i] = zero_points[i];
      }
      move_single_servo(1, 1500, 0); // call this function to move all servo
    }

    void send_message(int dt) {
      byte message[message_length] = {85, 85, message_length - 2, 0x03, num_of_servo, dt, dt >> 8};
      int base_length = 7;

      for (int i = 0; i < num_of_servo; i++) {
        message[base_length] = i; // servo num, starts from 0
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

    int *servo_degrees; // stores current servo degree.
    int *zero_points;   // zero points for set original
};

class my_servo {
  public:
    my_servo(Servo &ser, int lb, int zero, int ub) {
      s = ser;
      //s.write(zero);
      this->zero_point = zero;
      this->servo_degree = zero;
      lower_bound = lb;
      upper_bound = ub;
    }

    void move_by(int dx, int dt = 5) {

      servo_degree += dx;
      if (servo_degree > upper_bound) servo_degree = upper_bound;
      if (servo_degree < lower_bound) servo_degree = lower_bound;
      s.write(servo_degree);
      delay(dt);
    }
    void move_to(int degree, int dx = 1) {
      if (servo_degree - degree > 0) dx = -2;
      else dx = 2;
      while (abs(servo_degree - degree) > 5) {
        servo_degree += dx;
        delay(30);
        s.write(servo_degree);
      }
      servo_degree = degree;
      s.write(servo_degree);
    }

    void move_to_origin() {
      move_to(zero_point);
    }
  private:
    Servo s;
    int servo_pin;
    int servo_degree;
    int zero_point;
    int lower_bound, upper_bound;
};




//*********************** function declaration *****************************//
void ps2_setup();

float Polar_Angle(float, float);  //y, x
float Polar_Length(float, float);

// ps2 controller status
int error = 0;
byte type = 0;
byte vibrate = 0;

PS2X ps2x;
Servo myservo;  // claw clamp servo
Servo claw_rotate;

// mg996 s
my_servo claw(myservo, 140, 170, 180); // pin ,lower bound, zero, upper bound
my_servo claw_rotation(claw_rotate, 20, 85, 150);

//int claw_pos = 170;
//int claw_rotation = 85; // init claw ratation pos

// servo class initialization
int degree[4] = {500, 470, 150, 150};  // init servo degrees;
servo_set test(4, degree);

// two speed array for different speed mode
const int normal[] = {3, 5, 4};
const int fine_tune[] = {1, 1, 1};
int* servo_speed = normal;

// startup


//*********************** setup *****************************//
void setup() {

  Serial.begin(9600);
  myservo.attach(4);
  claw_rotate.attach(3);

  claw.move_to_origin();
  claw_rotation.move_to_origin();
  ps2_setup();
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

      test.move_single_servo(0, 1, -2); // base_rotation
      return;

    } else if (left_joystick_angle >= 67.5 && left_joystick_angle < 112.5) {

      test.move_radii(5, 2);;
      return;

    } else if (left_joystick_angle >= 157.5 && left_joystick_angle < 202.5) {

      test.move_single_servo(0, 1, 2); // base_rotation
      return;

    } else if (left_joystick_length > 200 && left_joystick_angle >= 247.5 && left_joystick_angle < 292.5) {

      test.move_radii(-5, 2);
      return;
    }


    /// right joystick
    if (right_joystick_angle < 22.5 || right_joystick_angle >= 337.5) {  // right-right

      test.move_single_servo(0, 1, -2); // base_rotation
      return;

    } else if (right_joystick_angle >= 67.5 && right_joystick_angle < 112.5) { // right-up

      test.move_height(2, 20);
      return;

    } else if (right_joystick_angle >= 157.5 && right_joystick_angle < 202.5) { // right-left

      test.move_single_servo(0, 1, 2);
      return;

    } else if (right_joystick_angle >= 247.5 && right_joystick_angle < 292.5) { // right-down

      test.move_height(-2, 20);
      return;
    }
  }

  if (ps2x.ButtonPressed(PSB_CIRCLE)) {
    if (servo_speed == normal) servo_speed = fine_tune;
    else servo_speed = normal;
    Serial.println("speed changed");
    delay(25);
  }

  if (ps2x.ButtonPressed(PSB_SQUARE)) {
    test.move_to_origin();
    servo_speed = normal;
    claw.move_to_origin();
    claw_rotation.move_to_origin();
    delay(25);
  }

  if (ps2x.Button(PSB_R2)) {

    claw.move_by(2);

  }  else if (ps2x.Button(PSB_L2)) {
    claw.move_by(-2);
  }

  if (ps2x.Button(PSB_R1)) { // claw rotation
    claw_rotation.move_by(2);

  }  else if (ps2x.Button(PSB_L1)) {
    claw_rotation.move_by(-2);
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
