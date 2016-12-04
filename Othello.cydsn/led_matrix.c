
#include <stdio.h>
#include <project.h>
#include "led_matrix.h"
#include "game_logic.h"

uint8 i_led = 0;
uint8 j_led = 0;

// Initialize the LED Matrix
//      link the Interrupt to
//      its respective function
//      and start the timer.

void led_matrix_init() {
    timerIsr_StartEx(LED_Update);
    LED_Update_Timer_Start();
}


// Update the LED Matrix
//      Retrieve data from some
//      function that will process
//      and return some color data
//      ( get_board_data() )

CY_ISR(LED_Update){
    
    // Reset the row and col positions
    i_led = 0;
    j_led = 0;
    
    // Start the row loop
    for(i_led = 0; i_led < LED_ROW_SIZE; i_led++) {
        
        // Turn off the Display and write the row
        OE_Write(1);
        CBA_Row_Write(i_led);
        
        // Start the Column loop
        for(j_led = 0; j_led < LED_COL_SIZE; j_led++)
        {
            
            // Get the color data from some function
            // the function get_board_data handles
            // every pixel on the matrix.
            RGB_1_Write(get_board_data(i_led, j_led));
            RGB_2_Write(get_board_data(i_led + 8, j_led));
                
            // Shift in the bit via a clock pulse
            CLK_Write(1);
            CLK_Write(0);

        }
        
        // Pulse the Latch
        LAT_Write(1);
        LAT_Write(0);
        
        // Turn on the Display and a bit of delay
        OE_Write(0);
        CyDelay(1);
    }
    
    // Clear the interrupt
    LED_Update_Timer_ReadStatusRegister();
}