#ifndef PID
#define PID

#define SAMPLE_RATE_HZ 100	// main filter and control loop speed
#define DT 0.01			// 1/sample_rate
#define cape_mount_angle -0.008 //0.037

// PID
#define KP					13.0 //10.0
#define KI					300.0 //200.0
#define KD					0.2  //0.4
#define STEERING_INPUT_MAX  1.0

// electrical hookups
#define MOTOR_CHANNEL_L			1
#define MOTOR_CHANNEL_R			2
#define MOTOR_POLARITY_L		1
#define MOTOR_POLARITY_R		1

#define PRINTF_HZ				50

#endif	//PID
