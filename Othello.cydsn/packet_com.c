
#include <project.h>
#include <stdio.h>
#include "packet_com.h"
#include "usb_uart.h"
#include "game_logic.h"
#include "othello_shell.h"

// Data packet data structure
struct data_packet {
    char id[9];
    int seq;
    int pass;
    int row;
    int col;
}pkt_host, pkt_guest;

int guest_id_size = 8;

// String form of the packets
char pkt_host_data[30];
char pkt_guest_data[30];

// TX and RX Count data
int txcount = 0;
int rxcount = 0;

uint8 search_for_packet = FALSE;

// Some temp stuff
int tempi = 0;
int tempj = 0;
int seqtemp = 0;
int passtemp = 0;
int rowtemp = 0;
int coltemp = 0;

uint8 firstsecond[2] = {0x55, 0xAA};

char advert_buffer[150] = {};
uint8 advert_buffer_count = 0;

enum RX_STATE {
    FIRSTSECOND,
    NAME,
    SPACE,
    SEQ,
    PASSF,
    ROW,
    COL
} rxstate;


// Crazy crazy initialization code
void packet_com_init() {
    guest_ip = 0;
    host_ip = 0;
    temp_ip = 0;
    
    tx_packet_isr_StartEx(tx_interrupt);
    tx_allow_isr_StartEx(tx_allow);
    rx_packet_isr_StartEx(rx_interrupt);
    tx_sender_Start();
    tx_packet_allow_Write(0);
    Packet_Uart_Start();
}

// The TX Interrupt. Sends the packet.
// Once the interrupt sends the whole
// packet, set the control register off
CY_ISR(tx_interrupt) {
    for (tempi = 0; tempi < 4; tempi++) {
        if(pkt_host_data[txcount] == '\0') {
            txcount = 0;
            tx_packet_allow_Write(0);
            
            if(board_state == ADVERT1)
                board_state = ADVERT2;
            
            else if(board_state == ADVERT2)
                pkt_host_data[0] = '\0';
            
            break;
        }
        else{
            Packet_Uart_PutChar(pkt_host_data[txcount++]);
        } 
    }
    
}

// Allows the TX Interrupt to go through
// by setting a control register high
CY_ISR(tx_allow) {
    //if(search_for_packet) 
        tx_packet_allow_Write(1);
    tx_sender_ReadStatusRegister();
}

// Resets the RX State Machine
void rx_reset() {
    
//    char str[10];
//    str[0] = '\0';
//    sprintf(str,"[ER:%d]",rxstate);
//    usb_uart_commit(str);
    
    rxstate = FIRSTSECOND; 
    rxcount = 0;
}

void reset_advert_buffer_count() {
    advert_buffer_count = 0;
}

