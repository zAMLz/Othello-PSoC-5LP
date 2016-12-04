
#include <project.h>
#include <stdio.h>
#include "usb_uart.h"
#include "othello_shell.h"
#include "game_logic.h"
#include "sd_card.h"
#include "packet_com.h"


// Shell Command String and arg string (arg string will contain command as well)
char cmd[64] = {};
char arg[6][64] = {};

// Shell Buffer Size
int cmd_count = 0;
int arg_count = 0;

// Shell commands
enum SHELL_COMMANDS {
    
    // Menu Control Commands
    Reset = 1,
    Help,
    Pvp,
    Pve,
    Avp,
    Ave,
    Clear,
    
    // Game control Options
    SDCard,
    
    // Network control Options
    Advertise,
    Connect,
    Disconnect,
    Hash,
    Bsize,
    
    // ALIASES
    A,
    Ip
    
} shell_cmd;

// Global Variable for deciding if we allow SD writes to happen
// or to not happen.
uint8 save_replay = FALSE;



// By the will of the global state machine of the game
void shell_update() {
    uint8 command = usb_uart_get();
    
    switch(board_state) {
            
        case MENU: 
            //game_menu_update_OUTDATED(command);
            command_update(command);
            break;
            
        case MENU_ADVERT:
            advert_stop(command);
            break;
            
        case ADVERT1:
            advert_stop(command);
            break;
            
        case ADVERT2:
            advert_stop(command);
            break;
        
        case PVP:
            game_logic_update(command);            
            break;
        
        case PVE:
            game_logic_update_pve(command);
            break;
        
        case AVP:
            board_state = MENU;
            break;
        
        case AVE:
            board_state = MENU;
            break;
            
        case END:
            if(command == 'R') {
                usb_uart_commit("Game has been reset!\r\r\r");
                game_board_reset();
            }
            break;           
        
        default: 
            break;
    }
    
}

// Stops the Advert state and goes back to the Menu state mode
void advert_stop(uint8 command) {
    if(command) {
        cmd_count = 0;
        board_state = MENU;
        usb_uart_commit("\r\r\r");
        flush_advert_buffer();
        command_update(command);
    }
}

// The commands available in the menu(SHELL)
// Stores actual text into the buffer, which is finally
// evaluated on the ENTER key press
void command_update(uint8 command) {
    
    // If user pressed enter or has exceeded the allotted buffer
    if(command == '\r' || cmd_count >= 63){
        
        //usb_uart_commit("\rIM OUT\r");
        
        cmd[cmd_count++] = '\0';
        cmd_count = 0;
        
        //usb_uart_commit(cmd);
        usb_uart_commit("\r");
        command_execute(command_parse());
    }
    
    // If the user presses backspace.
    else if(command == 0x08) {
        if(cmd_count > 0)
            cmd_count--;
        usb_uart_commit("<");
    }
    
    else if(command == '!') {
        usb_uart_commit("Panic Button Pressed\rGame has been reset!\r\r\r");
        game_board_reset();
    }
    
    // Otherwise if it is a valid character.
    else if(command){
        
        char cmd_instance[2];
        sprintf(cmd_instance,"%c", command);
        usb_uart_commit(cmd_instance);
        
        cmd[cmd_count++] = command;
    }
}

