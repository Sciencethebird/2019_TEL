
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
    servo_set(int num_of_servo) {
      this->message_length = num_of_servo * 3 + 7; // total message length including 0x55 0x55
      this->num_of_servo = num_of_servo;
      servo_degrees = new int [num_of_servo];
      for (int i = 0; i < num_of_servo; i++) {
        servo_degrees[i] = 500;
      }
    }
    void move_single_servo(int servo_num, int dt, int degree) {

      byte message[message_length] = {85, 85, message_length - 2, 0x03, num_of_servo, dt, dt >> 8};
      servo_degrees[servo_num - 1] = degree;
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
    int *servo_degrees;

};

//*********************** function declaration *****************************//
void ps2_setup();

float Polar_Angle(float, float);  //y, x
float Polar_Length(float, float);

int error = 0; // ps2 controller status
byte type = 0;
byte vibrate = 0;

bool stpr_enable_state = false;


PS2X ps2x;
Servo myservo;  // claw servo


//*********************** setup *****************************//
void setup() {

  Serial.begin(9600);

  ps2_setup();

  myservo.attach(3);
  //*********************PIN MODE*************************
}

servo_set test(6);

int claw_pos = 0;

int degree = 500;
int degree2 = 500;
int degree3 = 500;
int degree4 = 500;
int degree5 = 500;

void loop() {


  bool motorstate = 0;

  float left_joystick_angle = 0;
  float left_joystick_length = 0;

  float right_joystick_angle = 0;
  float right_joystick_length = 0;

  ps2x.read_gamepad(false, vibrate);
  delay(50);

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
    //rx = map(rx, -200, 200, -25, 25);

    if (left_joystick_angle < 22.5 || left_joystick_angle >= 337.5) {

      if (degree < 0) {
        degree = 0;
        Serial.println("no");
      }

      degree -= 5;
      test.move_single_servo(2, 1, degree);


      return;

    } else if (left_joystick_angle >= 67.5 && left_joystick_angle < 112.5) {


      if (degree2 > 1000) {
        degree2 = 1000;
        Serial.println("no");
      }
      degree2 += 5;
      test.move_single_servo(1, 1, degree2);

      return;

    } else if (left_joystick_angle >= 157.5 && left_joystick_angle < 202.5) {

      if (degree > 1000) {
        degree = 1000;
        Serial.println("no");
      }
      degree += 5;
      test.move_single_servo(2, 1, degree);


      return;

    } else if (left_joystick_length > 100 && left_joystick_angle >= 247.5 && left_joystick_angle < 292.5) {
      if (degree2 < 0) {
        degree2 = 0;
        Serial.println("no");
      }

      degree2 -= 5;
      test.move_single_servo(1, 1, degree2);
      return;
    }

  }
  if (ps2x.ButtonPressed(PSB_SQUARE)) {
    degree = 200;
    degree2 = 500;
    degree3 = 500;
    degree4 = 500;
    degree5 = 500;
    test.move_single_servo(2, 1000, degree);
    test.move_single_servo(1, 1000, degree2);
    test.move_single_servo(3, 1000, degree3);
    test.move_single_servo(4, 1000, degree4);
    test.move_single_servo(5, 1000, degree5);
    //Serial.println("PSB_UP just pressed");
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

  /// right joystick
  //Serial.println(right_joystick_angle);
  /*
    if (right_joystick_angle < 22.5 || right_joystick_angle >= 337.5) {

    degree3 += 5;
    test.move_single_servo(3, 1, degree3);

    degree5 -= 5;
    test.move_single_servo(5, 1, degree5);

    return;

    } else if (right_joystick_angle >= 67.5 && right_joystick_angle < 112.5) {

    degree4 += 5;
    test.move_single_servo(4, 1, degree4);

    return;

    } else if (right_joystick_angle >= 157.5 && right_joystick_angle < 202.5) {

    degree3 -= 5;
    test.move_single_servo(3, 1, degree3);

    degree5 += 5;
    test.move_single_servo(5, 1, degree5);

    return;

    } else if (right_joystick_angle >= 247.5 && right_joystick_angle < 292.5) {

    degree4 -= 5;
    test.move_single_servo(4, 1, degree4);

    return;
    }

    delay(10);
    }

    if (ps2x.Button(PSB_R1)) {

    degree5 += 10;
    test.move_single_servo(5, 1, degree5);

    return;

    }  else if (ps2x.Button(PSB_L1)) {

    degree5 -= 10;
    test.move_single_servo(5, 1, degree5);

    return;
    }*/
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
