// Virtual timer implementation

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nrf.h"

#include "virtual_timer.h"
#include "virtual_timer_linked_list.h"

// This is the interrupt handler that fires on a compare event
void TIMER4_IRQHandler(void) {
  // This should always be the first line of the interrupt handler!
  // It clears the event so that it doesn't happen again
  NRF_TIMER4->EVENTS_COMPARE[4] = 0;

  // Place your interrupt handler code here
  __disable_irq();
  // printf("Timer fired!\n");
  // list_print();
  // node_t* my_node = malloc(sizeof(node_t));


  while (list_get_first()->timer_value <= read_timer()) {
    node_t* my_node = list_remove_first();
    // printf("timer: %d\n", read_timer());
    my_node->cb();
    if (my_node->repeated) {
      my_node->timer_value += my_node->interval;
      // printf("timer value: %d\n", my_node->timer_value);
      list_insert_sorted(my_node);
      // printf("inserted here\n");
      // list_print();
    } else {
      free(my_node);
    }
    //my_node = list_remove_first();
    // printf("new node: %d\n", my_node->timer_value);
    // printf("timer at end of while%d\n", read_timer());
  }

  if (list_get_first() == NULL) {
    // printf("get first is null");
    NRF_TIMER4->CC[4] = 0;
  } else {
    // printf("new timer value:%d\n", list_get_first()->timer_value);
    NRF_TIMER4->CC[4] = list_get_first()->timer_value;
  }
  // list_print();
  // free(my_node);
  __enable_irq();
}

// Read the current value of the timer counter
uint32_t read_timer(void) {
  NRF_TIMER4->TASKS_CAPTURE[1] = 1;
  // Should return the value of the internal counter for TIMER4
  return NRF_TIMER4->CC[1];
}

// Initialize TIMER4 as a free running timer
// 1) Set to be a 32 bit timer
// 2) Set to count at 1MHz
// 3) Enable the timer peripheral interrupt (look carefully at the INTENSET register!)
// 4) Clear the timer
// 5) Start the timer
void virtual_timer_init(void) {
  // Place your timer initialization code here
  NRF_TIMER4->BITMODE |= 3;
  NRF_TIMER4->PRESCALER |= (1 << 2);
  NRF_TIMER4->INTENSET |= (1 << 20);
  NVIC_EnableIRQ(TIMER4_IRQn);
  NRF_TIMER4->TASKS_CLEAR |= 1;
  NRF_TIMER4->TASKS_START |= 1;
}

// Start a timer. This function is called for both one-shot and repeated timers
// To start a timer:
// 1) Create a linked list node (This requires `malloc()`. Don't forget to free later)
// 2) Setup the linked list node with the correct information
//      - You will need to modify the `node_t` struct in "virtual_timer_linked_list.h"!
// 3) Place the node in the linked list
// 4) Setup the compare register so that the timer fires at the right time
// 5) Return a timer ID
//
// Your implementation will also have to take special precautions to make sure that
//  - You do not miss any timers
//  - You do not cause consistency issues in the linked list (hint: you may need the `__disable_irq()` and `__enable_irq()` functions).
//
// Follow the lab manual and start with simple cases first, building complexity and
// testing it over time.
static uint32_t timer_start(uint32_t microseconds, virtual_timer_callback_t cb, bool repeated) {
  __disable_irq();
  node_t* my_node = malloc(sizeof(node_t));
  my_node->cb = cb;
  my_node->timer_value = read_timer() + microseconds;
  my_node->interval = microseconds;
  my_node->repeated = repeated;
  list_insert_sorted(my_node);
  NRF_TIMER4->CC[4] = list_get_first()->timer_value;

  __enable_irq();
  // Return a unique timer ID. (hint: What is guaranteed unique about the timer you have created?)
  return (uint32_t) my_node;
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start(uint32_t microseconds, virtual_timer_callback_t cb) {
  return timer_start(microseconds, cb, false);
}

// You do not need to modify this function
// Instead, implement timer_start
uint32_t virtual_timer_start_repeated(uint32_t microseconds, virtual_timer_callback_t cb) {
  return timer_start(microseconds, cb, true);
}

// Remove a timer by ID.
// Make sure you don't cause linked list consistency issues!
// Do not forget to free removed timers.
void virtual_timer_cancel(uint32_t timer_id) {
  node_t* curr_node = list_get_first();
  while ((uint32_t) curr_node != timer_id) {
    if (curr_node->next == NULL) {
      printf("Timer with this ID does not exist!\n");
      return;
    } else {
      curr_node = curr_node->next;
    }
  }

  list_remove(curr_node);
  free(curr_node);
  printf("Timer cancelled!\n");

  if (list_get_first() == NULL) {
    NRF_TIMER4->CC[4] = 0;
  } else {
    NRF_TIMER4->CC[4] = list_get_first()->timer_value;
  }
}
