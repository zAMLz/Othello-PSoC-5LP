#include <stdio.h>
#include <project.h>
#include <string.h>
#include "game_logic.h"
#include "usb_uart.h"
#include "packet_com.h"
#include "sd_card.h"
#include "othello_shell.h"

// 16 x 16 Game Board is Max Size
uint16 game_board[16][16] = {};

// Holds the board type so logic is handled properly.
uint8 board_type = BOARD_4;

// Current Player
uint8 player_cur = BLACK_DISC;
uint8 player_cur_pot = POT_BLACK;
uint8 player_cur_pass = FALSE;
// Oppenent
uint8 player_opp = WHITE_DISC;
uint8 player_opp_pot = POT_WHITE;
uint8 player_opp_pass = FALSE;

// Scores
int black_score = 2;
int white_score = 2;
uint8 winning_color = GAME_DRAW;

// Cursor Structure
struct Cursor {
    int row;
    int col;
} cursor;

// Player IDs
char host_ID[9] = "P1";
char guest_ID[9] = "P2";

    
// Game Player Change Variable
uint8 gameChange = FALSE;

// Some Local Side stuff
uint8 local_turn = TRUE;
uint8 local_row = 0;
uint8 local_col = 0;

// Boundary threshold values.
// Particulary used in the get_board_data function
// Updated in the game_board_init function.
int colBound = 12;
int rowBound = 4;

// Rectified Column Bound
int colBound_R = 4;

// Keep a value for the turn number
int turn_number_host = 0;
int turn_number_guest = 0;

// some random string data
char temp_string[64] = {};

// Check to see if the current player is a host
// Very similar to the local turn variable. But the
// local_turn variable has some minor limitations
// which the cur_is_host variable addresses
uint8 cur_is_host = TRUE;



// Initializes the board to starting position.
void game_board_init(int board_size) {
    
    Cursor_Update_Timer_Start();
    
    // Set the board type to the user defined board type.
    if (board_size%2 == 0)
        board_type = board_size;
    else
        board_type = board_size+1;
    
    if(board_type > BOARD_16)
        board_type = BOARD_16;
    else if(board_type < BOARD_4)
        board_type = BOARD_4;
    
    // Reset the cursors location to rightmost.
    cursor_home();
    
    // Reset the player start
    player_cur = BLACK_DISC;
    player_cur_pot = POT_BLACK;
    player_cur_pass = FALSE;
    
    player_opp = WHITE_DISC;
    player_opp_pot = POT_WHITE;
    player_opp_pass = FALSE;
    
    local_col = 0;
    local_row = 0;
    
    // Reset the scores
    black_score = 2;
    white_score = 2;
    winning_color = GAME_DRAW;
    
    // Update the Boundary threshold values
    colBound = (32 - board_type)/2;
    rowBound = (16 - board_type)/2;
    colBound_R = colBound - 8;
    
    int row = 0;
    int col = 0;
    
    // Clear the board
    for (row = 0; row < 16; row++)
        for (col = 0; col < 16; col++)
            game_board[row][col] = 0;
    
    // Setup the initial pieces
    game_board[7][7] = WHITE_DISC;
    game_board[7][8] = BLACK_DISC;
    game_board[8][7] = BLACK_DISC;
    game_board[8][8] = WHITE_DISC;
    
    // Reset the turn number
    turn_number_guest = 0;
    turn_number_host = 0;
    
    // Update the potential locations
    board_pot_update();
}