// The MIGHTY RX INTERRUPT
// Too much happens here as it
// parses data on the fly
CY_ISR(rx_interrupt) {

    if((board_state == ADVERT1) || (board_state == ADVERT2) || (board_state == MENU_ADVERT) || (board_state == MENU)) {
        
        // Get and store the byte data.
        uint8 pbyte = Packet_Uart_GetChar();
            
        advert_buffer[advert_buffer_count] = pbyte;
        advert_buffer_count++;
        
        if((pbyte == '\r') || (pbyte == '\n') || (advert_buffer_count >= 63)) {
            
            advert_buffer[advert_buffer_count]='\0';
            
            if(board_state != MENU)
                usb_uart_commit(advert_buffer);
            
            char temp_str[7][20] = {};
            sscanf(advert_buffer, "%s %s %s %s %s %s %s",temp_str[0],temp_str[1],temp_str[2],temp_str[3],temp_str[4],temp_str[5],temp_str[6]);
            if(!strcmp(get_host_id(),temp_str[1]) && !host_ip) {
                host_ip = evaluate_ip(temp_str[6]);
                
                usb_uart_commit("Obtained the Host Ip Address\r\n");
                //sprintf(temp_str[6],"\r\r%lx\r\r",host_ip);
                //usb_uart_commit(temp_str[6]);
            }
            
            else if(!strcmp("Connected",temp_str[0])) {
                guest_ip = evaluate_ip(temp_str[2]);
                copy_guest_id(ip_hash[guest_ip & 0x00FF].id);
                
                usb_uart_commit("Obtained the Guest Ip Address\r\n");
                //sprintf(temp_str[6],"\r\r%lx\r\r",host_ip);
                //usb_uart_commit(temp_str[6]);
            }
            
            else if(!strcmp("Player",temp_str[0])) {        
                uint16 temp_mod;
                
                temp_ip = evaluate_ip(temp_str[6]);
                //temp_mod = temp_ip % 256;
                temp_mod = temp_ip & 0x000000FF;
                ip_hash[temp_mod].ip = temp_ip;
                
                int i = 0;
                while(temp_str[1][i]) {
                    ip_hash[temp_mod].id[i] = temp_str[1][i];
                    i++;
                }
                ip_hash[temp_mod].id[i] = '\0';
                
                //usb_uart_commit("Stored in Hash Table\r\n");
//                sprintf(temp_str[6],"\r\r%lx\r\r",host_ip);
//                usb_uart_commit(temp_str[6]);
            }
            
            int i;
            for(i = 0; i < 150; i++)
                advert_buffer[i]='\0';
                
            advert_buffer_count = 0;
        }
    }
    
    // Ensure that we want to search for a packet
    else if(search_for_packet) {
        
        // Get and store the byte data.
        uint8 pbyte = Packet_Uart_GetChar();
        
        // Print out some debug code.
        char str[10];
        sprintf(str, "%c",pbyte);
        usb_uart_commit(str);
        
        // Start the RX State Machine
        switch(rxstate) {
            
            // Check to see if it is 0x55 and 0xAA
            case FIRSTSECOND:
                
                if(pbyte == firstsecond[rxcount]) {
                    pkt_guest_data[rxcount++] = pbyte;
                    
                    if(rxcount > 1) {rxstate = NAME;}
                }
                
                else rx_reset();
                
                break;
            
            // Check to see if the player ID is valid
            case NAME:
                
                if (pbyte == pkt_guest.id[rxcount-2]) {
                    pkt_guest_data[rxcount++] = pbyte;
                    
                    if(rxcount > guest_id_size + 1 ) {rxstate = SPACE;}
                }
                
                else rx_reset();
                
                break;
            
            // Ensure that this character is a space
            case SPACE:
                
                if (pbyte == 0x20) {
                    pkt_guest_data[rxcount++] = pbyte;
                    rxstate = SEQ;
                }
                
                else rx_reset();
                
                break;
            
            // Capture the sequence number
            case SEQ:
                pkt_guest_data[rxcount++] = pbyte;
                
                if(rxcount > 5 + guest_id_size) { 
                    seqtemp = (pkt_guest_data[rxcount-3] - 0x30)*100 + (pkt_guest_data[rxcount-2] - 0x30)*10 + (pkt_guest_data[rxcount-1] - 0x30);
                    
                    if( seqtemp == pkt_guest.seq) {
                        rx_reset();
                        break;
                    }
                    
                    rxstate = PASSF;
                }
                
                break;
            
            // Capture the pass data. Note how there is no error checking here.
            case PASSF:
                
                pkt_guest_data[rxcount++] = pbyte;
                
                //rectify the pass value.
                passtemp = pbyte - 0x30;
                
                rxstate = ROW;
                break;
            
            // Capture the row data. Note how there is no error checking here.
            case ROW:
                
                pkt_guest_data[rxcount++] = pbyte;
                
                if(rxcount > 8 + guest_id_size) {
                    
                    // Construct the row value.
                    rowtemp = (pkt_guest_data[rxcount-2] - 0x30)*10 + (pkt_guest_data[rxcount-1] - 0x30);
                    rxstate = COL;
                }
                
                break;
            
            // Capture the column data. Here we do have the error checking code.
            case COL:
                
                pkt_guest_data[rxcount++] = pbyte;
                
                if(rxcount > 10 + guest_id_size) {
                    
                    // Construct the col value.
                    coltemp = (pkt_guest_data[rxcount-2] - 0x30)*10 + (pkt_guest_data[rxcount-1] - 0x30);
                    if (get_raw_board_data(rowtemp - 1, coltemp - 1) == POT_BLACK || get_raw_board_data(rowtemp - 1, coltemp - 1) == POT_WHITE || passtemp) { 
                        
                        
                        // We assume here that we have a good packet here.
                        search_for_packet = FALSE;
                        
                        // Update relevent values in the structure pkt_guest
                        guest_packet_miniupdate(seqtemp, passtemp, rowtemp, coltemp);
                        usb_uart_commit("\r");
                    }
                    
                    // We always reset here since it is either an incorrect packet, or
                    // we have reached the end of the correct packet.
                    rx_reset();
                }
                break;
            
            default:
                break;
        }
    
    }
    
}

