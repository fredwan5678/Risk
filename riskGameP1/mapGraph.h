#ifndef _MAP_GRAPH_H_
#define _MAP_GRAPH_H_

#include "hashtable.h"
#include "dynarray.h"
#include <Arduino.h>

using namespace std;
//struct that represents a node
struct territory
{
    //coordinates of the territory
    int x;
    int y;
    //unique ID of node
    int id;
    int team;
    //The continent it's a part of
    int cont;
    //if it is special type of tile
    int type;
    //the strength of specialness
    int magnitude;
    //army power invested in tile
    int power;
};

//wrapper for int that allows it to be hashed
struct IntWrapper {
  IntWrapper(uint8_t number = 0)
  {
    val = number;
  }
  //the stored value
  uint8_t val;

  uint8_t hash() const {
    return val;
  }

  // still need this for the HashTable
  bool operator!=(const IntWrapper& rhs) const {
    return val != rhs.val;
  }
};
/*
  Represents a graph using an adjacency list representation.
  Vertices are assumed to be integers.
*/
class mapGraph {
public:
  // No constructor or destructor are necessary this time.
  // A new instance will be an empty graph with no nodes.
  mapGraph(uint8_t nodes) {
    count = nodes;
    nbrs = new HashTable<IntWrapper>*[nodes];
    for (int i = 0; i < nodes; ++i)
    {
        nbrs[i] = NULL;
    }
  }

  ~mapGraph() {
    //deallocate all the memory
    for (int i = 0; i < count; ++i)
    {
      delete nbrs[i];
    }
    delete[] nbrs;
  }
  // add a vertex
  void addVertex(territory t)
  {
    if (nbrs[t.id] == NULL)
    {
      nbrs[t.id] = new HashTable<IntWrapper>;
    }
  }

  // adds an edge, creating the vertices if they do not exist
  // if the edge already existed, does nothing
  void addEdge(territory t, territory destination)
  {
    addVertex(destination);
    nbrs[t.id]->insert(*(new IntWrapper(destination.id)));
    nbrs[destination.id]->insert(*(new IntWrapper(t.id)));
  }
  //removes and edge if it exists
  void removeEdge(uint8_t t, uint8_t destination)
  {
    if (nbrs[t]->contains(IntWrapper(destination)))
    {
      nbrs[t]->remove(IntWrapper(destination));
      nbrs[destination]->remove(IntWrapper(t));
    }
  }

  // returns true if and only if v is a vertex in the graph
  bool isVertex(uint8_t t)
  {
    if (nbrs[t] == NULL)
    {
        return false;
    }
    return true;
  }

  // returns true if and only if (u,v) is an edge in the graph
  // will certainly return false if neither vertex is in the graph
  bool isEdge(uint8_t t, uint8_t destination)
  {
    if (nbrs[t] != NULL)
    {
        return nbrs[t]->contains(IntWrapper(destination));
    }
    return false;
  }

  // returns a const iterator to the neighbours of v
  HashTableIterator<IntWrapper> neighbours(const uint8_t& t) const
  {
    return nbrs[t]->startIterator();
  }

  // returns a const iterator to the neighbours of v
  HashTableIterator<IntWrapper> nextNeighbour(const uint8_t& t, const HashTableIterator<IntWrapper>& iter) const
  {
    return nbrs[t]->nextIterator(iter);
  }

  // returns a const iterator to the end of v's neighour set
  bool isLastNeighbour(const uint8_t& t, const HashTableIterator<IntWrapper>& iter) const
  {
    return nbrs[t]->isEndIterator(iter);
  }

  // return the number of outgoing neighbours of v
  uint8_t numNeighbours(uint8_t t)
  {
    return nbrs[t]->size();
  }

  // returns the number of nodes
  uint8_t size()
  {
    return count;
  }
  //checks of a node is a neighbour
  bool isNeighbour(uint8_t from, uint8_t to)
  {
    if (nbrs[from]->contains(IntWrapper(to)))
    {
      return true;
    }
    return false;
  }

private:
  //a counter for how many nodes there are
  uint8_t count;
  //a storage for the neighbours for all the nodes
  HashTable<IntWrapper> **nbrs;
};