// This creates the processed 16 x 32
 uint16 get_board_data(int row, int col) {

    // PROOF OF CONCEPT FOR ROWBOUND AND COLBOUND
    //if (row == rowBound && col == colBound)
    //    return CYAN;
    
    // Check to see if we are in the gameboard and border region
    if( (row > (rowBound - 2)) && 
        (row < (rowBound + board_type + 1)) && 
        (col > (colBound - 2)) && 
        (col < (colBound + board_type + 1))) {
                
        // if in gameboard, display that
        if ( (row > (rowBound - 1)) && 
             (row < (rowBound + board_type)) && 
             (col > (colBound - 1)) && 
             (col < (colBound + board_type))) {
            
            if (row == cursor.row && 
                (col-8) == cursor.col && 
                (Cursor_Update_Timer_ReadCounter()/375) &&
                (board_state != MENU && board_state != END) &&
                 local_turn) {
                
                return CURSOR_DISC;
            }
            
            return game_board[row][col-8];
        }
        
        // else display the border
        return 0xF;
    }    
    
    if ((row+col)%2 == 0 && (Cursor_Update_Timer_ReadCounter()/375) && board_state == END) 
        return winning_color;
    else if ((row+col+1)%2 == 0 && !(Cursor_Update_Timer_ReadCounter()/375) && board_state == END) 
        return winning_color;
        
    // For now, we print nothing if we are not in the game logic bounds.
    return 0x00;
}

// Gets the raw data, also removes the unecessary bits.
uint16 get_raw_board_data(int row, int col) {
    return game_board[rowBound + row][colBound_R + col] & 0x00FF;
}

// Get info on the type of board.
uint8 get_board_type() {
    return board_type;
}

// Change board size
void change_board_type(uint8 size) {
    board_type = size;
}



void game_board_reset() {
    
    guest_ip = 0;
    host_ip = 0;
    host_packet_update(host_ID, 0, 0,-1,-1, !local_turn);
    construct_host_packet();
    stop_packet_search();
    usb_uart_commit("Welcome to Othello\r");
    usb_uart_commit("for the PSoC 5LP\r\r");
    game_board_init(board_type);
    board_state = MENU;
}

void game_menu_pvp_init() {
    board_state = PVP;
    usb_uart_commit("Starting a Local Game!\r\r");
    sd_card_file_write(LOG_FILE,"");
    game_board_init(board_type);
}

void game_menu_pve_init() {
    board_state = PVE;
    usb_uart_commit("Starting an Online Game!\r\r");
    sd_card_file_write(LOG_FILE,"");
    cur_is_host = TRUE;
    game_board_init(board_type);
    
    if(host_ip < guest_ip)
        local_turn = TRUE;
    else
        local_turn = FALSE;
    
    int i;
    for(i = 0; i < 8; i++){
        guest_ID[i] = ip_hash[guest_ip & 0x000000FF].id[i];
        host_ID[i] = ip_hash[host_ip & 0x000000FF].id[i];
    }
    guest_ID[8] = '\0';
    host_ID[8] = '\0';
    
    cur_is_host = local_turn;
    host_packet_update(host_ID, 0, 0,-1,-1, !local_turn);
    guest_packet_update(guest_ID, 0,0,-1,-1);
    construct_host_packet();
    
//    char str[20];
//    sprintf(str,"$%lu %lu", (host_ip & 0x000000FF),(guest_ip & 0x000000FF));
//    usb_uart_commit(str);
}

void game_menu_avp_init() {
    board_state = AVP;
    usb_uart_commit("Starting a Local Game against an AI!\r\r");
    sd_card_file_write(LOG_FILE,"");
    game_board_init(board_type);
}

void game_menu_ave_init() {
    board_state = AVE;
    usb_uart_commit("Starting a Online Game with an AI!\r\r");
    sd_card_file_write(LOG_FILE,"");
    game_board_init(board_type);
}

