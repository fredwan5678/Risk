#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <TouchScreen.h>
#include "readFile.h"
#include "comm.h"
//#include "draw.h"
//#include "globalData.h"

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
#define P1Color 0x001F  //****************
#define P2Color 0xF800  //****************
#define terrWidth 25    //****************
#define terrHeight 25   //****************

// thresholds to determine if there was a touch
#define MINPRESSURE   10
#define MAXPRESSURE 1000

// Use hardware SPI (on Mega2560, #52, #51, and #50) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// a multimeter reading says there are 300 ohms of resistance across the plate,
// so initialize with this to get more accurate readings
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

/***********************************************************************************/

uint8_t PAGENUMBER = 1;
uint8_t NUM_TERR;
territory* territories;

/***********************************************************************************/

// gets the coordinates of the users touch
/*
Takes in:   touch_x (x coordinate of users touch)
            touch_y (y coordinate of users touch)

Returns:  Nothing
*/
void getTouch(int16_t &touch_x, int16_t &touch_y) {
    TSPoint touch;
    do {
        touch = ts.getPoint();                
    } while(touch.z < MINPRESSURE or touch.z > MAXPRESSURE);

    // maps touch to screen
    touch_x = map(touch.y, TS_MINY, TS_MAXY, TFT_WIDTH - 1, 0) - 20;
    touch_y = map(touch.x, TS_MINX, TS_MAXX, 0, TFT_HEIGHT - 1); 

    // small delay so touches don't spam
    delay(100);
}

// draws the powers for each territory
/*
Takes in:   player (owner of that territory)
            id (to get the upper left corner of territory)

Returns:    Nothing
*/
void drawPowers(int player, int id) {
    // prints the number of armies in that territory, adjusts depending on how many armies are there
    tft.setTextSize(3);
    tft.setTextColor(0xFFFF);
    uint16_t shift = (PAGENUMBER - 1) * DISP_WIDTH;

    // changes the color if it is a special territory
    switch(territories[id].type) {
        // 1: fire, 2: dam, 3: fort, 4: fertile land
        case 1: tft.setTextColor(0xFCC6);   //orange
            break;
        case 2: tft.setTextColor(0x07FF);   // blue
            break;
        case 3: tft.setTextColor(0x8430);   // grey
            break;
        case 4: tft.setTextColor(0xFFED);   // yellow
            break;
    }

    // only print numbers if they are on page 1 (omit page 2)
    if (PAGENUMBER == 1 and territories[id].x <= DISP_WIDTH) {
        tft.setCursor((territories[id].x + 4), (territories[id].y + 2));
        // change text size and cursor location if there are more than 10 armies
        if (territories[id].power >= 10) {
            tft.setTextSize(2);
            tft.setCursor((territories[id].x + 1), (territories[id].y + 5));
        }
        tft.println(territories[id].power);
    }

    // only print numbers if they are on page 2 (omit page 1)
    else if (PAGENUMBER == 2 and territories[id].x >= DISP_WIDTH) {
        tft.setCursor((territories[id].x + 4) % DISP_WIDTH, (territories[id].y + 2));
        // change text size and cursor location if there are more than 10 armies    
        if (territories[id].power >= 10) {
            tft.setTextSize(2);
            tft.setCursor((territories[id].x - shift + 1), (territories[id].y + 5));
        }
        tft.println(territories[id].power);    
    }
}

// draws one territory, given the upper left coordinates
/*
Takes in:   player (owner of that territory)
            id (to get the upper left corner of territory)

Returns:    Nothing
*/
void drawTerritory(int player, int id) {
    // draws the territory a different color depending on who owns it, and what continent it is on
    int continent = territories[id].cont;
    int contWidth = 5;
    uint16_t shift = (PAGENUMBER - 1) * DISP_WIDTH;

    // fills in a colored rectangle where the territory will go - each color denotes which continent it is a part of
    switch (continent) {
        // 0: cyan, 1: magenta, 2: yellow, 3: green
        case 0: tft.fillRect(territories[id].x - contWidth - shift, territories[id].y - contWidth, terrWidth + 2*contWidth, terrHeight + 2*contWidth, 0x07FF);
            break;
        case 1: tft.fillRect(territories[id].x - contWidth - shift, territories[id].y - contWidth, terrWidth + 2*contWidth, terrHeight + 2*contWidth, 0xF81F);
            break;
        case 2: tft.fillRect(territories[id].x - contWidth - shift, territories[id].y - contWidth, terrWidth + 2*contWidth, terrHeight + 2*contWidth, 0xFFE0);
            break;
        case 3: tft.fillRect(territories[id].x - contWidth - shift, territories[id].y - contWidth, terrWidth + 2*contWidth, terrHeight + 2*contWidth, 0x07E0);
            break;
    } 

    // fills in the player's color if they own that territory
    // player 1 territories
    if (player == 1) {
        // prints all territories
        tft.fillRect(territories[id].x - shift, territories[id].y, terrWidth, terrHeight, P1Color);   
    }
    
    // player 2 territories
    else if (player == 2) {
        // prints all territories
        tft.fillRect(territories[id].x - shift, territories[id].y, terrWidth, terrHeight, P2Color);   
    }

    // draws the powers inside the territory
    drawPowers(player, id);
    
}

