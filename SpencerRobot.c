#include <stdio.h>
#include <roboticscape.h>
#include <rc_usefulincludes.h>
#include "ultrasonic.h"

int main()
{
	if(rc_initialize()){
		fprintf(stderr,"ERROR: failed to init");
	return -1;
	}

	int read;
	rc_set_state(RUNNING);

	while(rc_get_state()!=EXITING)
	{
		read = get_ultrasonic_value();
		
		printf("Measurment: %d\n",read);

		usleep(1000000);
	}
	rc_cleanup();
	return 0;
}
