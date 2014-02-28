#include "CB1.h"

class CB1 : public IterativeRobot
{
public:
	CB1(void)	{
		
		frontLeft = new Victor(1);												//New Motors
		backLeft = new Victor(2);
		frontRight = new Victor(3);
		backRight = new Victor(4);
		shooterFront = new Victor(5);
		shooterBack = new Victor(6); 
		tiltShooter = new Victor(7);

		drive = new RobotDrive(frontLeft, backLeft, frontRight, backRight);		//Drive Train
		
		discPush = new DoubleSolenoid(1,2);										//Pneumatics
		pumpAir	= new Compressor(14,8);
		
		ds = DriverStation::GetInstance();										//Driver Station
		dsLCD = DriverStationLCD::GetInstance();
		priorPacketNumber = 0;
		dsPacketsReceivedInCurrentSecond = 0;

		rightEncoder = new AugmentedEncoder(4, 5, d_p_r / t_p_r, false);		//Encoders
		leftEncoder = new AugmentedEncoder(6, 7, d_p_r / t_p_r, true);
		shooterEncoder = new AugmentedEncoder(8, 9, d_p_r / t_p_r, false);
		tiltEncoder = new AnalogChannel(1, 1);
		
		autotimer	=	new Timer();											//Timers
		blinktimer	=	new Timer();
		teletimer	=	new Timer();
		
		rightStick = new Joystick(1);											//Controls
		leftStick = new Joystick(2);
		pilotPad = new LogitechGamepad(3);
		
		shooterState = SHOOTEROFF;								//Values
		pistonState = PISTONO;
		
		autoPeriodicLoops = 0;
		disabledPeriodicLoops = 0;
		telePeriodicLoops = 0;

		rightD = 0.0;
		leftD = 0.0;
		rightV = 0.0;
		leftV = 0.0;
		shooterV = 0.0;
		
		pumpAir->Start();														//Compressor Start

		leftEncoder->Start();													//Encoders Start
		rightEncoder->Start();
		shooterEncoder->Start();
		
		autonMode = 2;
		lThrottleValue = 0;
		shootDelay = 0;
		
		GetWatchdog().SetExpiration(50);										//50ms Timeout
	}
	
	
	/********************************** Init Routines *************************************/

	void RobotInit(void) {
		Write2LCD();
	}
	
	void DisabledInit(void) {
		disabledPeriodicLoops = 0;
		
		gameMode = DISABLEDMODE;
	}

	void AutonomousInit(void) {
		autoPeriodicLoops = 0;

		autotimer->Reset();
		
		gameMode = AUTOMODE;
	}

	void TeleopInit(void) {
		telePeriodicLoops = 0;
		dsPacketsReceivedInCurrentSecond = 0;

		discPush->Set(PUSHIN);
		
		gameMode = TELEMODE;
		
		teletimer->Reset();
		teletimer->Start();
		
		rightEncoder->Reset();
		leftEncoder->Reset();
		shooterEncoder->Reset();
	}

	/********************************** Periodic Routines *************************************/
	
	void DisabledPeriodic(void)  {
		GetWatchdog().Feed();
		
		disabledPeriodicLoops++;
		
		if ((disabledPeriodicLoops % 4) == 0) {
		
			tiltA	=	(90.0 - (180.0 * (tiltEncoder->GetAverageVoltage() - 0.5) / 4.0));
			
			lThrottleValue = leftStick->GetThrottle();
			
			shootDelay = (4 - (((rightStick->GetThrottle()) + 1) * 2)); // Allows values from 0 - 4 instead of -1 to 1
			
			Write2LCD();
		
		}
		
		if (lThrottleValue > 0) {
			autonMode = 3;
		}
		
		else if (lThrottleValue < 0) {
			autonMode = 2;
		}
	}

