
#ifndef _USB_UART_H_
    
    #define _USB_UART_H_

    #include <project.h>
        
    void usb_uart_init();    
    void usb_uart_pull();
    void usb_uart_commit(char* data);
    char usb_uart_get();
    void usb_uart_push();
    
#endif