// draws the road from one connected territory to another
/*
Takes in:   id1, id2 (the ids of the 2 territories being connected)

Returns:    Nothing
*/
void drawRoad(int id1, int id2) {
    uint16_t shift = (PAGENUMBER - 1) * DISP_WIDTH;
    tft.drawLine(territories[id1].x - shift + terrWidth/2, territories[id1].y + terrHeight/2,
                 territories[id2].x - shift + terrWidth/2, territories[id2].y + terrHeight/2, 0xFFFF);
}

// draws all roads on the screen
/*
Takes in:   map (use methods to change the map)

Returns:    Nothing
*/
void drawAllRoads(masterMapGraph*& map) {
    uint8_t added[NUM_TERR] = {0};
    for (int i = 0; i < NUM_TERR; i++) {
        for (HashTableIterator<IntWrapper> j = map->neighbours(i); !map->isLastNeighbour(i,j); j = map->nextNeighbour(i,j)) {
            if (added[j.item().val] == 0) {
                drawRoad(i, j.item().val);
            }
        }
        added[i] = 1;
    }
}

// draws all territories and roads on that page of the map
/*
Takes in:   map (use methods to change the map)

Returns:  Nothing
*/
void updateMap(masterMapGraph*& map) {
    // draws all roads to the screen
    drawAllRoads(map);

    // draws all territories on the screen
    for (int i = 0; i < NUM_TERR; i++) {
        drawTerritory(territories[i].team, i);
    }
}

// draws the exit button (in white)
void drawCancel() {
    tft.setTextSize(2);
    tft.drawRect(DISP_WIDTH, 0, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0xFFFF);
    tft.setCursor(DISP_WIDTH + 2, 4);
    tft.println("CAN");
    tft.setCursor(DISP_WIDTH + 2, 20);
    tft.println("CEL");
}

// draws the end turn button
void drawEndTurn() {
    tft.setTextSize(2);
    tft.drawRect(DISP_WIDTH, 2*TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0xFFFF);
    tft.setCursor(DISP_WIDTH + 2, 4 + 2*TFT_PANEL_WIDTH);
    tft.println("END");
    tft.setCursor(DISP_WIDTH + 2, 20 + 2*TFT_PANEL_WIDTH);
    tft.println("TRN");   
}

// fills in to let user know who's turn it is
void drawPlayerTurn(int player) {
    int color;
    if (player == 1) {
        color = P1Color;
    }
    else {
        color = P2Color;
    }

    tft.fillRect(DISP_WIDTH + 1, TFT_PANEL_WIDTH + 1, TFT_PANEL_WIDTH - 2, TFT_PANEL_WIDTH - 2, color);
    tft.fillRect(DISP_WIDTH + 1, 3*TFT_PANEL_WIDTH + 1, TFT_PANEL_WIDTH - 2, 2*TFT_PANEL_WIDTH - 2, color);
}

// draws the increase button for redistributing armies (in green)
void drawIncrease() {
    tft.drawRect(DISP_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0x07E0);
    tft.fillRect(DISP_WIDTH + TFT_PANEL_WIDTH/2 - 1, TFT_PANEL_WIDTH, 2, TFT_PANEL_WIDTH - 4, 0x07E0);
    tft.fillRect(DISP_WIDTH + 2, 3*TFT_PANEL_WIDTH/2 -1, TFT_PANEL_WIDTH - 4, 2, 0x07E0);
}

