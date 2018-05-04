#include <rc_usefulincludes.h>
#include <roboticscape.h>

#include "PID.h"

void balance_controller();
void* printf_loop(void* ptr);

rc_filter_t D1;
rc_imu_data_t imu_data;

float theta;
float d_u;


int main(){

	if(rc_initialize()){
		fprintf(stderr,"ERROR: failed to run rc_initialize(), are you root?\n");
		return -1;
	}

	rc_set_led(RED,1);
	rc_set_led(GREEN,0);
	rc_set_state(UNINITIALIZED);

	D1=rc_empty_filter();

	// set up D1 PID controller
	if(rc_pid_filter(&D1, KP, KI, KD, 4*DT, DT)){
		fprintf(stderr,"ERROR in rc_balance, failed to make steering controller\n");
		return -1;
	}
	rc_enable_saturation(&D1, -STEERING_INPUT_MAX, STEERING_INPUT_MAX);

	// start printf_thread if running from a terminal
	// if it was started as a background process then don't bother
	if(isatty(fileno(stdout))){
		pthread_t  printf_thread;
		
		pthread_create(&printf_thread, NULL, printf_loop, (void*) NULL);
	}

	// set up IMU configuration
	rc_imu_config_t imu_config = rc_default_imu_config();
	imu_config.dmp_sample_rate = SAMPLE_RATE_HZ;
	imu_config.orientation = ORIENTATION_X_UP;

	// start imu
	if(rc_initialize_imu_dmp(&imu_data, imu_config)){
		fprintf(stderr,"ERROR: can't talk to IMU, all hope is lost\n");
		rc_blink_led(RED, 5, 5);
		return -1;
	}

	rc_set_imu_interrupt_func(&balance_controller);

	rc_set_state(RUNNING);

	// chill until something exits the program
	while(rc_get_state()!=EXITING){
		rc_usleep(10000);
	}
	
	// cleanup
	rc_free_filter(&D1);
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
	static int inner_saturation_counter = 0; 
	float dutyL, dutyR;
	/******************************************************************
	* STATE_ESTIMATION
	* read sensors and compute the state when either ARMED or DISARMED
	******************************************************************/
	// angle theta is positive in the direction of forward tip around X axis
	theta = imu_data.dmp_TaitBryan[TB_PITCH_X]; 
	
	
	/*************************************************************
	* check for various exit conditions AFTER state estimate
	***************************************************************/
	if(rc_get_state()==EXITING){
		rc_disable_motors();
		return;
	}
	
	d_u = rc_march_filter(&D3,theta);
	
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
		new_rc_state = rc_get_state();
		// check if this is the first time since being paused
		if(new_rc_state==RUNNING && last_rc_state!=RUNNING){
			printf("\nRUNNING: Hold upright to balance.\n");
			printf("    θ    |");
			printf("    d_y  |");
			printf("\n");
		}
		else if(new_rc_state==PAUSED && last_rc_state!=PAUSED){
			printf("\nPAUSED: press pause again to start.\n");
		}
		last_rc_state = new_rc_state;
		
		// decide what to print or exit
		if(new_rc_state == RUNNING){	
			printf("\r");
			printf("%7.3f  |", theta);
			printf("%7.3f  |", d_u);
			
			fflush(stdout);
		}
		rc_usleep(1000000 / PRINTF_HZ);
	}
	return NULL;
} 