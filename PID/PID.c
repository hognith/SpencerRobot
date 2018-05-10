#include <rc_usefulincludes.h>
#include <roboticscape.h>

#include "ultrasonic.h"
#include "PID.h"

void balance_controller();
void* printf_loop(void* ptr);
void* scanf_loop(void* ptr);
void Spencer_fine_tuneing();
void Spencer_turn_left(int turn_angle);

rc_filter_t D1;
//rc_filter_t D2;
rc_imu_data_t imu_data;

float theta;
float d_u;
float d_e;
float setpoint = 0.0;
int input;
float turnL = 1.0;
float turnR = 1.0;
float Lencoder;
float Rencoder;
int ultra;
int state = 0;
int turn_angle = 0;

int main(){

	if(rc_initialize()){
		fprintf(stderr,"ERROR: failed to run rc_initialize(), are you root?\n");
		return -1;
	}

	rc_set_led(RED,1);
	rc_set_led(GREEN,0);
	rc_set_state(UNINITIALIZED);

	Init_ultrasonic();

	D1 = rc_empty_filter();

	// set up D1 PID controller
	if(rc_pid_filter(&D1, KP, KI, KD, 4*DT, DT)){
		fprintf(stderr,"ERROR in rc_balance, failed to make imu controller\n");
		return -1;
	}
	rc_enable_saturation(&D1, -STEERING_INPUT_MAX, STEERING_INPUT_MAX);

/*
	D2 = rc_empty_filter();

	// set up D2 PID controller
	if(rc_pid_filter(&D2, KP2, KI2, KD2, 4*DT, DT)){
		fprintf(stderr,"ERROR in rc_balance, failed to make encouder controller\n");
		return -1;
	}
	rc_enable_saturation(&D2, (1 - turnrate_MAX), (1 + turnrate_MAX));
*/

	// start printf_thread if running from a terminal
	// if it was started as a background process then don't bother
	if(isatty(fileno(stdout))){
		pthread_t  printf_thread;
		pthread_create(&printf_thread, NULL, printf_loop, (void*) NULL);

		pthread_t scanf_thread;
		pthread_create(&scanf_thread, NULL, scanf_loop, (void*) NULL);
	}

	// set up IMU configuration
	rc_imu_config_t imu_config = rc_default_imu_config();
	imu_config.dmp_sample_rate = SAMPLE_RATE_HZ;
	imu_config.orientation = ORIENTATION_Z_UP;

	// start imu
	if(rc_initialize_imu_dmp(&imu_data, imu_config)){
		fprintf(stderr,"ERROR: can't talk to IMU, all hope is lost\n");
		rc_blink_led(RED, 5, 5);
		return -1;
	}

	rc_enable_motors();

	rc_set_encoder_pos(ENCODER_CHANNEL_L,0);
	rc_set_encoder_pos(ENCODER_CHANNEL_R,0);

	rc_set_imu_interrupt_func(&balance_controller);

	rc_set_state(RUNNING);

	// chill until something exits the program
	while(rc_get_state()!=EXITING){
		rc_usleep(10000);
	}
	
	// cleanup
	rc_free_filter(&D1);
	//rc_free_filter(&D2);
	rc_power_off_imu();
	rc_cleanup();
	return 0;
}

/*******************************************************************************
* void balance_controller()
*
* discrete-time balance controller operated off IMU interrupt
* Called at SAMPLE_RATE_HZ
*******************************************************************************/
void balance_controller(){
	//float setpoint = 0.1;
	//static int inner_saturation_counter = 0; 
	//float dutyL, dutyR;
	/******************************************************************
	* STATE_ESTIMATION
	* read sensors and compute the state when either ARMED or DISARMED
	******************************************************************/
	// angle theta is positive in the direction of forward tip around X axis
	
	theta = imu_data.dmp_TaitBryan[TB_PITCH_X] + cape_mount_angle; 

	/*************************************************************
	* check for various exit conditions AFTER state estimate
	***************************************************************/
	if(rc_get_state()==EXITING){
		rc_disable_motors();
		return;
	}
	
	Spencer_fine_tuneing();

	
	Lencoder = rc_get_encoder_pos(ENCODER_CHANNEL_L);
	Rencoder = rc_get_encoder_pos(ENCODER_CHANNEL_R);

	float error1 = setpoint + theta;
	//turnrate setpoint = 1;
	//float error2 = fabs(Lencoder - Rencoder);

	d_u = rc_march_filter(&D1, error1);
	//d_e = rc_march_filter(&D2, error2);
	
	if(state == 0){
		rc_set_motor(MOTOR_CHANNEL_L, MOTOR_POLARITY_L * d_u * turnL); 
		rc_set_motor(MOTOR_CHANNEL_R, MOTOR_POLARITY_R * d_u * turnR);
	}
	else if(state == 1){
		rc_set_encoder_pos(ENCODER_CHANNEL_L, 0);
		rc_set_encoder_pos(ENCODER_CHANNEL_R, 0);
		while(Rencoder <= 3000){
			Lencoder = rc_get_encoder_pos(ENCODER_CHANNEL_L);
			Rencoder = rc_get_encoder_pos(ENCODER_CHANNEL_R);
			rc_set_motor(MOTOR_CHANNEL_L, MOTOR_POLARITY_L * 0.0);
			rc_set_motor(MOTOR_CHANNEL_R, MOTOR_POLARITY_R * 0.3);
		}
		state = 0;
	}
	return;
}