	void AutonomousPeriodic(void) {
		GetWatchdog().Feed();

		autoPeriodicLoops++;

		if ((autoPeriodicLoops % 4) == 0) {
			
			tiltA	=	(90.0 - (180.0 * (tiltEncoder->GetAverageVoltage() - 0.5) / 4.0));

			Write2LCD();
			
		}

		shooterBack->Set(0.65);		//.35 normal
		shooterFront->Set(1.0);
		shooterState = SHOOTERON;
		
		//Tilt!!!
		
		if (autonMode == 2) {
			
			if (tiltA <= 29.0) {
				tiltShooter->Set(-0.28);
			}
			
			else if (tiltA >= 31.0) {
				tiltShooter->Set(0.28);
			}
	
			else {
				tiltShooter->Set(0.0);
			}

			if ((tiltA >= 29.0) && (tiltA <= 31.0)) {
				autotimer->Start();
			
				aTimer = autotimer->Get();
						
				AutoPneumatics();
			}
		} //autonMode = 2
		
		else if (autonMode = 3) {
			
			if (tiltA <= 25.0) {
				tiltShooter->Set(-0.28);
			}
			
			else if (tiltA >= 27.0) {
				tiltShooter->Set(0.28);
			}
	
			else {
				tiltShooter->Set(0.0);
			}

			if ((tiltA >= 25.0) && (tiltA <= 27.0)) {
				autotimer->Start();
			
				aTimer = autotimer->Get();
						
				AutoPneumatics();
			}
		} //autonMode = 3
		
		else {
			NeutralDrive();
		}
	}

	void TeleopPeriodic(void) {
		GetWatchdog().Feed();
		
		tTimer = (int) teletimer->Get();
		
		telePeriodicLoops++;			 
		
		GetControllerIO();
		
		TelePneumatics();
		
		TeleTilt();
		
		TeleShooter();
		
		if ((telePeriodicLoops % 4) == 0) {
			shooterEncoder->Recalculate();
			
			tiltA = (90.0 - (180.0 * (tiltEncoder->GetAverageVoltage() - 0.5) / 4.0));
			
			shooterV = 60.0 * shooterEncoder->GetVelocity();

			lThrottleValue = leftStick->GetThrottle();
			
			Write2LCD();
		}
		
		if ((telePeriodicLoops % 2) == 0) {
			rightD = rightEncoder->GetDistance();
			leftD = leftEncoder->GetDistance();

			Write2LCD();
		}
				
		dsPacketsReceivedInCurrentSecond++;

		if (halfSpeed == true) {
			if (leftAxisY > .65) {
				leftAxisY = .65;
			}
			
			else {
				leftAxisY = leftAxisY;
			}
			
			if (rightAxisY > .65) {
				rightAxisY = .65;
			}
			
			else {
				rightAxisY = rightAxisY;
			}
		}
		
		else {
			leftAxisY	=	leftAxisY;
			rightAxisY	=	rightAxisY;
		}
		
		drive->TankDrive(leftAxisY, rightAxisY, true);
		drive->SetExpiration(50);
		drive->SetSafetyEnabled(true);
		
		Write2LCD();
	}


/********************************** Continuous Routines *************************************/

	/* 
	 * Not Used
	 * 
	void DisabledContinuous(void) {
	}

	void AutonomousContinuous(void)	{
	}

	void TeleopContinuous(void) {
	}
	*/

	
/********************************** Miscellaneous Routines *************************************/
		
	void Write2LCD() {
	/*	
		if (shooterState = SHOOTERON) {
			dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Shooter Motors ON!!!");
		}

		else {
			dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Shooter Motors OFF!!!");
		}
		
		if (pistonState = PISTONF) {
			dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Piston is OUT!!!");
		}
		
		else if (pistonState = PISTONB) {
			dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Piston is IN!!!");
		}
		
		else {
			dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Piston is OFF!!!");
		}
	*/	
		dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "A Tilt: %f", tiltA);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Throttle Value: %f", lThrottleValue);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "Delay: %f", shootDelay);
		
		dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Auto Mode: %i", autonMode);	
		
