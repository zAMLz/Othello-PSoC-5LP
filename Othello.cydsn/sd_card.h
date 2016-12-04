#ifndef _SD_CARD_H_
    
    #define _SD_CARD_H_
    
    #include <project.h>
    
    #define TRUE    1
    #define FALSE    0
    
    void sd_card_init();
    void sd_card_deinit();
    void sd_card_file_append(char *file_name, char *str_data);
    void sd_card_file_write(char *file_name, char *str_data);
    
#endif