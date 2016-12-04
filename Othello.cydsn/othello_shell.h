#ifndef _OTHELLO_SHELL_H_
    
    #define _OTHELLO_SHELL_H_
          
    #define TRUE    1
    #define FALSE   0
    
    #include <project.h>
    #include <stdio.h>
    
    // State Machine
    enum STATE_MACHINE {
        MENU,
        MENU_ADVERT,
        ADVERT1,
        ADVERT2,
        PVP,
        PVE,
        AVP,
        AVE,
        END,
    } board_state;
    
    void shell_update();
    void advert_stop(uint8 command);
    void command_update(uint8 command);
    void command_execute(int command);
    int command_parse();
    
#endif