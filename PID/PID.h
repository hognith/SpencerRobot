#ifndef PID
#define PID

#define SAMPLE_RATE_HZ 100	// main filter and control loop speed
#define DT 0.01			// 1/sample_rate

// PID
#define KP					1.0
#define KI					0.3
#define KD					0.05
#define STEERING_INPUT_MAX  0.5

// electrical hookups
#define MOTOR_CHANNEL_L			1
#define MOTOR_CHANNEL_R			2
#define MOTOR_POLARITY_L		1
#define MOTOR_POLARITY_R		1

#define PRINTF_HZ				50

#endif	//PID