void flush_advert_buffer() {
    advert_buffer_count = 0;
    usb_uart_commit(advert_buffer);
}

// Update contents of the host packet struct
void host_packet_update(char* id, int seq, uint8 pass, int row, int col, uint8 packetSearch) {
    int i = 0;
    
    for(i = 0; i < 8; i++){
        pkt_host.id[i] = id[i];
        if(pkt_host.id[i] == '\0')
            break;
    }
    pkt_host.id[8] = '\0';
    
    pkt_host.seq = seq;
    pkt_host.pass = pass;
    pkt_host.row = row + 1;
    pkt_host.col = col + 1;
    
    search_for_packet = packetSearch;
}

// Construct the host packet in advertise mode
void construct_host_advert(char* advert_data) {
    int i;
    
    for(i = 0; i < 18; i++){
        pkt_host_data[i] = advert_data[i];
        if(pkt_host_data[i] == '\0' || pkt_host_data[i] == '\n' || pkt_host_data[i] == '\r'){
            pkt_host_data[i] = '\n';
            pkt_host_data[i+1] = '\0';
            break;
        }
    }
    pkt_host_data[29] = '\0';
    txcount = 0;
}

// Construct the host packet in connection mode
void construct_host_connect(char* connect_data) {
    int i;
    for(i = 0; i < 23; i++){
        pkt_host_data[i] = connect_data[i];
        if(pkt_host_data[i] == '\0' || pkt_host_data[i] == '\n' || pkt_host_data[i] == '\r'){
            pkt_host_data[i] = '\n';
            pkt_host_data[i+1] = '\0';
            break;
        }
    }
    pkt_host_data[29] = '\0';
    txcount = 0;
}

// Construct the host packet in disconnect mode
void construct_host_disconnect(char* disconnect_data) {
    int i;
    for(i = 0; i < 23; i++){
        pkt_host_data[i] = disconnect_data[i];
        if(pkt_host_data[i] == '\0' || pkt_host_data[i] == '\n' || pkt_host_data[i] == '\r'){
            pkt_host_data[i] = '\n';
            pkt_host_data[i+1] = '\0';
            break;
        }
    }
    pkt_host_data[29] = '\0';
    txcount = 0;
}

// Construct the host packet into a single string
void construct_host_packet() {
    int index = 0, i = 0;
    int tempnum = 0;
    
    pkt_host_data[index++] = 'd';
    pkt_host_data[index++] = 'a';
    pkt_host_data[index++] = 't';
    pkt_host_data[index++] = 'a';
    pkt_host_data[index++] = ' ';
    
    pkt_host_data[index++] = 0x55;
    pkt_host_data[index++] = 0xAA;
    
    for(i = 0; i < 8; i++){
        if(pkt_host.id[i] == '\0' || pkt_host.id[i] == ' ')
            break;
        pkt_host_data[index++] = pkt_host.id[i];
    }
    
    pkt_host_data[index++] = 0x20;
    
    tempnum = pkt_host.seq; 
    for(i = 100; i > 0; i = i/10) {
        pkt_host_data[index++] = 0x30 + (tempnum/i);
        tempnum = tempnum - (tempnum/i)*i;
    }
    
    pkt_host_data[index++] = 0x30 + pkt_host.pass;
    
    tempnum = pkt_host.row;
    for(i = 10; i > 0; i = i/10) {
        pkt_host_data[index++] = 0x30 + (tempnum/i);
        tempnum = tempnum - (tempnum/i)*i;
    }
    
    tempnum = pkt_host.col;
    for(i = 10; i > 0; i = i/10) {
        pkt_host_data[index++] = 0x30 + (tempnum/i);
        tempnum = tempnum - (tempnum/i)*i;
    }
    
    pkt_host_data[index++] = ' ';
    pkt_host_data[index++] = '\n';
    pkt_host_data[index++] = '\0';
    
}

