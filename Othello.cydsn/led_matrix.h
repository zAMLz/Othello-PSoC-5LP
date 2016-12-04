
#ifndef _LED_MATRIX_H_
    
    #define _LED_MATRIX_H_
    
    #include <project.h>
        
    #define LED_ROW_SIZE 8
    #define LED_COL_SIZE 32
        
    #define NO_COLOR 0
        
    CY_ISR_PROTO(LED_Update);

    void led_matrix_init();
    
#endif
