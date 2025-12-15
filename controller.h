#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

// Button bit positions
#define BTN_B 0
#define BTN_Y 1
#define BTN_SELECT 2
#define BTN_START 3
#define BTN_UP 4
#define BTN_DOWN 5
#define BTN_LEFT 6
#define BTN_RIGHT 7
#define BTN_A 8
#define BTN_X 9
#define BTN_L 10
#define BTN_R 11

#ifdef __linux__
int gpio_init();
uint16_t read_controller();
int button_pressed(uint16_t state, int button);
void print_controller_state(uint16_t state);
#else
static inline int gpio_init(void) { return 0; }
static inline uint16_t read_controller(void) { return 0; }
static inline int button_pressed(uint16_t state, int button) { return 0; }
static inline void print_controller_state(uint16_t state) { }
#endif

#endif
