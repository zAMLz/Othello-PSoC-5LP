
#include <project.h>
#include <stdio.h>
#include "usb_uart.h"
#include "game_logic.h"

// 64 byte string to hold data.
uint8 usbDataRX[64] = {};
uint8 usbDataTX[64][64] = {};

// Holds information of size of each data in each TX buffer
uint8 usbDataTXsize[64] ={};

// New data has been recieved state
uint8 newDataRX = 0;

// New data ready to transmit, value will scale
uint8 newDataTX = 0;

// Holds how much data was captured
uint8 usbDataSize = 0;


// Initialize the USB UART
//      Set it to 5V operation

void usb_uart_init() {
    USBUART_Start(0u, USBUART_3V_OPERATION);
    while (!USBUART_GetConfiguration()){};    
    USBUART_CDC_Init();
}


// Update the subData arry
//      Check to see if there is data
//      to recieve and process that data

void usb_uart_pull() {
        
    if(USBUART_GetConfiguration()) {
        
        // If data is ready to be recieved
        if(USBUART_DataIsReady()) {
            
            // get dat data
            usbDataSize = USBUART_GetAll(usbDataRX); 
            
            // if there is anything, set the proper flags
            if(usbDataSize) {                
                newDataRX = 1;
            }
        }
    }
}


// Pull data from the usbData array
//      See if there is new data
//      If there is send it else
//      sends an n

char usb_uart_get() {
    if(newDataRX) {
        newDataRX = 0;
        return usbDataRX[0];
    }
    else
        return 0;
}


// Commits data into the TX Buffers
void usb_uart_commit(char* data) {
    int index = 0;
    
    for (index = 0; index < 64; index++) {
        usbDataTX[newDataTX][index] = data[index];
        if(data[index] == '\0')
            break;
    }
    
    if(index) {
        usbDataTXsize[newDataTX] = index;
        if(newDataTX < 64)
            newDataTX++;
    }
}

// Send data from the usbData array
//      See if there is data to
//      transmit. If there is,
//      Send the data back to
//      the host.

void usb_uart_push() {
    uint8 count = 0;
    
    for (count = 0; count < newDataTX; count++) {
        
        while (!USBUART_CDCIsReady()) {}
        USBUART_PutData(usbDataTX[count],usbDataTXsize[count]);
        
        if(usbDataTXsize[count] == 64) {
            while (!USBUART_CDCIsReady()) {}
            USBUART_PutData(NULL, 0u);
        } 
    }
    newDataTX = 0;
}