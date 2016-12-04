#ifndef _PACKET_COM_H_
    
    #define _PACKET_COM_H_

    #include <project.h>
        
    #define TRUE 1
    #define FALSE 0

    // Personal IP Codes
    uint32 host_ip;
    uint32 guest_ip;
    uint32 temp_ip;
    
    // Define the IP Hash Table
    typedef struct {
        char id[9];
        uint32 ip;
    } hash_table_ip;
    
    
    // Every value in this table is constantly updated at all times in isr
    hash_table_ip ip_hash[256];
    
    CY_ISR_PROTO(tx_interrupt);
    CY_ISR_PROTO(rx_interrupt);
    CY_ISR_PROTO(tx_allow);

    void reset_advert_buffer_count();
    void rx_reset();
        
    void packet_com_init();
        
    void host_packet_update(char* id, int seq, uint8 pass, int row, int col, uint8 packetSearch);

    void guest_packet_miniupdate(int seq, uint8 pass, int row, int col);
    void guest_packet_update(char* id, int seq, uint8 pass, int row, int col);
    int guest_row();
    int guest_col();
    int guest_pass();
    void stop_packet_search();

    void construct_host_packet();
    void construct_host_advert(char* advert_data);
    void construct_host_connect(char* connect_data);
    void construct_host_disconnect(char* disconnect_data);
    void flush_advert_buffer();

    uint32 evaluate_ip(char* ip_str);
    
#endif