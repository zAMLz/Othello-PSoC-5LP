
#include <project.h>
#include <stdio.h>
#include "led_matrix.h"
#include "othello_shell.h"
#include "game_logic.h"
#include "usb_uart.h"
#include "packet_com.h"

int main()
{
    
    // Enable Global Interrupts
    CyGlobalIntEnable;
    
    // Start Devices
    LCD_Start();
    
    // Start Init Functions (look at respective header files)
    game_board_reset();
    led_matrix_init();
    usb_uart_init();
    packet_com_init();
    
    // Main Program Loop
    for(;;) {
        
        // Get updates from the Computer Host
        usb_uart_pull();
        
        // Update the current game state
        shell_update();
        
        // Send Feedback to the Computer Host
        usb_uart_push();
    }
    
}