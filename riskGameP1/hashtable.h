/*
  Name: Aditya Harvi
  Student ID: 1532770
  CMPUT 275, Winter 2019
  Weekly Assignment 4
  Dynamic Hashing and Makefile
*/

#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include "linkedlist.h"
//#include <cassert>

// An iterator. With more time, we could make this "safer" by
// making the members private but visible to the HashTable class
// by using "friend".
template <typename T>
struct HashTableIterator {
  unsigned int bucket;
  ListNode<T> *node;

  T item() const {
    return node->item;
  }
};

/*
  A hash table for storing items. It is assumed the type T of the item
  being stored has a hash method, eg. you can call item.hash(), which
  returns an unsigned integer.

  Also assumes the != operator is implemented for the item being stored,
  so we could check if two items are different same.

  If you just want store integers, wrap them up in a struct
  with a .hash() method and an != operator.
*/

template <typename T>
class HashTable {
public:
  // creates an empty hash table of the given size.
  // If no parameter is given then it is default to 10.
  HashTable(unsigned int tableSize = 4) {
    // This will ensure valid tablesize if given.
    //assert(tableSize > 0);
    // Makes the table.
    table = new LinkedList<T>[tableSize];
    //assert(table != NULL);

    this->tableSize = tableSize;
    numItems = 0;
  }

  ~HashTable() {
    // will call the destructor for each list in the table
    delete[] table;
  }

  // Check if the item already appears in the table.
  bool contains(const T& item) const;

  // Insert the item, do nothing if it is already in the table.
  // Returns true iff the insertion was successful (i.e. the item was not there).
  bool insert(const T& item);

  // Removes the item after checking, via assert, that the item was in the table.
  void remove(const T& item);

  // Returns the number of items held in the table.
  unsigned int size() const { return numItems; }

  // Useful methods for iteration.

  // Get the iterator for the first item.
  HashTableIterator<T> startIterator() const;

  // Advance the iterator (returns the next iterator).
  HashTableIterator<T> nextIterator(const HashTableIterator<T>& iter) const;

  // Test if a given iterator is the end iterator.
  bool isEndIterator(const HashTableIterator<T>& iter) const;

private:
  LinkedList<T> *table;
  LinkedList<T> *temptable;
  HashTableIterator<T> *starting;
  unsigned int numItems, tableSize;

  // Computes which hash table bucket the item maps to.
  unsigned int getBucket(const T& item) const {
    return item.hash() % tableSize;
  }

  // Computes the new buckets for the new tablesize.
  unsigned int getnewBucket(const T& item) {
    return item.hash() % (tableSize * 2);
  }

  // This will resize the table. (Double it)
  void resize();
};

template <typename T>
bool HashTable<T>::contains(const T& item) const {
  // just get the item's bucket and use the lists find feature
  unsigned int hashVal = getBucket(item);
  return (table[hashVal].find(item) != NULL);
}

template <typename T>
void HashTable<T>::remove(const T& item) {
  unsigned int bucket = getBucket(item);

  ListNode<T> *node = table[bucket].find(item);

  table[bucket].removeNode(node);
  numItems--;
}

template <typename T>
HashTableIterator<T> HashTable<T>::startIterator() const {
  HashTableIterator<T> iter;
  if (numItems == 0) {
    // indicates this is after all items
    iter.bucket = tableSize;
    iter.node = NULL;
  }
  else {
    // find the first nonempty bucket
    for (iter.bucket = 0; table[iter.bucket].size() == 0; iter.bucket++);
    // and point to the first item in this bucket
    iter.node = table[iter.bucket].getFirst();
  }
  return iter;
}

template <typename T>
HashTableIterator<T> HashTable<T>::nextIterator(const HashTableIterator<T>& iter) const {
  HashTableIterator<T> next = iter;

  next.node = next.node->next;

  // This bucket is done, move on to the next nonempty bucket or
  // to the end of the buckets (i.e. end iterator).
  if (next.node == NULL) {
    next.bucket++;

    // Iterate until you reach another nonempty bucket or the end of buckets
    while (next.bucket < tableSize && table[next.bucket].size() == 0) {
      next.bucket++;
    }

    // If we ended at a nonempty bucket, point to the first item.
    if (next.bucket < tableSize) {
      next.node = table[next.bucket].getFirst();
    }
    // Otherwise, this is the end iterator so let's point to NULL to be safe.
    else {
      next.node = NULL;
    }
  }
  return next;
}

template <typename T>
bool HashTable<T>::isEndIterator(const HashTableIterator<T>& iter) const {
  // We use iter.bucket == tableSize to represent the end iterator.
  return (iter.bucket == tableSize);
}

template <typename T>
void HashTable<T>::resize() {
  // Creates a temporary table which will be double the size of the old table.
  temptable = new LinkedList<T>[tableSize * 2];
  // This will mark the starting iterator.
  *starting = startIterator();
  // This will be a variable which determines the new buckets.
  unsigned int newbucket;
  // For loop will determine the new buckets for each iterator and then insert
  // them into the temptable.
  for (unsigned int i = 0; i < size(); i++) {
    // Gets the new bucket.
    newbucket = getnewBucket(starting->item());
    // Inserts the item from the old table into the new based off bucket
    // location.
    temptable[newbucket].insertFront(starting->item());
    // This will mark the next interator.
    *starting = nextIterator(*starting);
  }
  // This will delete the old HashTable.
  HashTable<T>::~HashTable();
  // Doubles the tableSize
  tableSize *= 2;
  // Copies data from the temptable into the table variable.
  table = temptable;
}

template <typename T>
bool HashTable<T>::insert(const T& item) {
  // Records the current size of the table.
  unsigned int size = HashTable<T>::size();
  // If all the buckets are full then we will resize the table.
  if (size == tableSize) {
    resize();
  }

  // get the item's bucket
  unsigned int bucket = getBucket(item);

  // If the item is not in this bucket, insert it.
  // Use insertFront, it always takes O(1) time.
  if (table[bucket].find(item) == NULL) {
    table[bucket].insertFront(item);
    numItems++;
    return true;
  }
  else {
    // the item was already in the table
    return false;
  }
}

#endif
