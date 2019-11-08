#include <stdio.h>
#include <pthread.h>

#include "brakes.h"
#include "turn.h"
// interupt handler (takes care of brake, left, right events)

void * brake(){
    printf("brake\n");
    return NULL;
}

void * turn(){
    printf("turn\n");
    return NULL;
}

int main(){
	//set up interupt handler

	//set up brake sensing (accelerometer)
	//set up left turn sensing
	//set up right turn sensing
	//set up LED display
	//set up left signal
	//set up right signal
	//set up brake light
    pthread_t brake_thread, turn_thread;

    pthread_create(&brake_thread,NULL, brake, NULL);
    pthread_create(&turn_thread, NULL, turn, NULL);
    pthread_join(brake_thread, NULL);
    pthread_join(turn_thread, NULL);
    printf("done\n");
}
