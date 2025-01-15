from gpiozero import AngularServo
from time import sleep
from gpiozero.pins.pigpio import PiGPIOFactory

#GPIO pins on left rail counting up: 26, 19, 13, 6
#GPIO pins on right rail counting up: 21, 7, 8, 25
#GPIO pin 1 does not work for factory pin setup

#example setup for plastic servo (SG90 Tower Pro)
#servo = AngularServo(27, min_angle = 0, max_angle = 180, min_pulse_width = 0.5/1000, max_pulse_width = 2.5/1000, pin_factory = PiGPIOFactory())

#Stats of our metal Engage Servos (TS90MD 270 degrees)
ENGAGE_MIN_PULSE = 0.5/1000
ENGAGE_MAX_PULSE = 2.5/1000
ENGAGE_MIN_ANGLE = 0
ENGAGE_MAX_ANGLE = 180

#Fine tuned angles for the Engage Servos
NORTH_ENGAGE_ANGLE = 35
NORTH_DISENGAGE_ANGLE = 120 #screwed in at 180
EAST_ENGAGE_ANGLE = 35
EAST_DISENGAGE_ANGLE = 120  #screwed in at 180
SOUTH_ENGAGE_ANGLE = 145
SOUTH_DISENGAGE_ANGLE = 20  #screwed in at 0
WEST_ENGAGE_ANGLE = 145
WEST_DISENGAGE_ANGLE = 20   #screwed in at 0

#Stats of our metal Claw Servos (MG90S 360 degrees)
CLAW_MIN_PULSE = 0.5/1000
CLAW_MAX_PULSE = 2.5/1000
CLAW_MIN_ANGLE = 0
CLAW_MAX_ANGLE = 360

#Time in seconds for each move to complete
QUARTER_TURN_TIME = 0.35
HALF_TURN_TIME = 0.5
ENGAGE_TIME = 0.375

#Stores the angles of the previous servo state in the format N E S W U R D L 
previous_servo_state = [0 for _ in range(8)]

#Attaches all 8 servos to the specified GPIO Pins (not physical board numbering) 
#Angle and Pulses are set up, as well as setting the Pi's GPIO pins to their factory output
#This is so we send a hardware PWM signal rather than a software one (removes jittering)
servo_north = AngularServo(26, min_angle = ENGAGE_MIN_ANGLE, max_angle = ENGAGE_MAX_ANGLE, min_pulse_width = ENGAGE_MIN_PULSE, max_pulse_width = ENGAGE_MAX_PULSE, pin_factory = PiGPIOFactory())
servo_east =  AngularServo(19, min_angle = ENGAGE_MIN_ANGLE, max_angle = ENGAGE_MAX_ANGLE, min_pulse_width = ENGAGE_MIN_PULSE, max_pulse_width = ENGAGE_MAX_PULSE, pin_factory = PiGPIOFactory())
servo_south = AngularServo(13, min_angle = ENGAGE_MIN_ANGLE, max_angle = ENGAGE_MAX_ANGLE, min_pulse_width = ENGAGE_MIN_PULSE, max_pulse_width = ENGAGE_MAX_PULSE, pin_factory = PiGPIOFactory())
servo_west =   AngularServo(6, min_angle = ENGAGE_MIN_ANGLE, max_angle = ENGAGE_MAX_ANGLE, min_pulse_width = ENGAGE_MIN_PULSE, max_pulse_width = ENGAGE_MAX_PULSE, pin_factory = PiGPIOFactory())

servo_U = AngularServo(21, min_angle = CLAW_MIN_ANGLE, max_angle = CLAW_MAX_ANGLE, min_pulse_width = CLAW_MIN_PULSE, max_pulse_width = CLAW_MAX_PULSE, pin_factory = PiGPIOFactory())
servo_R =  AngularServo(7, min_angle = CLAW_MIN_ANGLE, max_angle = CLAW_MAX_ANGLE, min_pulse_width = CLAW_MIN_PULSE, max_pulse_width = CLAW_MAX_PULSE, pin_factory = PiGPIOFactory())
servo_D =  AngularServo(8, min_angle = CLAW_MIN_ANGLE, max_angle = CLAW_MAX_ANGLE, min_pulse_width = CLAW_MIN_PULSE, max_pulse_width = CLAW_MAX_PULSE, pin_factory = PiGPIOFactory())
servo_L = AngularServo(25, min_angle = CLAW_MIN_ANGLE, max_angle = CLAW_MAX_ANGLE, min_pulse_width = CLAW_MIN_PULSE, max_pulse_width = CLAW_MAX_PULSE, pin_factory = PiGPIOFactory())