// The commands available in the menu
//void game_menu_update_OUTDATED(uint8 command) {
//    
//    switch(command) {
//        case 'h' :
//            usb_uart_commit("Help Dialogue Shit here...\r\r");
//            break;
//            
//        case '1' :
//            board_state = PVP;
//            usb_uart_commit("Starting a Local Game!\r\r");
//            sd_card_file_write(LOG_FILE,"");
//            game_board_init(board_type);
//            break;
//                        
//        case '2' :
//            board_state = PVE;
//            usb_uart_commit("Starting an Online Game!\r\r");
//            sd_card_file_write(LOG_FILE,"");
//            cur_is_host = TRUE;
//            game_board_init(board_type);
//            
//            if(FALSE)
//                local_turn = TRUE;
//            else
//                local_turn = FALSE;
//            
//            cur_is_host = local_turn;
//            host_packet_update(host_ID, turn_number, 0,-1,-1, !local_turn);
//            guest_packet_update(guest_ID, turn_number,0,-1,-1);
//            construct_host_packet();
//            break;
//                        
//        case '3' :
//            board_state = AVP;
//            usb_uart_commit("Starting a Local Game against an AI!\r\r");
//            sd_card_file_write(LOG_FILE,"");
//            game_board_init(board_type);
//            break;
//                        
//        case '4' :
//            board_state = AVE;
//            usb_uart_commit("Starting a Online Game with an AI!\r\r");
//            sd_card_file_write(LOG_FILE,"");
//            game_board_init(board_type);
//            break;
//            
//        case 'R':
//            usb_uart_commit("Game has been reset!\r\r\r");
//            game_board_reset();
//            break;
//        
//        default:
//            break;
//    }
//}

// Update function handler for the PVE
void game_logic_update_pve(uint8 command) {
    
    if (local_turn)
        game_logic_update(command);
    
    else{
        
        player_cur_pass = FALSE; 
        
        game_logic_update_only_reset(command);
        
        gameChange = place_piece_nonlocal(rowBound + guest_row(), colBound_R + guest_col(), guest_pass());
        
        if(gameChange)
            game_logic_super_update();
    }
}

// The actual game logic
void game_logic_update(uint8 command) {
    
    // Clear the pass
    player_cur_pass = FALSE;
    
    switch(command) {
        // Move Up
        case 30:
            cursor_move(UP);
            //usb_uart_commit("Cursor Move Up!\r");
            break;
        
        // Move Left
        case 28:
            cursor_move(LEFT);
            //usb_uart_commit("Cursor Move Left!\r");
            break;
            
        // Move Down
        case 31:
            cursor_move(DOWN);
            //usb_uart_commit("Cursor Move Down!\r");
            break;
            
        // Move Right
        case 29:
            cursor_move(RIGHT);
            //usb_uart_commit("Cursor Move Right!\r");
            break;
            
        // Cursor Home
        case 'c':
            cursor_home();
            usb_uart_commit("Cursor Location has been reset!\r");
            break;
            
        // Board Reset
        case 'R':
            usb_uart_commit("Game has been reset!\r\r\r");
            game_board_reset();
            break;
            
        case 'x':
            if(place_piece()) 
                usb_uart_commit("Piece has been successfully placed!\r");
            else
                usb_uart_commit("You cannot place a piece there.\r");    
            break;
            
        case 'S':
            gameChange = TRUE;
            player_cur_pass = TRUE;
            usb_uart_commit("You have passed.\r");
            break;
            
        default: 
            break;
    }
    
    if(gameChange)
        game_logic_super_update();
}

// The actual game logic when
// it is the nonlocal turn
void game_logic_update_only_reset(uint8 command) {
    
    switch(command) {
            
        // Board Reset
        case 'R':
            usb_uart_commit("Game has been reset!\r\r\r");
            game_board_reset();
            break;
            
        default: 
            break;
    }
    
}

