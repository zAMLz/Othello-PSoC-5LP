
#include <project.h>
#include <stdio.h>
#include "sd_card.h"
#include "FS.h"
#include "usb_uart.h"

uint8 sd_success = FALSE;

// File to Open
char sd_file_name[9] = "File.txt";

// SD Card Volume Name
char sd_card_vol_name[10] = {};

// Current Open File
// You can only modify one file at a time.
FS_FILE * pFile;


// SD Card init code
void sd_card_init() {
    
    // Initialize the files system
    FS_Init();
    sd_success = TRUE;
}

void sd_card_deinit() {
    
    // Deinitialze the file system
    FS_DeInit();
    sd_success = FALSE;
}

// File Append function for the sd card
void sd_card_file_append(char *file_name, char *str_data) {
    
    int str_size = 0;
    
    while(str_data[str_size] != '\0')
        str_size++;
    
    if(sd_success) {
        
        // Load file into the file pointer
        pFile = FS_FOpen(file_name, "a");
        
        // If it was successful,
        if(pFile) {
            usb_uart_commit("Successfully opened ");
            usb_uart_commit(file_name);
            usb_uart_commit("!\r");
            
            // Append data to the file
            if(FS_Write(pFile, str_data, str_size))
                usb_uart_commit("Append Succes!\r");
            
            else 
                usb_uart_commit("Append Fail1\r");
            
            // Close the file 
            if(FS_FClose(pFile))
                usb_uart_commit("Failed to close file!\r");
            else
                usb_uart_commit("File successfully closed!\r");
        }
        else {
            usb_uart_commit("Unable to create\\open ");
            usb_uart_commit(file_name);
            usb_uart_commit("!\r");
        }
        
    }
}

// File Write function for the sd card
void sd_card_file_write(char *file_name, char *str_data) {
    
    int str_size = 0;
    
    while(str_data[str_size] != '\0')
        str_size++;
    
    if(sd_success) {
        
        // Load file into the file pointer
        pFile = FS_FOpen(file_name, "w");
        
        // If it was successful,
        if(pFile) {
            usb_uart_commit("Successfully opened ");
            usb_uart_commit(file_name);
            usb_uart_commit("!\r");
            
            // Append data to the file
            if(FS_Write(pFile, str_data, str_size))
                usb_uart_commit("Append Succes!\r");
            
            else 
                usb_uart_commit("Append Fail1\r");
            
            // Close the file 
            if(FS_FClose(pFile))
                usb_uart_commit("Failed to close file!\r");
            else
                usb_uart_commit("File successfully closed!\r");
        }
        else {
            usb_uart_commit("Unable to create\\open ");
            usb_uart_commit(file_name);
            usb_uart_commit("!\r");
        }
        
    }
}