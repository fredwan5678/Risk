#ifndef _COMM_H
#define _COMM_H

#include <Arduino.h>
#include "mapGraph.h"

//read one 'word' at a time
String timedRead(const int patience) {
  String input;
  char current;
  uint32_t startTime = millis();

  while (true) {
    if (Serial3.available() != 0) {
      current = Serial3.read();
      //if there is a separating character, return the string
      //therefore splitting characters automatically 
      if (current != '\n' and current != ' ') {
        input += current;
      }
      else {
        return input;
      }
    }
    else if ((millis() - startTime) > patience) {
      return "";
    }
  }
}
//wait for a character to be recieved in serial mon(used in send+wait)
bool wait(char target)
{
    int32_t start = millis();
    char current;
    while ((millis() - start) <= 1000)
    {
        if (Serial3.available())
        {
            current = Serial3.read();
            if (current == target)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}
//send A wait for R
bool handshake()
{
    Serial3.print('A');
    return wait('R');
}
//send points
void sendPoints(territory*& allTerr, int length)
{
    Serial3.flush();
    Serial3.println();
    Serial3.print("B ");
    for (int i = 0; i < length; ++i)
    {
        Serial3.print("T ");
        Serial3.print(allTerr[i].id);
        Serial3.print(' ');
        Serial3.print(allTerr[i].power);
        Serial3.print(' ');
        Serial3.print(allTerr[i].team);
        Serial3.print(' ');
        Serial3.flush();
    }
}
//recieve points
bool recievePoints(masterMapGraph*& gameMap, territory*& allTerr, int length)
{
    //the current token recieved
    String item;

    int parameters[3];

    while(!Serial3.available()) {}

    item = timedRead(1000);
    //starting character
    if (item != "B")
    {
        Serial.println("Out of sync");
        return false;
    }
    for (int i = 0; i < length; ++i)
    {
        while(!Serial3.available()) {}

        item = timedRead(1000);

        if (item != "T")
        {
            return false;
        }

        for (int j = 0; j < 3; ++j)
        {
            while(!Serial3.available()) {}
            item = timedRead(1000);

            parameters[j] = item.toInt();
            Serial.println(item);
        }
        //changes a map node if it is different from what was recieved
        if (allTerr[i].power != parameters[1])
        {
            allTerr[i].power = parameters[1];
        }
        if (allTerr[i].team != parameters[2])
        {
            allTerr[i].team = parameters[2];
            gameMap->flip(allTerr[i], allTerr);
        }
    }
    return true;
}

#endif