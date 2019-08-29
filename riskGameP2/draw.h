#ifndef _DRAW_H
#define _DRAW_H

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <TouchScreen.h>
#include "globalData.h"

// TFT display and SD card will share the hardware SPI interface.
// For the Adafruit shield, these are the default.
// The display uses hardware SPI, plus #9 & #10
// Mega2560 Wiring: connect TFT_CLK to 52, TFT_MISO to 50, and TFT_MOSI to 51.
#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6

// joystick pins
#define JOY_VERT_ANALOG A1
#define JOY_HORIZ_ANALOG A0
#define JOY_SEL 2

// width/height of the display when rotated horizontally
#define TFT_WIDTH 320
#define TFT_HEIGHT 240
#define TFT_PANEL_WIDTH 40 //****************

// layout of display (with side bar)
#define DISP_WIDTH (TFT_WIDTH - TFT_PANEL_WIDTH)
#define DISP_HEIGHT TFT_HEIGHT

// constants for the joystick
#define JOY_DEADZONE 64
#define JOY_CENTRE 512
#define JOY_STEPS_PER_PIXEL 64

// touch screen pins, obtained from the documentaion
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM  5  // can be a digital pin
#define XP  4  // can be a digital pin

// calibration data for the touch screen, obtained from documentation
// the minimum/maximum possible readings from the touch point
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// defines the player color for player 1 and player 2
// and the dimensions of a territory
#define P1Color 0x001F
#define P2Color 0xF800
#define terrWidth 25
#define terrHeight 20

// thresholds to determine if there was a touch
#define MINPRESSURE   10
#define MAXPRESSURE 1000

// Use hardware SPI (on Mega2560, #52, #51, and #50) and the above for CS/DC
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// a multimeter reading says there are 300 ohms of resistance across the plate,
// so initialize with this to get more accurate readings
//TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// page number, number of territories and paths

extern sharedData shared; 

// draws one territory, given the upper left coordinates
/*
Takes in:   player (owner of that territory)
            id (to get the upper left corner of territory)
*/
void drawTerritory(int player, int id) {
    // draws the territory a different color depending on who owns it, and what continent it is on
    int continent = shared.territories[id].cont;

    // fills in a colored rectangle where the territory will go - each color denotes which continent it is a part of
    switch (continent) {
        // 0: cyan, 1: magenta, 2: yellow, 3: green
        case 0: shared.tft->fillRect(shared.territories[id].x - 3, shared.territories[id].y - 3, terrWidth + 6, terrHeight + 6, 0x07FF);
        case 1: shared.tft->fillRect(shared.territories[id].x - 3, shared.territories[id].y - 3, terrWidth + 6, terrHeight + 6, 0xF81F);
        case 2: shared.tft->fillRect(shared.territories[id].x - 3, shared.territories[id].y - 3, terrWidth + 6, terrHeight + 6, 0xFFE0);
        case 3: shared.tft->fillRect(shared.territories[id].x - 3, shared.territories[id].y - 3, terrWidth + 6, terrHeight + 6, 0x07E0);
    } 

    // fills in the player's color if they own that territory
    // player 1 territories
    if (player == 1) {
        // prints the territories on page 1
        if (shared.PAGENUMBER == 1) {
            shared.tft->fillRect(shared.territories[id].x, shared.territories[id].y, terrWidth, terrHeight, P1Color);
        }

        // prints the territories on page 2
        else {
            shared.tft->fillRect(shared.territories[id].x - 280, shared.territories[id].y, terrWidth, terrHeight, P1Color);   
        }
        //shared.tft->drawRect(shared.territories[id].x, shared.territories[id].y, terrWidth, terrHeight, P1Color);
    }
    // player 2 territories
    else if (player == 2) {
        // prints the territories on page 1
        if (shared.PAGENUMBER == 1) {
            shared.tft->fillRect(shared.territories[id].x, shared.territories[id].y, terrWidth, terrHeight, P1Color);
        }

        // prints the territories on page 2
        else {
            shared.tft->fillRect(shared.territories[id].x - 280, shared.territories[id].y, terrWidth, terrHeight, P1Color);   
        }
        //shared.tft->drawRect(shared.territories[id].x, shared.territories[id].y, terrWidth, terrHeight, P2Color);   
    }

    // prints the number of armies in that territory
    shared.tft->setTextSize(3);
    shared.tft->setCursor(shared.territories[id].x + 4, shared.territories[id].y + 2);
    shared.tft->println(shared.territories[id].power);
}