#Function that takes in a servo string and commands the servos, void function
#servo string must be in the format n.e.s.w.U0.R0.D0.L0
#any character can be omitted although data will be lost for previous servo state
#spaces should be added between steps, the number proceeding a claw servo represents the angle
#0 for 0 deg, 1 for 90 deg, 2 for 180 deg, 3 for 270 deg, and 4 for 360 deg
def execute(servo_str: str) -> None:
	length = len(servo_str)
	start = 0; angle = None
	#current state of servo given by command initialized to all 0s
	current_servo_state = [0 for _ in range(8)]
	for i in range (length):
		#once it reaches a space, period or end of string the previous characters are the command
		#the characters index is stored in 'start' and if its a claw servo, the angle is stored in start+1
		#to convert the claws stored angle to the actual angle, the character is converted to an int and multiplied by 90
		#afterwards, the servo state is saved, 1 for engaged, 0 for disengageed, or the angle for a clas servo 
		if(servo_str[i] == ' ' or servo_str[i] == '.' or i == length-1):
			match(servo_str[start]):
				case 'N':
					servo_north.angle = NORTH_ENGAGE_ANGLE
					current_servo_state[0] = 1
				case 'n':
					servo_north.angle = NORTH_DISENGAGE_ANGLE
					current_servo_state[0] = 0
				case 'E':
					servo_east.angle = EAST_ENGAGE_ANGLE
					current_servo_state[1] = 1
				case 'e':
					servo_east.angle = EAST_DISENGAGE_ANGLE
					current_servo_state[1] = 0
				case 'S':
					servo_south.angle = SOUTH_ENGAGE_ANGLE
					current_servo_state[2] = 1
				case 's':
					servo_south.angle = SOUTH_DISENGAGE_ANGLE
					current_servo_state[2] = 0
				case 'W':
					servo_west.angle = WEST_ENGAGE_ANGLE
					current_servo_state[3] = 1
				case 'w':
					servo_west.angle = WEST_DISENGAGE_ANGLE
					current_servo_state[3] = 0
				case 'U':
					angle = (int(servo_str[start+1]))*90
					servo_U.angle = angle
					current_servo_state[4] = angle
				case 'R':
					angle = (int(servo_str[start+1]))*90
					servo_R.angle = angle
					current_servo_state[5] = angle
				case 'D':
					angle = (int(servo_str[start+1]))*90
					servo_D.angle = angle
					current_servo_state[6] = angle
				case 'L':
					angle = (int(servo_str[start+1]))*90
					servo_L.angle = angle
					current_servo_state[7] = angle
			#if the character wasnt a period then a delay is neccessary, calculated by the longest move made
			if(servo_str[i] == ' ' or i == length-1):
				#with the current model our claws only move 180 degrees maximum
				#if the delta claw movement is 180 degrees then the half turn delay will be used
				max = abs(current_servo_state[4] - servo_state[4])
				for j in range(5, 8):
					if(abs(current_servo_state[j] - servo_state[j]) > max):
							max = abs(current_servo_state[j] - servo_state[j])
				if(max == 180):
					sleep(HALF_TURN_TIME)
				#The next longest time is an engage or disengage
				#If there is a delta engage for any of the engage servos, the engage delay is used
				elif(current_servo_state[0] - servo_state[0] != 0 or current_servo_state[1] - servo_state[1] != 0 or current_servo_state[2] - servo_state[2] != 0 or current_servo_state[3] - servo_state[3] != 0):
					sleep(ENGAGE_TIME)
				#else the shortest delay is used, which is the quarter turn
				else:
					sleep(QUARTER_TURN_TIME)
				#copies over the current servo state to the previous servo state
				for k in range(8):
					servo_state[k] = current_servo_state[k]
				#hardware debugging delay used to assess and fix cube positioning as needed, comment out once finalized
				sleep(3)
			#When reaching a space or period, the next servo data is stored in the index thereafter
			start = i+1

#Void function called to return the servos to their default state, and turn them off
def move_to_default() -> None:
	execute("n.e.s.w")
	#Delay is added so the user can remove the cube before the bottom potentially returns to 0
	sleep(5)
	execute("n.e.s.w.U0.R0.D0.L0")

	servo_north.value = None
	servo_east.value  = None
	servo_south.value = None
	servo_west.value  = None

	servo_U.value = None
	servo_R.value = None
	servo_D.value = None
	servo_L.value = None
	print("Servos Powered Off!")

#This while loop is used for hardware debugging purposes only, comment out when finalized
while True:
	angle = int(input("Enter servo angle: "))
	servo = input("Enter servo character: ")
	match(servo):
		case 'N':
			servo_north.angle = angle
		case 'E':
			servo_east.angle = angle
		case 'S':
			servo_south.angle = angle
		case 'W':
			servo_west.angle = angle
		case 'U':
			servo_U.angle = angle
		case 'R':
			servo_R.angle = angle
		case 'D':
			servo_D.angle = angle
		case 'L':
			servo_L.angle = angle
		case '0':
			move_to_default()
			exit()
		case '1':
			user_str = input("Enter test string: ")
			execute(user_str)