		dsLCD->UpdateLCD();
	}
	
	void NeutralDrive(void) {
		frontLeft->Set(0.0);
		backLeft->Set(0.0);
		frontRight->Set(0.0);
		backRight->Set(0.0);
	}

	void AutoPneumatics(void) {

		if ((aTimer > (0.0 + shootDelay)) && (aTimer < (.3 + shootDelay))) {
			discPush->Set(PUSHOUT);
			pistonState = PISTONB;
		}
		
		else if ((aTimer > (0.75 + shootDelay)) && (aTimer < (1.05 + shootDelay))) {
			discPush->Set(PUSHIN);
			pistonState = PISTONF;
		}
				
		else if ((aTimer > (1.5 + shootDelay)) && (aTimer < (1.8 + shootDelay))) {
			discPush->Set(PUSHOUT);
			pistonState = PISTONB;
		}
			
		else if ((aTimer > (2.25 + shootDelay)) && (aTimer < (2.55 + shootDelay))) {
			discPush->Set(PUSHIN);
			pistonState = PISTONF;
		}
				
		else if ((aTimer > (3.0 + shootDelay)) && (aTimer < (3.3 + shootDelay))) {
			discPush->Set(PUSHOUT);
			pistonState = PISTONB;
		}

		else if ((aTimer > (3.75 + shootDelay)) && (aTimer < (4.05 + shootDelay))) {
			discPush->Set(PUSHIN);
			pistonState = PISTONF;
		}
		
		else if ((aTimer > (4.5 + shootDelay)) && (aTimer < (4.8 + shootDelay))) {
			discPush->Set(PUSHOUT);
			pistonState = PISTONB;
		}
		
		else if ((aTimer > (5.25 + shootDelay)) && (aTimer < (5.55 + shootDelay))) {
			discPush->Set(PUSHIN);
			pistonState = PISTONF;
		}
		
		else {
			discPush->Set(PUSHOFF);
			pistonState = PISTONO;
		}
	}
	
	void TelePneumatics(void) {
		if (pushPiston == true) {
			discPush->Set(PUSHOUT);
			pistonState = PISTONF;
			
			Wait(.5);
			
			discPush->Set(PUSHIN);
			pistonState = PISTONB;

			Wait(.5);
		}
		
		else {
			discPush->Set(PUSHOFF);
			pistonState = PISTONO;
		}
	}
	
	void TeleTilt(void) {
		
		if ((tiltF == true) && (tiltB == false)) {
			
			if (tiltA <= 29.0) {
				tiltShooter->Set(-0.24);
			}

			else if (tiltA >= 31.0) {
				tiltShooter->Set(0.24);
			}

			else {
				tiltShooter->Set(0.0);
			}

		}
		
		else if ((tiltF == false) && (tiltB == true)) {
			
			if (tiltA <= 25.0) {
				tiltShooter->Set(-0.24);
			}

			else if (tiltA >= 27.0) {
				tiltShooter->Set(0.24);
			}

			else {
				tiltShooter->Set(0.0);
			}
			
		}

		else {

			if ((tiltUp == true) && (tiltDown == false)) {
				
				if (tiltA <= 42.0) {
					tiltShooter->Set(-0.28);
				}  
				
				else if (tiltA >= 44.0) {
					tiltShooter->Set(0.28);
				}
				
				else {
					tiltShooter->Set(0.0);
				}
			}
		
			else if ((tiltDown == true) && (tiltUp == false)) {
		
				if (tiltA >= 1.0) {
					tiltShooter->Set(0.28);
				}
			
				else if (tiltA <= -1.0) {
					tiltShooter->Set(-0.28);
				}
			
				else {
					tiltShooter->Set(0.0);
				}			
			}
		
			else {
				tiltShooter->Set(0.0);
			}
		}
}
	
	void TeleShooter(void) {
/*
		if (shooterOn == true && shooterOff == false) {
			shooterState = SHOOTERON;
		}
		
		else if (shooterOn == false && shooterOff == true) {
			shooterState = SHOOTEROFF;
		}
		
		else {
			shooterState = shooterState;
		}
		
		if (shooterState = SHOOTERON) {
			shooterFront->Set(1.0);
			shooterBack->Set(0.35);
		}
		
		else if (shooterState = SHOOTEROFF) {
			shooterFront->Set(0.0);
			shooterBack->Set(0.0);
		}
	*/
		
		if (shooterOn == true) {
			shooterFront->Set(1.0);
			shooterBack->Set(.65);		//.35 normal
		}
		
		else {
			shooterFront->Set(0);
			shooterBack->Set(0);
		}
	}
	
	void GetControllerIO(void){
		
		/*
		 * Left Joystick
		 */

		leftAxisY		=		-.97 * leftStick->GetY();
			
		/*
		 * Right Joystick
		 */

		rightAxisY		=		-.965 * rightStick->GetY();
		
		/*
		 * Copilot Pad
		 */
		
		pushPiston		=		pilotPad->GetNumberedButton(8);
		tiltUp			=		pilotPad->GetNumberedButton(4);
		tiltDown		=		pilotPad->GetNumberedButton(2);
		tiltF			=		pilotPad->GetNumberedButton(1);
		tiltB			=		pilotPad->GetNumberedButton(3);
		shooterOn		=		pilotPad->GetNumberedButton(5);
		halfSpeed		=		pilotPad->GetNumberedButton(7);
		
//		shooterOff		=		pilotPad->GetNumberedButton(6);
//		tilt			=		pilotPad->GetLeftY();
//		tiltL			=		pilotPad->GetLeftPush();
	}

};

START_ROBOT_CLASS(CB1);
