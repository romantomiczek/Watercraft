#include <Arduino.h>
#include "Config.h"

// you can enable debug logging to Serial at 115200
// #define REMOTEXY__DEBUGLOG

// RemoteXY select connection mode and include library
#define REMOTEXY_MODE__WIFI_POINT

#include <ESP8266WiFi.h>

// RemoteXY connection settings
#define REMOTEXY_WIFI_SSID "Watercraft_Demo"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377

#include <RemoteXY.h>

// RemoteXY GUI configuration
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = // 53 bytes
    {255, 5, 0, 0, 0, 46, 0, 19, 0, 0, 0, 0, 30, 1, 200, 84, 1, 1, 4, 0,
     4, 28, 0, 13, 84, 48, 202, 26, 4, 161, 0, 13, 84, 48, 188, 26, 5, 65, 12, 72,
     72, 41, 175, 26, 24, 4, 0, 255, 6, 85, 0, 24, 26};

// this structure defines all the variables and events of your control interface
struct
{

  // input variables
  int8_t slider_motor_left;  // from -100 to 100
  int8_t slider_motor_right; // from -100 to 100
  int8_t joystick_x;         // from -100 to 100
  int8_t joystick_y;         // from -100 to 100
  int8_t slider_sensibility; // from 0 to 100

  // other variable
  uint8_t connect_flag; // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)

#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

int motorSensibility = 0;
int motorPWNRange = 255;

int motorLeft = 0;
int motorRight = 0;
int motorLeftSpeed = 0;
int motorRightSpeed = 0;

// Variables to store joystick readings
int xValue = 0;
int yValue = 0;

// Variables to store motor speeds
int leftMotorSpeed;
int rightMotorSpeed;

int deadband = 10;

void motorStop(int Pin1, int Pin2)
{
  digitalWrite(Pin1, LOW);
  digitalWrite(Pin2, LOW);
}

void motorForward(int Pin1, int Pin2, int moveSpeed)
{
  analogWrite(Pin1, abs(moveSpeed));
  digitalWrite(Pin2, LOW);
}

void motorBackward(int Pin1, int Pin2, int moveSpeed)
{
  digitalWrite(Pin1, LOW);
  analogWrite(Pin2, abs(moveSpeed));
}

void motorControl()
{
  if (motorLeftSpeed > 0)
  {
    motorForward(MOTOR1_PIN1, MOTOR1_PIN2, motorLeftSpeed);
  }
  else if (motorLeftSpeed < 0)
  {
    motorBackward(MOTOR1_PIN1, MOTOR1_PIN2, motorLeftSpeed);
  }
  else
  {
    motorStop(MOTOR1_PIN1, MOTOR1_PIN2);
  }

  if (motorRightSpeed > 0)
  {
    motorForward(MOTOR2_PIN1, MOTOR2_PIN2, motorRightSpeed);
  }
  else if (motorRightSpeed < 0)
  {
    motorBackward(MOTOR2_PIN1, MOTOR2_PIN2, motorRightSpeed);
  }
  else
  {
    motorStop(MOTOR2_PIN1, MOTOR2_PIN2);
  }
}

void setup()
{
  Serial.begin(115200);

  RemoteXY_Init();

  motorStop(MOTOR1_PIN1, MOTOR1_PIN2);
  motorStop(MOTOR2_PIN1, MOTOR2_PIN2);
}

void loop()
{
  RemoteXY_Handler();

  motorSensibility = RemoteXY.slider_sensibility;
  motorPWNRange = map(motorSensibility, 0, 100, 25, 255);

  // Read joystick values
  xValue = RemoteXY.joystick_x;
  yValue = RemoteXY.joystick_y;

  // Calculate motor speeds using a tank steering algorithm
  // This approach mixes the forward/backward (Y) and turning (X) values
  leftMotorSpeed = yValue + xValue;
  rightMotorSpeed = yValue - xValue;

  // Clamp the motor speeds to the -100 to 100 range
  leftMotorSpeed = constrain(leftMotorSpeed, -100, 100);
  rightMotorSpeed = constrain(rightMotorSpeed, -100, 100);

  motorLeftSpeed = map(leftMotorSpeed, -100, 100, -motorPWNRange, motorPWNRange);
  motorRightSpeed = map(rightMotorSpeed, -100, 100, -motorPWNRange, motorPWNRange);

  // Apply a deadband to prevent motor movement when the joystick is near the center
  /* if (abs(xValue) < deadband && abs(yValue) < deadband)
  {
    motorLeftSpeed = 0;
    motorRightSpeed = 0;
  } */

  if (xValue == 0 && yValue == 0)
  {

    motorLeft = RemoteXY.slider_motor_left;
    motorRight = RemoteXY.slider_motor_right;

    motorLeftSpeed = map(motorLeft, -100, 100, -motorPWNRange, motorPWNRange);
    motorRightSpeed = map(motorRight, -100, 100, -motorPWNRange, motorPWNRange);
  }

  motorControl();
}