void game_logic_super_update() {
    uint8 curTemp;
    uint8 potTemp;
    uint8 pasTemp;
   
      
    char file_data[50] = {};
    
    if(local_turn && (board_state == PVE)){
        host_packet_update(host_ID, ++turn_number_host, player_cur_pass, local_row, local_col,TRUE);
        construct_host_packet();
        local_turn = FALSE;
    }
    else {
        guest_packet_update(guest_ID, turn_number_guest,0,-1,-1);
        local_turn = TRUE;
    }
    
    if(cur_is_host) {
        file_data[0] = '\0';
        sprintf(file_data, "%s %d %d %d\r\n", host_ID, player_cur_pass, local_row, local_col);
        sd_card_file_append(LOG_FILE, file_data); 
    }
    else {
        file_data[0] = '\0';
        sprintf(file_data, "%s %d %d %d\r\n", guest_ID, player_cur_pass, local_row, local_col);
        sd_card_file_append(LOG_FILE, file_data); 
    }
    cur_is_host ^= 1;
    
    black_score = game_update_score(BLACK_DISC);
    white_score = game_update_score(WHITE_DISC);
    
    curTemp = player_cur;
    potTemp = player_cur_pot;
    pasTemp = player_cur_pass;
    
    player_cur = player_opp;
    player_cur_pot = player_opp_pot;
    player_cur_pass = player_opp_pass;
    
    player_opp = curTemp;
    player_opp_pot = potTemp;
    player_opp_pass = pasTemp;
    
    board_pot_update();
    
    if(player_cur_pass && player_opp_pass) {
        board_state = END;
        if (black_score > white_score)
            winning_color = BLACK_DISC;
        else if(white_score > black_score)
            winning_color = WHITE_DISC;
    }
    

    
    gameChange = FALSE;
}

int game_update_score(uint8 piece) {
    int score = 0;
    int i, j;
    for(i = 0; i < 16; i++){
        for(j = 0; j < 16; j++) {
            if (game_board[i][j] == piece)
                score++;
        }
    }
    return score;
}

void cursor_move(uint8 dir) {
    Cursor_Update_Timer_WriteCounter(0);
    
    switch(dir) {
        case UP:
            cursor.row--;
            if (cursor.row < rowBound) {
                cursor.row = rowBound + board_type - 1;
                cursor.col--;
                if (cursor.col < colBound_R)
                    cursor.col = colBound_R + board_type - 1;
            }
            break;
        case DOWN:
            cursor.row++;
            if (cursor.row >= rowBound + board_type) {
                cursor.row = rowBound;
                cursor.col++;
                if (cursor.col >= colBound_R + board_type)
                    cursor.col = colBound_R;
            }
            break;
        case LEFT:
            cursor.col--;
            if (cursor.col < colBound_R) {
                cursor.col = colBound_R + board_type - 1;
                cursor.row--;
                if (cursor.row < rowBound)
                    cursor.row = rowBound + board_type - 1;
            }
            break;
        case RIGHT:
            cursor.col++;
            if (cursor.col >= colBound_R + board_type) {
                cursor.col = colBound_R;
                cursor.row++;
                if (cursor.row >= rowBound + board_type)
                    cursor.row = rowBound;
            }
            break;
    }
    
}

// Sends the cursor back home
void cursor_home() {
    Cursor_Update_Timer_WriteCounter(0);
    cursor.row = 8  - board_type/2;
    cursor.col = 8  - board_type/2;
}