// Update particular contents of the guest packet struct
void guest_packet_miniupdate(int seq, uint8 pass, int row, int col) {
    pkt_guest.seq = seq;
    pkt_guest.pass = pass;
    pkt_guest.row = row;
    pkt_guest.col = col;
}

// This is a guest packet struct initialize function.
// It does what the two host* functions do (combined)
// It is only invoked once.
void guest_packet_update(char* id, int seq, uint8 pass, int row, int col) {
    guest_id_size = 0;
    
    for(guest_id_size = 0; guest_id_size < 8; guest_id_size++){
        pkt_guest.id[guest_id_size] = id[guest_id_size];
        if(pkt_guest.id[guest_id_size] == '\0' || pkt_guest.id[guest_id_size] == ' '){
            break;
        }
    }
    pkt_guest.id[8] = '\0';
    
    if(seq != 0 )
        pkt_guest.seq = pkt_guest.seq;
    else
        pkt_guest.seq = seq;
    
    pkt_guest.pass = pass;
    pkt_guest.row = row + 1;
    pkt_guest.col = col + 1;
    
    // Construct the guest packet (template)
    int index = 0, i = 0;
    int tempnum = 0;
    
    pkt_guest_data[index++] = 0x55;
    pkt_guest_data[index++] = 0xAA;
    
    for(i = 0; i < 8; i++){
        if(pkt_guest.id[i] == '\0' || pkt_guest.id[i] == ' ')
            break;
        pkt_guest_data[index++] = pkt_guest.id[i];
    }
    
    pkt_guest_data[index++] = 0x20;
    
    tempnum = pkt_host.seq; 
    for(i = 100; i > 0; i = i/10) {
        pkt_guest_data[index++] = 0x30 + (tempnum/i);
        tempnum = tempnum - (tempnum/i)*i;
    }
    
    pkt_guest_data[index++] = 0x30 + pkt_guest.pass;
    
    tempnum = pkt_guest.row;
    for(i = 10; i > 0; i = i/10) {
        pkt_guest_data[index++] = 0x30 + (tempnum/i);
        tempnum = tempnum - (tempnum/i)*i;
    }
    
    tempnum = pkt_guest.col;
    for(i = 10; i > 0; i = i/10) {
        pkt_guest_data[index++] = 0x30 + (tempnum/i);
        tempnum = tempnum - (tempnum/i)*i;
    }
    
    pkt_guest_data[index++] = '\0';
    
}

// Im sorry this exists... But it needs to
int guest_row() {
    return pkt_guest.row - 1;
}

// Im sorry for my lazy coding.. :[
int guest_col() {
    return pkt_guest.col - 1;
}

int guest_pass() {
    return pkt_guest.pass;
}

void stop_packet_search() {
    search_for_packet = FALSE;
    tx_packet_allow_Write(0);
}

uint32 evaluate_ip(char* ip_str) {
    uint32 i,j,k,l;
    
    i = 0;
    while(ip_str[i] != '\0') {
        if(ip_str[i] == '.')
            ip_str[i] = ' ';
        i++;
    }
            
    sscanf(ip_str,"%u %u %u %u", (int*)&i, (int*)&j, (int*)&k, (int*)&l);
    
    return (i<<24) + (j<<16) + (k<<8) + (l);
}