class masterMapGraph : public mapGraph
{
public:
  masterMapGraph(uint8_t nodes, uint8_t conts) : mapGraph(nodes)
  {
    //initialize all the data storage elements
    teamMap = new mapGraph(nodes);
    team1Amt = 0;
    team2Amt = 0;

    //initialize continent containers
    this->conts = conts;
    allConts = new DynamicArray<uint8_t>*[conts];
    contAmts = new int[conts];
    for (int i = 0; i < conts; ++i)
    {
      allConts[i] = NULL;
      contAmts[i] = 0;
    }
    //wheat bonus initialization(special tile)
    wheatbonus[0] = 0;
    wheatbonus[1] = 0;
  }
  ~masterMapGraph()
  {
    //free up all the memory
    for (int i = 0; i < conts; ++i)
    {
      delete allConts[i];
    }
    delete[] contAmts;
    delete[] teamMap;
    delete[] allConts;
  }
  //adds a vertex to all relavant containers
  void addVertex(territory t)
  {
    //add to team map
    teamMap->addVertex(t);
    if (!isVertex(t.id))
    {
      if (t.team == 1)
      {
        team1Amt++;
        Serial.print("BLUE TERR ADDED ");
        Serial.println(team1Amt);
      }
      else
      {
        team2Amt++;
        Serial.print("RED TERR ADDED ");
        Serial.println(team2Amt);
      }
      //wheat bonus
      if (t.type == 4)
      {
        wheatbonus[t.team-1] += t.magnitude;
        Serial.println("Wheat");
        Serial.println(t.id);
      }
    }
    //add to contienent list
    if (allConts[t.cont] == NULL)
    {
      allConts[t.cont] = new DynamicArray<uint8_t>;
      allConts[t.cont]->pushBack(t.id);
      contAmts[t.cont]++;
    }
    else if (!mapGraph::isVertex(t.id))
    {
      allConts[t.cont]->pushBack(t.id);
      contAmts[t.cont]++;
    }
    //add it to the main map
    mapGraph::addVertex(t);
  }

  //add an edge to all relavant maps
  void addEdge(territory t, territory destination)
  {
    addVertex(destination);
    if (t.team == destination.team)
    {
      teamMap->addEdge(t, destination);
    }
    mapGraph::addEdge(t, destination);
  }

  //filp a territory in the team map
  void flip(territory t, territory* allTerr)
  {
    //update team counts
    if (t.team == 1)
    {
      team1Amt++;
      team2Amt--;
    }
    else
    {
      team2Amt++;
      team1Amt--;
    }
    Serial.print("-------------------");
    Serial.println(team1Amt);

    if (t.type == 4)
    {
      wheatbonus[t.team-1] -= t.magnitude;
      wheatbonus[t.team] += t.magnitude;
    }
    //flip all neighbours in team map
    for (HashTableIterator<IntWrapper> i = neighbours(t.id); !isLastNeighbour(t.id, i); i = nextNeighbour(t.id, i))
    {
      if (teamMap->isEdge(t.id, i.item().val))
      {
        teamMap->removeEdge(t.id, i.item().val);
      }
      else
      {
        teamMap->addEdge(t, allTerr[i.item().val]);
      }
    }
  }

  int winner()
  {
    if (team1Amt == 0)
    {
      return 2;
    }
    else if (team2Amt == 0)
    {
      return 1;
    }
    else
    {
      return -1;
    }
  }
  //calculate continent bonus
  int continentBonus(int team, territory*& allTerr)
  {
    int bonus = size() + wheatbonus[team-1];
    for (int i = 0; i < conts; ++i)
    {
      for (int j = 0; j < contAmts[i]; ++j)
      {
        if (allTerr[allConts[i]->getItem(j)].team != team)
        {
          bonus -= contAmts[i];
          break;
        }
      }
    }
    return bonus;
  }

private:
  //a count of contienents
  uint8_t conts;
  //storage for important maps
  mapGraph *teamMap;
  int team1Amt;
  int team2Amt;
  //2D array of all contienents and the territories in them
  DynamicArray<uint8_t> **allConts;
  int* contAmts;
  int wheatbonus[2];
};

#endif