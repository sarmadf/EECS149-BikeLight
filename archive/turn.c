#include <stdio.h>
#include <pthread.h>

#include "states.h"
#include "turn.h"

void * right_turn(void * state){
    while((int)state != 0){
        printf("right turn %d\n", (int)state);
        state--;
    }
    return NULL;
}

void * left_turn(void * state){
    while((int)state != 0){
        printf("left turn %d\n", (int)state);
        state--;
    }
    return NULL;
}

void * track_turning(){
//    enum turn left = noTurn;
//    enum turn right = noTurn;

    int left = 5;
    int right = 5;

    pthread_t left_thread, right_thread;

    pthread_create(&left_thread, NULL, left_turn, left); //left
    pthread_create(&right_thread, NULL, right_turn, right); //right
    pthread_join(left_thread, NULL);
    pthread_join(right_thread, NULL);
    return NULL;
}