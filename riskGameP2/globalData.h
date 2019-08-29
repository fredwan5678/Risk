#ifndef _GLOBAL_DATA_H
#define _GLOBAL_DATA_H

// global variables shared across files 
struct sharedData {
    uint8_t PAGENUMBER = 1;
    uint8_t NUM_TERR; 
    territory* territories;
    Adafruit_ILI9341* tft;
}

#endif