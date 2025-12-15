#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "controller.h"

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdint.h>

#define INPUT_DEVICE "/dev/input/event13"  

#define EV_KEY 1
#define EV_ABS 3
#define EV_SYN 0

static int input_fd = -1;

int gpio_init() {
    printf("Initializing USB controller...\n");
    
    input_fd = open(INPUT_DEVICE, O_RDONLY | O_NONBLOCK);
    if (input_fd < 0) {
        perror("Failed to open input device");
        return 0;
    }
    
    printf("USB controller initialized successfully\n");
    return 1;
}

uint16_t read_controller() {
    static uint16_t current_buttons = 0;
    struct input_event ev;
    
    while (read(input_fd, &ev, sizeof(ev)) > 0) {
        if (ev.type == EV_KEY) {
            int button_bit = -1;
            
            switch(ev.code) {
                case 288:  button_bit = BTN_X; break;       // X button
                case 289:  button_bit = BTN_A; break;       // A button
                case 290:  button_bit = BTN_B; break;       // B button
                case 291:  button_bit = BTN_Y; break;       // Y button
                case 292:  button_bit = BTN_L; break;       // L button
                case 293:  button_bit = BTN_R; break;       // R button
                case 296:  button_bit = BTN_SELECT; break;  // SELECT
                case 297:  button_bit = BTN_START; break;   // START
                default:
                    break;
            }
            
            if (button_bit >= 0) {
                if (ev.value) {
                    current_buttons |= (1 << button_bit);   
                } else {
                    current_buttons &= ~(1 << button_bit);
                }
            }
        }
    }
    
    return current_buttons;
}

int button_pressed(uint16_t state, int button) {
    return (state >> button) & 1;
}

#endif
