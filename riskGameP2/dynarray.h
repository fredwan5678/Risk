#ifndef _DYNARRAY_H_
#define _DYNARRAY_H_

//#include <cassert>
#include <stddef.h>
#include <Arduino.h>

using namespace std;

// A dynamic array that can be resized
// when desired.
template <typename T>
class DynamicArray {
public:
  // create a new array with the given size
  DynamicArray(unsigned int size = 0);
  ~DynamicArray();

  // add a new entry to the end of the array
  void pushBack(const T& item);

  // You implement this!
  // decrease the number of items by 1
  // what should you do if it is empty?
  // Try a "quartering" trick: if only
  // 1/4 of the array is used then resize it
  // (still with some padding).
  void popBack();

  // resize the array
  void resize(unsigned int size);

  // have the array hold "item" at the given index
  void setItem(unsigned int index, const T& item);

  // get a copy of the item at the given index
  T getItem(unsigned index) const;

  unsigned int size() const;

  T& operator[] (unsigned int index);
  const T& operator[] (unsigned int index) const;


private:
  T *array; // the allocated space in the heap
  unsigned int numItems; // size of the array, for the user
  unsigned int arraySize; // size of the underlying array in the heap
};

template <typename T>
DynamicArray<T>::DynamicArray(unsigned int numItems) {
  this->numItems = 0;
  array = NULL;
  resize(numItems);
}

template <typename T>
void DynamicArray<T>::resize(unsigned int newSize) {
  unsigned int newArraySize = max(newSize*2, 5u);
  T *newArray = new T[newArraySize];

  if (array != NULL) {
    // copy the old array over
    for (unsigned int i = 0; i < min(numItems, newSize); i++) {
      newArray[i] = array[i];
    }
  }

  // update the class members for this
  // new array
  numItems = newSize;
  arraySize = newArraySize;

  // delete the old array and point to the
  // new array
  delete[] array;
  array = newArray;
}

template <typename T>
DynamicArray<T>::~DynamicArray() {
  delete[] array;
}

template <typename T>
void DynamicArray<T>::setItem(unsigned int index, const T& item) {
  //assert(0 <= index && index < numItems);
  array[index] = item;
}

template <typename T>
T DynamicArray<T>::getItem(unsigned int index) const {
  //assert(0 <= index && index < numItems);
  return array[index];
}

template <typename T>
unsigned int DynamicArray<T>::size() const {
  return numItems;
}

template <typename T>
void DynamicArray<T>::pushBack(const T& item) {
  if (numItems == arraySize) {
    resize(numItems+1);
    // we haven't put the new item in yet
    numItems--;
  }
  array[numItems] = item;
  numItems++;
}

template <typename T>
T& DynamicArray<T>::operator[] (unsigned int index) {
  //assert(0 <= index && index < numItems);
  return array[index];
}

template <typename T>
const T& DynamicArray<T>::operator[] (unsigned int index) const {
  //assert(0 <= index && index < numItems);
  return array[index];
}

#endif
