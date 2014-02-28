#ifndef CB1_H_
#define CB1_H_

#include "WPILib.h"
#include "LogitechGamepad.h"
#include "TractionControl.h"

/*
 * Definitions
 */
#define PUSHOUT			DoubleSolenoid::kForward
#define PUSHIN			DoubleSolenoid::kReverse
#define PUSHOFF			DoubleSolenoid::kOff
#define ONETIME			Encoder::k1X

/*
 * Drive Train
 */
RobotDrive *drive;

Victor *frontLeft;
Victor *backLeft;
Victor *frontRight;
Victor *backRight;
Victor *tiltShooter;
Victor *shooterBack;
Victor *shooterFront;

/*
 * Driver Station
 */
DriverStation *ds;						// driver station object
DriverStationLCD *dsLCD;
UINT32 priorPacketNumber;					// keep track of the most recent packet number from the DS
UINT8 dsPacketsReceivedInCurrentSecond;	// keep track of the ds packets received in the current second

/*
 * Encoders
 */
AugmentedEncoder *rightEncoder;
AugmentedEncoder *leftEncoder;
AugmentedEncoder *shooterEncoder;
AnalogChannel *tiltEncoder;
float d_p_r = 18.8496;					//Wheel diameter x pi - 6" x pi = 18.8496
float t_p_r = 360;						//360 tick per rev encoder
float rightD;
float leftD;
float rightV;
float leftV;
float shooterV;
float tiltA;
float tilt;

/*
 * Pneumatics
 */
Compressor *pumpAir;
DoubleSolenoid *discPush;

/*
 * Timers
 */
Timer *autotimer;
Timer *blinktimer;
Timer *teletimer;
float aTimer;
int bTimer;
int tTimer;

/*
 * Controls
 */
Joystick *rightStick;
Joystick *leftStick;
LogitechGamepad *pilotPad;
bool pushPiston;
bool pushOut;
bool shooterOff;
bool shooterOn;
bool tiltUp;
bool tiltDown;
bool tiltL;
bool tiltF;
bool tiltB;
float leftAxisY;
float rightAxisY;

bool halfSpeed;

/*
 * Miscellaneous
 */
typedef enum {AUTOMODE, DISABLEDMODE, TELEMODE} GameMode;
typedef enum {SHOOTERON, SHOOTEROFF} ShooterState;
typedef enum {PISTONF, PISTONB, PISTONO} PistonState;

GameMode gameMode;
ShooterState shooterState;
PistonState pistonState;

UINT32 autoPeriodicLoops;
UINT32 disabledPeriodicLoops;
UINT32 telePeriodicLoops;	

int autonMode;
float lThrottleValue;
float shootDelay;

#endif