// draws the road from one connected territory to another
/*
Takes in:   id1, id2 (the ids of the 2 territories being connected)
*/
void drawRoad(int id1, int id2) {
    shared.tft->drawLine(shared.territories[id1].x + terrWidth/2, shared.territories[id1].y + terrHeight/2,
                 shared.territories[id2].x + terrWidth/2, shared.territories[id2].y + terrHeight/2, 0xFFFF);
}

// draws all roads on the screen
void drawAllRoads(const masterMapGraph& map) {
    uint8_t added[shared.NUM_TERR] = {0};
    for (int i = 0; i < shared.NUM_TERR; i++) {
        for (HashTableIterator<IntWrapper> j = map.neighbours(i); !map.isLastNeighbour(i,j); j = map.nextNeighbour(i,j)) {
            if (added[j.item()] == 0) {
                drawRoad(i, j.item());
            }
        }
        added[i] = 1;
    }
}

// draws all territories and roads on that page of the map
void updateMap() {
    // draws all roads to the screen
    drawAllRoads(map);

    // draws all territories on the screen
    for (int i = 0; i < shared.NUM_TERR; i++) {
        drawTerritory(shared.territories[i].team, i);
    }
}

// draws the exit button (in white)
void drawExit() {
    shared.tft->setTextSize(1);
    shared.tft->drawRect(DISP_WIDTH, 0, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0xFFFF);
    shared.tft->setCursor(DISP_WIDTH + 2, TFT_PANEL_WIDTH/2 - 4);
    shared.tft->println("EXIT");
}

// draws the increase button for redistributing armies (in green)
void drawIncrease() {
    shared.tft->drawRect(DISP_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0x07E0);
    shared.tft->fillRect(DISP_WIDTH + TFT_PANEL_WIDTH/2 - 1, TFT_PANEL_WIDTH, 2, TFT_PANEL_WIDTH - 4, 0x07E0);
    shared.tft->fillRect(DISP_WIDTH + 2, 3*TFT_PANEL_WIDTH/2 -1, TFT_PANEL_WIDTH - 4, 2, 0x07E0);
}

// draws the decrease button for redistributing armies (in red)
void drawDecrease() {
    shared.tft->drawRect(DISP_WIDTH, 2*TFT_PANEL_WIDTH + 2, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0xF800);
    shared.tft->fillRect(DISP_WIDTH + 2, 5*TFT_PANEL_WIDTH/2 -1, TFT_PANEL_WIDTH - 4, 2, 0xF800);
}

// draws a right arrow to allow player to shift the screen to the right
void drawScrollRight() {
    shared.tft->drawRect(DISP_WIDTH, 5*TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0xFFFF);
    shared.tft->drawLine(DISP_WIDTH + 3, 5*TFT_PANEL_WIDTH + 2, DISP_WIDTH + TFT_PANEL_WIDTH - 2, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    shared.tft->drawLine(DISP_WIDTH + 2, 5*TFT_PANEL_WIDTH + 2, DISP_WIDTH + TFT_PANEL_WIDTH - 3, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    shared.tft->drawLine(DISP_WIDTH + 3, 6*TFT_PANEL_WIDTH - 2, DISP_WIDTH + TFT_PANEL_WIDTH - 2, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    shared.tft->drawLine(DISP_WIDTH + 2, 6*TFT_PANEL_WIDTH - 2, DISP_WIDTH + TFT_PANEL_WIDTH - 3, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
}

// draws a left arrow to allow player to shift the screen to the left
void drawScrollLeft() {
    shared.tft->drawRect(DISP_WIDTH, 5*TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0xFFFF);
    shared.tft->drawLine(TFT_WIDTH - 3, 5*TFT_PANEL_WIDTH + 2, DISP_WIDTH + 2, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    shared.tft->drawLine(TFT_WIDTH - 2, 5*TFT_PANEL_WIDTH + 2, DISP_WIDTH + 3, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    shared.tft->drawLine(TFT_WIDTH - 3, 6*TFT_PANEL_WIDTH - 2, DISP_WIDTH + 2, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    shared.tft->drawLine(TFT_WIDTH - 2, 6*TFT_PANEL_WIDTH - 2, DISP_WIDTH + 3, 11*TFT_PANEL_WIDTH/2, 0xFFFF);   
}


#endif