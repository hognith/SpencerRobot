#ifndef PID
#define PID

#define SAMPLE_RATE_HZ 100	// main filter and control loop speed
#define DT 0.01			// 1/sample_rate
#define cape_mount_angle 0.005 //0.037

// PID
#define KP					6.0 //10.0
#define KI					70.0 //200.0
#define KD					0.35  //0.4
#define STEERING_INPUT_MAX  1.0

// PID encoders
#define KP2					0.5
#define KI2					0.0
#define KD2					0.0
#define turnrate_MAX  		0.2

// electrical hookups
#define MOTOR_CHANNEL_L			1
#define MOTOR_CHANNEL_R			2
#define MOTOR_POLARITY_L		1
#define MOTOR_POLARITY_R		1

#define ENCODER_CHANNEL_L		1
#define ENCODER_CHANNEL_R		2
#define ENCODER_POLARITY_L		1
#define ENCODER_POLARITY_R		-1

#define PRINTF_HZ				50

#endif	//PID