// Updates the potential location
// Please ENSURE that player_cur and player_opp
// are up to date!!!
void board_pot_update() {
    uint8 i, j;
    
    
    // Iterate through the whole gameboard
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            
            // If its not a played disc, lets search for potential locations
            if (game_board[i][j] != WHITE_DISC && game_board[i][j] != BLACK_DISC) {
                
                // this will clear any previous data held in this spot
                game_board[i][j] = 0;
                
                // the previous-row checks
                if(board_pot_update_rec_step1(i,j,-1, -1)) {
                    game_board[i][j] = (0xFF00 & (0x0100 | game_board[i][j])) | (0x00FF & player_cur_pot);
                }
                if(board_pot_update_rec_step1(i,j,-1, 0)) {
                    game_board[i][j] = (0xFF00 & (0x0200 | game_board[i][j])) | (0x00FF & player_cur_pot);
                }
                if(board_pot_update_rec_step1(i,j,-1, 1)) {
                    game_board[i][j] = (0xFF00 & (0x0400 | game_board[i][j])) | (0x00FF & player_cur_pot);
                }
                
                // the in-same-row checks
                if(board_pot_update_rec_step1(i,j,0, -1)) {
                    game_board[i][j] = (0xFF00 & (0x0800 | game_board[i][j])) | (0x00FF & player_cur_pot);
                }
                if(board_pot_update_rec_step1(i,j,0, 1)) {
                    game_board[i][j] = (0xFF00 & (0x1000 | game_board[i][j])) | (0x00FF & player_cur_pot);
                }
                
                // the up-and-comming-row checks
                if(board_pot_update_rec_step1(i,j,1, -1)) {
                    game_board[i][j] = (0xFF00 & (0x2000 | game_board[i][j])) | (0x00FF & player_cur_pot);
                }
                if(board_pot_update_rec_step1(i,j,1, 0)) {
                    game_board[i][j] = (0xFF00 & (0x4000 | game_board[i][j])) | (0x00FF & player_cur_pot);
                }
                if(board_pot_update_rec_step1(i,j,1, 1)) {
                    game_board[i][j] = (0xFF00 & (0x8000 | game_board[i][j])) | (0x00FF & player_cur_pot);
                }
            }
        }
    }
}

uint8 board_pot_update_rec_step1(int i, int j, int i_up, int j_up) {
    uint16 next_piece = game_board[i + i_up][j + j_up];
    
    if (next_piece == 0)
        return FALSE;
    else if (next_piece == player_cur)
        return FALSE;
    else if (next_piece == player_opp)
        return board_pot_update_rec_step2(i+i_up, j+j_up, i_up, j_up);
    else
        return FALSE;
}

uint8 board_pot_update_rec_step2(int i, int j, int i_up, int j_up) {
    uint16 next_piece = game_board[i + i_up][j + j_up];
    
    if (next_piece == 0)
        return FALSE;
    else if (next_piece == player_cur)
        return TRUE;
    else if (next_piece == player_opp)
        return board_pot_update_rec_step2(i+i_up, j+j_up, i_up, j_up);
    else
        return FALSE;
}


// This function places the piece onto the gameboard
// It assumes that it is a valid location and that it will
// find the location. The location is guaranteed via the
// board_pot_update function. It will write to the game board
// the possible locations and this function will just test to
// see if thats value is there.

uint8 place_piece() {
    char str[20];
    
    uint8 cursor_loc_data = game_board[cursor.row][cursor.col] & 0x00FF;
    
    if (cursor_loc_data == player_cur_pot ) {
        
        str[0] = '\0';
        sprintf(str,"\r\rFound Local PotLoc\r %0x\r\r", 0xFF00 & game_board[cursor.row][cursor.col] );
        usb_uart_commit(str);
        
        if(0x0100 & game_board[cursor.row][cursor.col])
            place_piece_rec(cursor.row - 1, cursor.col - 1, -1, -1);
        if(0x0200 & game_board[cursor.row][cursor.col])
            place_piece_rec(cursor.row - 1, cursor.col    , -1,  0);        
        if(0x0400 & game_board[cursor.row][cursor.col])
            place_piece_rec(cursor.row - 1, cursor.col + 1, -1,  1);        
        if(0x0800 & game_board[cursor.row][cursor.col])
            place_piece_rec(cursor.row    , cursor.col - 1,  0, -1);        
        if(0x1000 & game_board[cursor.row][cursor.col])
            place_piece_rec(cursor.row    , cursor.col + 1,  0,  1);        
        if(0x2000 & game_board[cursor.row][cursor.col])
            place_piece_rec(cursor.row + 1, cursor.col - 1,  1, -1);        
        if(0x4000 & game_board[cursor.row][cursor.col])
            place_piece_rec(cursor.row + 1, cursor.col    ,  1,  0);        
        if(0x8000 & game_board[cursor.row][cursor.col])
            place_piece_rec(cursor.row + 1, cursor.col + 1,  1,  1);
        
        game_board[cursor.row][cursor.col] = player_cur; 
        
        
        local_row = cursor.row - rowBound;
        local_col = cursor.col - colBound_R;
        
            
        gameChange = TRUE;
        return TRUE;
    }
    else 
        return FALSE;
}