// draws the decrease button for redistributing armies (in red)
void drawDecrease() {
    tft.drawRect(DISP_WIDTH, 2*TFT_PANEL_WIDTH + 2, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0x07E0);
    tft.fillRect(DISP_WIDTH + 2, 5*TFT_PANEL_WIDTH/2 -1, TFT_PANEL_WIDTH - 4, 2, 0x07E0);
}

// draws a right arrow to allow player to shift the screen to the right
void drawScrollRight() {
    tft.drawRect(DISP_WIDTH, 5*TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0xFFFF);
    tft.drawLine(DISP_WIDTH + 3, 5*TFT_PANEL_WIDTH + 2, DISP_WIDTH + TFT_PANEL_WIDTH - 2, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    tft.drawLine(DISP_WIDTH + 2, 5*TFT_PANEL_WIDTH + 2, DISP_WIDTH + TFT_PANEL_WIDTH - 3, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    tft.drawLine(DISP_WIDTH + 3, 6*TFT_PANEL_WIDTH - 2, DISP_WIDTH + TFT_PANEL_WIDTH - 2, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    tft.drawLine(DISP_WIDTH + 2, 6*TFT_PANEL_WIDTH - 2, DISP_WIDTH + TFT_PANEL_WIDTH - 3, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
}

// draws a left arrow to allow player to shift the screen to the left
void drawScrollLeft() {
    tft.drawRect(DISP_WIDTH, 5*TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, TFT_PANEL_WIDTH, 0xFFFF);
    tft.drawLine(TFT_WIDTH - 3, 5*TFT_PANEL_WIDTH + 2, DISP_WIDTH + 2, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    tft.drawLine(TFT_WIDTH - 2, 5*TFT_PANEL_WIDTH + 2, DISP_WIDTH + 3, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    tft.drawLine(TFT_WIDTH - 3, 6*TFT_PANEL_WIDTH - 2, DISP_WIDTH + 2, 11*TFT_PANEL_WIDTH/2, 0xFFFF);
    tft.drawLine(TFT_WIDTH - 2, 6*TFT_PANEL_WIDTH - 2, DISP_WIDTH + 3, 11*TFT_PANEL_WIDTH/2, 0xFFFF);   
}

// prints the side bar
/*
Takes in:   player (whos turn it is)

Returns:  Nothing
*/
void sideBar(int player) {
    // draws boundary
    tft.fillRect(DISP_WIDTH, 0, TFT_PANEL_WIDTH, DISP_HEIGHT, 0x0000);
    tft.drawRect(DISP_WIDTH, 0, TFT_PANEL_WIDTH, DISP_HEIGHT, 0xFFFF);
    // draws scroll button depending on what page we are on
    if (PAGENUMBER == 1) {
        drawScrollRight();
    }
    else if (PAGENUMBER == 2) {
        drawScrollLeft();
    }
    // draws end turn and cancel buttons
    drawCancel();
    drawEndTurn();
    drawPlayerTurn(player);
}

// Uses insertion sort to sort the territories by x-coordinate so we can
// quickly see what territory was touched by the user
/*
Takes in:   xSortedTerritories (territories sorted by x-coordinate for binary search)
            NUM_TERR (total number of territories)

Returns:  Nothing
*/
void sortTerritories(territory *&xSortedTerritories, uint8_t NUM_TERR) {
    territory key;
    int j;

    for (int i = 1; i < NUM_TERR; i++) {
        key = xSortedTerritories[i];
        j = i - 1;

        while (j >= 0 && xSortedTerritories[j].x > key.x) {
            xSortedTerritories[j + 1] = xSortedTerritories[j];
            j -= 1;
        }

        xSortedTerritories[j + 1] = key;
    }
}

// randomly draws single pixel stars to the screen for visual effect
void drawStars() {
    int x, y;
    for (int i = 0; i < 250; i++) {
        // random x and y coordinate
        x = random(0, DISP_WIDTH);
        y = random(0, DISP_HEIGHT);

        tft.drawPixel(x, y, 0xFFFF);
    }
}

// redraws the stars, roads, territories and buttons
/*
Takes in:   player (whos turn it is)
            xSortedTerritories (territories sorted by x-coordinate for binary search)
            map (use methods to change the map)
            NUM_TERR (total number of territories)

Returns:  Nothing
*/
void drawAll (masterMapGraph *&map, territory *&xSortedTerritories, int player) {
    drawStars();

    // draws all roads to the screen
    drawAllRoads(map);

    // draws all territories on the screen
    for (int ID = 0; ID < NUM_TERR; ID++) {
        drawTerritory(territories[ID].team, ID);
    }

    // draws the sidebar and buttons
    sideBar(player);
}

// setup function for beginning the game
/*
Takes in:   player (whos turn it is)
            xSortedTerritories (territories sorted by x-coordinate for binary search)
            map (use methods to change the map)

Returns:  Nothing
*/
void setup(masterMapGraph *&map, territory *&xSortedTerritories, int player) {
    // initializes SD card and serial comms
    init();
    tft.begin();
    tft.setRotation(3);
    
    // Welcome graphics
    tft.fillScreen(ILI9341_BLACK);
    drawStars();
    tft.setTextSize(3);
    tft.setTextColor(0xFFFF);
    tft.println("WELCOME TO: ");
    tft.setTextSize(4);
    tft.println("RISK IN SPACE");
    delay(5000);
    tft.fillScreen(ILI9341_BLACK);

    // initializes serial communications
    Serial.begin(9600);
    Serial3.begin(9600);
    Serial.println("Start");
    if (!SD.begin(SD_CS)) {
        Serial.println("FAILED");
        Serial.println("Make sure the SD card is inserted properly!");
        while (true) {}
    }

    // takes the custom game map (territories) and turns it into a map so we can easily change it
    map = makeMap(territories);
    NUM_TERR = map->size();

    // creates a temporary map which we sort for the binary search later
    xSortedTerritories = new territory[NUM_TERR];
    for (int i = 0; i < NUM_TERR; ++i)
    {
        xSortedTerritories[i] = territories[i];
    }

    sortTerritories(xSortedTerritories, NUM_TERR);

    // draws the full map to the screen
    drawAll(map, xSortedTerritories, player);
}

/*
The function that determines the winner of a battle

Takes in:   player (which player is attacking)
            attackingID (which territory is attacking)
            defendingID (which territory is being attacked)
            gameMap (to build map/ use methods to change the map)

Returns:  Nothing
*/
void attack(int player, int attackingID, int defendingID, masterMapGraph *&gameMap) {
    // initialize variables, (attacker chance is in percentage)
    int attackerChance = 50;
    int roll;
    randomSeed(analogRead(0));

    while (true) {
        if (territories[attackingID].power <= 1) {
            break;
        }
        // random number to simulate who wins the battle
        roll = random(1,100);

        // attacker wins battle
        if (roll < attackerChance) {
            // subtract 1 from the defenders army
            territories[defendingID].power--;

            // critical blow, defender loses an extra army
            if (roll <= 4) {
                territories[defendingID].power--;
            }
        }
        // defender wins battle
        else {
            // subtract 1 from the attackers army
            territories[attackingID].power--;
            // critical blow, sttacker loses an extra army
            if (roll >= 97) {
                territories[attackingID].power--;
            }
        }

        // check to see if the defending player is out of armies
        if (territories[defendingID].power <= 0) {
            territories[defendingID].power = (territories[attackingID].power - 1);
            territories[attackingID].power = 1;
            territories[defendingID].team = player;
            gameMap->flip(territories[defendingID], territories);
            break;
        }
        // check to see if the attacking player has 1 army left
        else if (territories[attackingID].power <= 1) {
            territories[attackingID].power = 1;
            break;
        }
        // if neither end condition is satisfied, continue battling
        else {
            continue;
        }
    }

    // player 1 wins
    if (gameMap->winner() == 1) {
        tft.fillScreen(P1Color);
        tft.setTextSize(4);
        tft.setTextColor(0xFFFF);
        tft.println("PLAYER 1 WINS");
        while (true) {}
    }
    // player 2 wins
    else if (gameMap->winner() == 2) {
        tft.fillScreen(P2Color);
        tft.setTextSize(4);
        tft.setTextColor(0xFFFF);
        tft.println("PLAYER 2 WINS");
        while (true) {}
    }

    // draws all territories on the screen
    for (int ID = 0; ID < NUM_TERR; ID++) {
        drawTerritory(territories[ID].team, ID);
    }
    sideBar(player);

}

// uses a binary search algorithm to quickly return the ID of the territory that was touched (returns -1 if not)
/*
Takes in:   xSortedTerritories (territories sorted by x-coordinate for binary search)
            start (0 in most cases)
            end (total number of territories being searched)
            x_coord (the x coordinate of the users touch)
            y_coord (the y coordinate of the users touch)            

Returns:    the id of the touched territory (-1 if its not a valid territory)
*/
int terrTouched(territory *xSortedTerritories, int start, int end, int x_coord, int y_coord) {
    int ID;
    uint16_t shift = (PAGENUMBER - 1) * DISP_WIDTH;
    x_coord += shift;
    end --;

    while (start <= end) {
        ID = start + (end - start)/2;

        // if the x coordinate is within range of the users touch
        if (x_coord > xSortedTerritories[ID].x and x_coord < (xSortedTerritories[ID].x + terrWidth)) {

            // if the y coordinate is within range of the users touch
            if (y_coord > xSortedTerritories[ID].y and y_coord < (xSortedTerritories[ID].y + terrHeight)) {
                // returns the ID of the touched territory
                return xSortedTerritories[ID].id;
            }
            // if the x-coordinates are fine, but the y-coordinates aren't
            else {
                return -1;
            }
        }
        // if the territory with ID is further right than users touch
        else if (x_coord < xSortedTerritories[ID].x) {
            end = ID - 1;
            
        }
        // if the territory with ID is further left than users touch
        else {
            start = ID + 1;
        }
    }
    // return -1 if the user didn't touch a territory
    return -1;
}

// changes the page displayed by the arduino
/*
Takes in:   player (whos turn it is)
            gameMap (to build map/ use methods to change the map)
*/
void nextPageTouch(masterMapGraph *&gameMap, int player) {

    // if the change screen button is pressed
    if (PAGENUMBER == 2) {
        PAGENUMBER = 1;
        tft.fillScreen(ILI9341_BLACK);
        drawStars();
    }
    else if (PAGENUMBER == 1) {
        PAGENUMBER = 2;
        tft.fillScreen(ILI9341_BLACK);
        drawStars();
    }

    // draws all roads to the screen
    drawAllRoads(gameMap);

    // draws all territories on the screen
    for (int ID = 0; ID < NUM_TERR; ID++) {
        drawTerritory(territories[ID].team, ID);
    }

    // draws the sidebar for the buttons
    sideBar(player);
}

/*
For when the 2 players distribute their armies at the beginning of their turn

Takes in:   player (whos turn it is)
            xSortedTerritories (territories sorted by x-coordinate for binary search)
            gameMap (use methods to change the map)
*/
void distribute(int player, territory* xSortedTerritories, masterMapGraph *&gameMap) {
    int armies = 4;
    int16_t touch_x, touch_y;
    int ID;
    // calculates the total number of armies the player gets 
    armies += gameMap->continentBonus(player, territories);

    // sets display settings
    tft.setTextSize(1); 
    tft.setTextColor(0xFFFF);
    tft.setCursor(DISP_WIDTH + 8, 3*TFT_PANEL_WIDTH + 4);
    tft.println("DIST");

    // for displaying number
    tft.setTextSize(2);
    tft.setCursor(DISP_WIDTH + 14, 3*TFT_PANEL_WIDTH + 20);

    // sets a color depending on whos turn it is
    int color;
    if (player == 1) {
        color = P1Color;
    }
    else {
        color = P2Color;
    }

    tft.print(armies);

    // while there are still armies to distribute
    while (armies > 0) {

        getTouch(touch_x, touch_y);
        ID = terrTouched(xSortedTerritories, 0, NUM_TERR, touch_x, touch_y);
        while (ID == -1 or touch_x > DISP_WIDTH and touch_y > 5*TFT_PANEL_WIDTH) {
            if (touch_x > DISP_WIDTH and touch_y > 5*TFT_PANEL_WIDTH) {
                // goes to the next page
                nextPageTouch(gameMap, player);

                // prints dist on the screen when the screen changes
                tft.setTextSize(1); 
                tft.setTextColor(0xFFFF);
                tft.setCursor(DISP_WIDTH + 8, 3*TFT_PANEL_WIDTH + 4);
                tft.println("DIST");

                // number of armies remaining to be distributed
                tft.setTextSize(2); 
                tft.setCursor(DISP_WIDTH + 14, 3*TFT_PANEL_WIDTH + 20);
                tft.print(armies);
            }
            // gets the touch if the while loop condition is satisfied
            getTouch(touch_x, touch_y);
            ID = terrTouched(xSortedTerritories, 0, NUM_TERR, touch_x, touch_y);
        }

        // if the player doesn't own that territory, get the input again
        if (territories[ID].team != player) {
            continue;
        }

        // distribute to the territory with the ID, subtract 1 from the remaining armies
        territories[ID].power++;
        armies--;
        drawTerritory(player, ID);

        // format and print the number of armies
        tft.fillRect(DISP_WIDTH + 1, 3*TFT_PANEL_WIDTH + 20, TFT_PANEL_WIDTH - 2, TFT_PANEL_WIDTH, color);
        tft.setCursor(DISP_WIDTH + 14, 3*TFT_PANEL_WIDTH + 20);
        tft.setTextSize(2);
        tft.setTextColor(0xFFFF);
        tft.print(armies);

        // short delay to not spam
        delay(200);
    }

    // redraws the sidebar once all armies are distributed
    sideBar(player);
}

// the basic game turn - player 1 attacks, player 2 attacks, both players redistribute their armies
/*
Takes in:   player (whos turn it is)
            xSortedTerritories (territories sorted by x-coordinate for binary search)
            gameMap (to build map/ use methods to change the map)
*/
void playerTurn (int player, territory* xSortedTerritories, masterMapGraph *&gameMap) {
    
    int attackingID;
    int defendingID;

    int16_t touch_x, touch_y;
    uint16_t shift = (PAGENUMBER - 1) * DISP_WIDTH;

    while (true) {

        // waits for a touch
        getTouch(touch_x, touch_y);

        // if the touch is to the right of the map
        if (touch_x > DISP_WIDTH) {
            // cancel button
            if (touch_y < TFT_PANEL_WIDTH) {
                shift = (PAGENUMBER - 1) * DISP_WIDTH;
                if (player == 1) {
                    tft.drawRect(territories[attackingID].x - shift, territories[attackingID].y, terrWidth, terrHeight, P1Color);
                    tft.drawRect(territories[attackingID].x + 1 - shift, territories[attackingID].y + 1, terrWidth - 2, terrHeight - 2, P1Color);
                }
                else {
                    tft.drawRect(territories[attackingID].x - shift, territories[attackingID].y, terrWidth, terrHeight, P2Color);
                    tft.drawRect(territories[attackingID].x + 1 - shift, territories[attackingID].y + 1, terrWidth - 2, terrHeight - 2, P2Color);
                }
                continue; //--------------------------------------
            }
            // end turn
            else if (touch_y > 2*TFT_PANEL_WIDTH and touch_y < 3*TFT_PANEL_WIDTH) {
                delay(250);
                return;
            }
            // other page button
            else if (touch_y > (DISP_HEIGHT - TFT_PANEL_WIDTH) and touch_x > DISP_WIDTH) {
                nextPageTouch(gameMap, player);
            }
            // no button was pressed
            else {
                continue;
            }
        }

        // if the touch is somewhere on the map
        else {
            attackingID = terrTouched(xSortedTerritories, 0, NUM_TERR, touch_x, touch_y);
                       
            // if the user doesn't touch a territory
            if (attackingID == -1) {
                Serial.println("NO TERRITORY");
                continue;
            }
            // if user touches enemy territory
            else if (territories[attackingID].team != player) {
                continue;
            }
            // if a valid territory is touched
            else {
                delay(200);
                shift = (PAGENUMBER - 1) * DISP_WIDTH;
                tft.drawRect(territories[attackingID].x - shift, territories[attackingID].y, terrWidth, terrHeight, 0xFFFF);
                tft.drawRect(territories[attackingID].x - shift + 1, territories[attackingID].y + 1, terrWidth - 2, terrHeight - 2, 0xFFFF);

                while (true) {
                    // waits for second territory to be touched
                    getTouch(touch_x, touch_y);

                    // if the touch is to the right of the map
                    if (touch_x > DISP_WIDTH) {
                        // end turn button
                        if (touch_y < TFT_PANEL_WIDTH) {
                            // calculate shift
                            shift = (PAGENUMBER - 1) * DISP_WIDTH;
                            if (player == 1) {
                                tft.drawRect(territories[attackingID].x - shift, territories[attackingID].y, terrWidth, terrHeight, P1Color);
                                tft.drawRect(territories[attackingID].x + 1 - shift, territories[attackingID].y + 1, terrWidth - 2, terrHeight - 2, P1Color);
                            }
                            else {
                                tft.drawRect(territories[attackingID].x - shift, territories[attackingID].y, terrWidth, terrHeight, P2Color);
                                tft.drawRect(territories[attackingID].x + 1 - shift, territories[attackingID].y + 1, terrWidth - 2, terrHeight - 2, P2Color);
                            }
                            break; //--------------------------------------
                        }
                        // end turn button
                        else if (touch_y > 2*TFT_PANEL_WIDTH and touch_y < 3*TFT_PANEL_WIDTH) {
                            delay(250);
                            return;
                        }
                        // other page button
                        else if (touch_y > (DISP_HEIGHT - TFT_PANEL_WIDTH) and touch_x > DISP_WIDTH) {
                            nextPageTouch(gameMap, player);
                            if (PAGENUMBER == 1 and territories[attackingID].x <= 280) {
                                shift = (PAGENUMBER - 1) * DISP_WIDTH;
                                tft.drawRect(territories[attackingID].x - shift, territories[attackingID].y, terrWidth, terrHeight, 0xFFFF);
                                tft.drawRect(territories[attackingID].x + 1 - shift, territories[attackingID].y + 1, terrWidth - 2, terrHeight - 2, 0xFFFF);
                            }
                            else if (PAGENUMBER == 2 and territories[attackingID].x >= 280) {
                                shift = (PAGENUMBER - 1) * DISP_WIDTH;
                                tft.drawRect(territories[attackingID].x - shift, territories[attackingID].y, terrWidth, terrHeight, 0xFFFF);
                                tft.drawRect(territories[attackingID].x + 1 - shift, territories[attackingID].y + 1, terrWidth - 2, terrHeight - 2, 0xFFFF);
                                Serial.println(territories[attackingID].x - shift);
                            }
                            
                        }
                        // no button pressed
                        else {
                            continue;
                        }
                    }

                    // if somewhere on the map is touched
                    else {
                        defendingID = terrTouched(xSortedTerritories, 0, NUM_TERR, touch_x, touch_y);

                        // if the user doesn't touch a territory
                        if (defendingID == -1) {
                            continue;
                        }
                        // if it is not a neighbour
                        else if (!gameMap->isNeighbour(attackingID, defendingID)) {
                            continue;
                        }
                        // if it is a neighbouring territory
                        else {
                            // if the user touched an enemy territory
                            if (territories[defendingID].team != player) {
                                // carries out an attack
                                attack(player, attackingID, defendingID, gameMap);
                                break;
                            }
                            // if the user touched their own territory
                            else {
                                Serial.println("REASSIGNING TROOPS");
                                if (territories[attackingID].power > 1) {
                                    territories[attackingID].power--;
                                    territories[defendingID].power++;
                                    drawTerritory(player, attackingID);
                                    drawTerritory(player, defendingID);
                                }
                                break;
                            }
                        }
                    } 
                }
                // delay so no double presses
                delay(200);
            }    
        }
    }
}

// main game loop
/*
takes in: player (whos turn it is)
*/
void gameLoop(int player) {
    territory* xSortedTerritories;
    masterMapGraph* gameMap;

    setup(gameMap, xSortedTerritories, player);

    // playerTurn(player, xSortedTerritories, gameMap);

    // comms
    bool acknowledge;

    // while the game is still going
    while (true) {
        // wait for an Acknowledgement
        do {
            acknowledge = wait('A');
            Serial.println("waiting");
        } while(!acknowledge);

        // sends an R
        Serial3.println('R');
        do
        {
            acknowledge = recievePoints(gameMap, territories, NUM_TERR);
        } while (!acknowledge);
        // sends an R
        Serial3.print('R');

        // redraws the map
        drawAll(gameMap, territories, player);
        sideBar(player);

        // main player turn
        distribute(player, xSortedTerritories, gameMap);
        playerTurn(player, xSortedTerritories, gameMap);
        //send the changes to the other player
        do {
            acknowledge = handshake();
            Serial.println("handshake 1");
        } while (!acknowledge);

        do {
            sendPoints(territories, NUM_TERR);
            acknowledge = handshake();
            Serial.println("transmit");
        } while (!acknowledge);
    }
}

// main
int main() {
    // player 2
    int player = 2;

    // main game loop
    gameLoop(player);

    return 0;
}