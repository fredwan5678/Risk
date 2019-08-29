#ifndef _READ_FILE_H
#define _READ_FILE_H

#include <SPI.h>
#include <SD.h>
#include "mapGraph.h"
#include <Arduino.h>

int readNumber(File& file)
{
  String input = "";
  char current;
  while(true)
  {
    current = file.read();
    if (current != ' ' and current != '\n' and current != 'E')
    {
      input += current;
    }
    else
    {
      return input.toInt();
    }
  }
}

masterMapGraph* makeMap(territory *&allTerritories) {
  File file;

  // Open requested file on SD card if not already open
  if ((file = SD.open("map.txt", FILE_READ)) == NULL) {
    Serial.println("File not found");
    masterMapGraph temp = masterMapGraph(0,0);
    return &temp;
  }

  String input = "";
  char current = file.read();
  file.read();
  if (current != 'B')
  {
    file.close();
    Serial.println("Incorrect file format");
    masterMapGraph temp = masterMapGraph(0,0);
    return &temp;
  }

  int parameters[3] = {0};
  int parameter = 0;

  while(true)
  {
    current = file.read();
    if (current != ' ' and current != '\n')
    {
      input += current;
    }
    else
    {
      parameters[parameter] = input.toInt();
      parameter++;
      input = "";
      if (parameter == 3)
      {
        break;
      }
    }
  }

  masterMapGraph *map = new masterMapGraph(parameters[0], parameters[2]);

  allTerritories = new territory[parameters[0]];
  territory newTerr;
  for (int i = 0; i < parameters[0]; ++i)
  {
    current = file.read();
    file.read();
    if (current != 'T')
    {
      file.close();
      Serial.println("Incorrect file format T");
      masterMapGraph temp = masterMapGraph(0,0);
      return &temp;
    }

    parameter = readNumber(file);
    newTerr.x = parameter;
    parameter = readNumber(file);
    newTerr.y = parameter;
    parameter = readNumber(file);
    newTerr.id = parameter;
    parameter = readNumber(file);
    newTerr.team = parameter;
    parameter = readNumber(file);
    newTerr.cont = parameter;
    parameter = readNumber(file);
    newTerr.type = parameter;
    parameter = readNumber(file);
    newTerr.magnitude = parameter;
    parameter = readNumber(file);
    newTerr.power = parameter;

    map->addVertex(newTerr);

    delay(50);

    allTerritories[newTerr.id] = newTerr;
  }

  territory from;
  territory to;
  for (int i = 0; i < parameters[1]; ++i)
  {
    current = file.read();
    file.read();
    if (current != 'P')
    {
      file.close();
      Serial.println("Incorrect file format P");
      Serial.println(current);
      masterMapGraph temp = masterMapGraph(0,0);
      return &temp;
    }

    parameter = readNumber(file);
    from = allTerritories[parameter];
    parameter = readNumber(file);
    to =  allTerritories[parameter];

    map->addEdge(from, to);
  }
  Serial.println("Finished Reading");
  file.close();
  return map;
}

#endif