int command_parse() {
    
    // obtain the number of arguments while parsing each individual argument
    arg_count = sscanf(cmd, "%s %s %s %s %s %s", arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
    
    if(!strcmp("reset",arg[0]))
        return Reset;
    if(!strcmp("help",arg[0]))
        return Help;
    if(!strcmp("pvp",arg[0]))
        return Pvp;
    if(!strcmp("pve",arg[0]))
        return Pve;
    if(!strcmp("avp",arg[0]))
        return Avp;
    if(!strcmp("ave",arg[0]))
        return Ave;
    if(!strcmp("clear",arg[0]))
        return Clear;
    if(!strcmp("sdcard",arg[0]))
        return SDCard;
    if(!strcmp("advertise",arg[0]))
        return Advertise;
    if(!strcmp("connect",arg[0]))
        return Connect;
    if(!strcmp("disconnet",arg[0]))
        return Disconnect;
    if(!strcmp("hash",arg[0]))
        return Hash;
    if(!strcmp("bsize",arg[0]))
        return Bsize;
    if(!strcmp("A",arg[0]))
        return A;
    if(!strcmp("ip",arg[0]))
        return Ip;
    
    return 0;
}

void command_execute(int command) {
    
    // random temp variable
    int temp = 0;
    
    usb_uart_commit("\r");
    switch(command) {
        
        // Menu control commands
        
        case Reset:
            usb_uart_commit("Game has been reset!\r\r\r");
            if(arg_count > 1) {
                sscanf(arg[1],"%d", &temp);
                change_board_type(temp);
            }   
            game_board_reset();
            board_state = MENU_ADVERT;
            break;
        
        case Help:
            usb_uart_commit("Help Dialogue Shit here...\r\r");
            board_state = MENU_ADVERT;
            break;
            
        case Pvp:
            game_menu_pvp_init();
            break;
        
        case Pve:
            game_menu_pve_init();
            break;
    
        case Avp:
            game_menu_avp_init();
            break;
        
        case Ave:
            game_menu_ave_init();
            break;
            
        case Clear:
            for(temp = 0; temp < 60; temp++) {
                usb_uart_commit("\r");
            }
            board_state = MENU_ADVERT;
            break;

        // Game control Options
        
        case SDCard:
            if(arg_count > 1) {
                if(!strcmp("--on", arg[1])){
                    if(!save_replay){
                        sd_card_init();
                        save_replay = TRUE;
                        usb_uart_commit("SD Card has been initialized!\r");
                    }
                    else
                        usb_uart_commit("SD Card is aldready initialized!\r");
                }
                else if(!strcmp("--off", arg[1])){
                    if(save_replay){
                        sd_card_deinit();
                        save_replay = FALSE;
                        usb_uart_commit("SD Card has been deinitialized!\r");
                    }
                    else
                        usb_uart_commit("SD Card is aldready deinitialized!\r");
                }
                else {
                    usb_uart_commit("Command sdcard needs a valid argument. \rRefer to command help.\r");
                }    
            }
            else {
                usb_uart_commit("Command sdcard needs an argument. \rRefer to command help.\r");
            }
            board_state = MENU_ADVERT;
            break;

        // Network control Options
        
        case Advertise:
            if(arg_count > 0) {
                copy_host_id(arg[1]);
                
                construct_host_advert(cmd);
                board_state = ADVERT1;
            }
            else {
                usb_uart_commit("Command advertise needs an String argument.\r");
            }
            break;
        
        case Connect:
            if(arg_count > 0) {
                construct_host_connect(cmd);
                board_state = ADVERT1;
                guest_ip = evaluate_ip(arg[1]);
                copy_guest_id(ip_hash[guest_ip & 0x00FF].id);
            }
            else {
                usb_uart_commit("Command connect needs an String argument.\r");
            }
            break;
        
        case Disconnect:
            construct_host_disconnect(cmd);
            board_state = ADVERT1;
            break;
            
        case Hash:
            if(arg_count > 0) {
                sscanf(arg[1],"%d",(int*)&temp);
                arg[1][0] = '\0';
                sprintf(arg[1], "\rHash: %s -> %lx\r", ip_hash[temp].id, ip_hash[temp].ip);
                usb_uart_commit(arg[1]);
                
            }
            board_state = MENU_ADVERT;
            break;
            
        case Bsize:
            if(arg_count > 1) 
                sscanf(arg[1],"%d", &temp);
            game_board_init(temp);
            board_state = MENU_ADVERT;
            break;
            
        case A:
            copy_host_id("LESH");
            construct_host_advert("advertise LESH\n");
            board_state = ADVERT1;
            break;
            
        case Ip:
            arg[0][0] ='\0';
            sprintf(arg[0],"\r\rGuest IP-> %s -> %lx\rHost IP -> %s  -> %lx\r\r", get_guest_id(),guest_ip,get_host_id(),host_ip);
            usb_uart_commit(arg[0]);
            board_state = MENU_ADVERT;
            break;
            
        default: 
            board_state = MENU_ADVERT;
            break;
    }

    reset_advert_buffer_count();
    cmd[0] = '\0';
    arg[0][0] = '\0';
    usb_uart_commit("\r");
}