// THIS FUNCTION IS THE NON LOCAL VERSION OF THE PREVIOUS FUNCTION
// i.e. IT MEANS THAT THE ROW AND COL VALUE IS NOT COMING FROM THE
// CURSOR AND FROM ANOTHER INTERFACE.
//-----------------------------------------------------------------
// This function places the piece onto the gameboard
// It assumes that it is a valid location and that it will
// find the location. The location is guaranteed via the
// board_pot_update function. It will write to the game board
// the possible locations and this function will just test to
// see if thats value is there.

uint8 place_piece_nonlocal(int row_nonlocal, int col_nonlocal, int pass_nonlocal) {
    char str[20];
    
    if (pass_nonlocal) {
        player_cur_pass = TRUE;
        return TRUE;
    }
    
    uint8 cursor_loc_data = game_board[row_nonlocal][col_nonlocal] & 0x00FF;
    
    if (cursor_loc_data == player_cur_pot ) {
        
        str[0] = '\0';
        sprintf(str,"\r\rFound NonLocal PotLoc\r %0x\r\r", 0xFF00 & game_board[row_nonlocal][col_nonlocal] );
        usb_uart_commit(str);
        
        if(0x0100 & game_board[row_nonlocal][col_nonlocal])
            place_piece_rec(row_nonlocal - 1, col_nonlocal - 1, -1, -1);
        if(0x0200 & game_board[row_nonlocal][col_nonlocal])
            place_piece_rec(row_nonlocal - 1, col_nonlocal    , -1,  0);        
        if(0x0400 & game_board[row_nonlocal][col_nonlocal])
            place_piece_rec(row_nonlocal - 1, col_nonlocal + 1, -1,  1);        
        if(0x0800 & game_board[row_nonlocal][col_nonlocal])
            place_piece_rec(row_nonlocal    , col_nonlocal - 1,  0, -1);        
        if(0x1000 & game_board[row_nonlocal][col_nonlocal])
            place_piece_rec(row_nonlocal    , col_nonlocal + 1,  0,  1);        
        if(0x2000 & game_board[row_nonlocal][col_nonlocal])
            place_piece_rec(row_nonlocal + 1, col_nonlocal - 1,  1, -1);        
        if(0x4000 & game_board[row_nonlocal][col_nonlocal])
            place_piece_rec(row_nonlocal + 1, col_nonlocal    ,  1,  0);        
        if(0x8000 & game_board[row_nonlocal][col_nonlocal])
            place_piece_rec(row_nonlocal + 1, col_nonlocal + 1,  1,  1);
        
        game_board[row_nonlocal][col_nonlocal] = player_cur;
        return TRUE;
    }
    else 
        return FALSE;
}

void place_piece_rec(int i, int j, int i_up, int j_up) {
    char str[20];
    sprintf(str,"\ri:%d j:%d iu:%d ju:%d\r",i,j,i_up,j_up);
    usb_uart_commit(str);
    if (game_board[i][j] == player_opp){
        game_board[i][j] = player_cur;
        place_piece_rec(i + i_up, j + j_up, i_up, j_up);
    }
}

void copy_host_id(char* temp_id) {
    int i = 0;
    while(temp_id[i] != '\0') {
        host_ID[i] = temp_id[i];
        i++;
    }
    host_ID[i] = '\0';
    //usb_uart_commit(host_ID);
}

void copy_guest_id(char* temp_id) {
    int i = 0;
    while(temp_id[i] != '\0') {
        guest_ID[i] = temp_id[i];
        i++;
    }
    guest_ID[i] = '\0';
    //usb_uart_commit(guest_ID);
}

char* get_host_id() {
    return host_ID;
}

char* get_guest_id() {
    return guest_ID;
}