/*******************************************************************************
* printf_loop() 
*
* prints diagnostics to console
* this only gets started if executing from terminal
*******************************************************************************/
void* printf_loop(void* ptr){
	rc_state_t last_rc_state, new_rc_state; // keep track of last state 
	last_rc_state = rc_get_state();
	while(rc_get_state()!=EXITING){
		ultra = get_ultrasonic_value();

		new_rc_state = rc_get_state();
		// check if this is the first time since being paused
		if(new_rc_state==RUNNING && last_rc_state!=RUNNING){
			printf("\nRUNNING: Hold upright to balance.\n");
			printf("     θ     |");
			printf("     d_y   |");
			printf(" Setpoint  |");
			printf("L encoder  |");
			printf("R encoder  |");
			printf("Ultrasonic |");
			printf("\n");
		}
		else if(new_rc_state==PAUSED && last_rc_state!=PAUSED){
			printf("\nPAUSED: press pause again to start.\n");
		}
		last_rc_state = new_rc_state;
		
		// decide what to print or exit
		if(new_rc_state == RUNNING){	
			printf("\r");
			printf("%8.4f   |", theta);
			printf("%8.4f   |", d_u);
			printf("%8.4f   |", setpoint);
			printf("%8.4f   |", Lencoder);
			printf("%8.4f   |", Rencoder * -1);
			printf("%d      |",ultra);
			fflush(stdout);
		}

		rc_usleep(1000000 / PRINTF_HZ);
	}
	return NULL;
} 

void* scanf_loop(void* ptr){
	rc_state_t last_rc_state, new_rc_state; // keep track of last state 
	last_rc_state = rc_get_state();
	char buf[BUFSIZ];
	char *p;
	while(rc_get_state()!=EXITING){
		new_rc_state = rc_get_state();
		// check if this is the first time since being paused
		
		if(new_rc_state==RUNNING && last_rc_state!=RUNNING){
			//printf("\nTaking Input:");
			//input = getchar();
		}
		else if(new_rc_state==PAUSED && last_rc_state!=PAUSED){
			printf("\nPAUSED: press pause again to start.\n");
		}
		last_rc_state = new_rc_state;
		
		// decide what to print or exit
		if(new_rc_state == RUNNING){
			if(fgets(buf, sizeof(buf), stdin) != NULL){
				if((p = strchr(buf, '\n')) != NULL)
					*p = '\0';
			}
			input = atoi(buf);
			
		}

		rc_usleep(1000000 / PRINTF_HZ);
	}
	return NULL;
}
void Spencer_fine_tuneing()
{
	
	if(input == 8){
		setpoint -= 0.0005;
		input = 0;
	}
	if(input == 2){
		setpoint += 0.0005;
		input = 0;
	}
	if(input == 5){
		setpoint = 0.0;
		turnL = 1.0;
		turnR = 1.0;
		input = 0;
	}
	if(input == 4){

		Spencer_turn_left(45);
		//turnL = 0.90;
		//turnR = 1.1;
		//input = 0;
	}
	if(input == 6){
		turnL = 1.1;
		turnR = 0.90;
		input = 0;
	}
	if(input == 9){
		state = 1;
		//turnR = 1.5;
		//turnL = 0.5;
		input = 0;
	}
	if(input == 7){
		turnL = 2;
		input = 0;
	}

}
void Spencer_turn_left(int turn_angle)
{
	int tikk = 8 * turn_angle;

	while((-1)*Lencoder < tikk)
	{	

		rc_set_motor(MOTOR_CHANNEL_L, MOTOR_POLARITY_L * 1.0);
		rc_set_motor(MOTOR_CHANNEL_R, MOTOR_POLARITY_L * 1.0);	

	}

}

