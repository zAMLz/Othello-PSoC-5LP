
#ifndef _GAME_LOGIC_H_
    
    #define _GAME_LOGIC_H_

    #include <project.h>
        
    #define BOARD_4     4
    #define BOARD_6     6
    #define BOARD_8     8
    #define BOARD_10    10
    #define BOARD_12    12
    #define BOARD_14    14
    #define BOARD_16    16
        
    #define WHITE_DISC 4
    #define BLACK_DISC 1
    #define GAME_DRAW   7
    #define CURSOR_DISC 2
    #define POT_WHITE   5
    #define POT_BLACK   3
        
    #define UP      0
    #define DOWN    1
    #define LEFT    2
    #define RIGHT   3
        
    #define CYAN    6
        
    #define TRUE    1
    #define FALSE   0
    
    #define LOG_FILE "LOG.TXT"

     
    void game_board_init(int board_size); 

    uint16 get_board_data(int row, int col);
    uint16 get_raw_board_data(int row, int col);
    uint8 get_board_type();
    void change_board_type(uint8 size);

    void game_board_reset();
    
    void game_menu_pvp_init();
    void game_menu_pve_init();
    void game_menu_avp_init();
    void game_menu_ave_init();
    
    void game_menu_update_OUTDATED(uint8 command);

    void game_logic_update_pve(uint8 command);
    void game_logic_update(uint8 command);
    void game_logic_update_only_reset(uint8 command);
    void game_logic_super_update();
    int game_update_score(uint8 piece);

    void cursor_move(uint8 dir);
    void cursor_home();

    void board_pot_update();
    uint8 board_pot_update_rec_step1(int i, int j, int i_up, int j_up);
    uint8 board_pot_update_rec_step2(int i, int j, int i_up, int j_up);

    uint8 place_piece();
    uint8 place_piece_nonlocal(int row_nonlocal, int col_nonlocal, int pass_nonlocal);
    void place_piece_rec();
    
    void copy_host_id(char* temp_id);
    void copy_guest_id(char* temp_id);
    char* get_host_id();
    char* get_guest_id();

#endif