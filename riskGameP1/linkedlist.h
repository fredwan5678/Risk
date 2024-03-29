#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

//#include <cassert>
#include <stddef.h>
/*#include <stdlib.h> // for malloc and free 
void* operator new(size_t size) { return malloc(size); } 
void operator delete(void* ptr) { free(ptr); }*/

using namespace std;

// struct for holding an item and pointers to the next and previous node
template <typename T>
struct ListNode {
  // constructor
  ListNode(const T& item, ListNode<T> *prev = NULL, ListNode<T> *next = NULL) {
    this->item = item;
    this->prev = prev;
    this->next = next;
  }

  T item;
  ListNode<T> *next, *prev;
};

// A linked list, just as discussed in the slides.
template <typename T>
class LinkedList {
public:
  LinkedList() {
    first = last = NULL;
    listSize = 0;
  }

  ~LinkedList() {
    // delete one item at a time until the list is empty
    while (size() > 0) {
      removeFront();
    }
  }

  void insertFront(const T& item);
  void insertBack(const T& item);

  void removeFront();
  void removeBack();

  // assumes the node is in this list
  // will insert the item just before this node in the list
  void insertBefore(const T& item, ListNode<T> *node);

  // assumes the node is in this list
  void removeNode(ListNode<T> *node);

  unsigned int size() const { return listSize; }

  // Get ListNode pointers to the first and last items in the list,
  // respectively. Both return a pointer to NULL if the list is empty.
  ListNode<T>* getFirst() const { return first; }
  ListNode<T>* getLast() const { return last; }

  // Find and return a pointer to the first node with the item.
  // Returns the NULL pointer if the item is not in the list.
  ListNode<T>* find(const T& item) const;

private:
  ListNode<T> *first, *last;
  unsigned int listSize;
};

template <typename T>
void LinkedList<T>::insertFront(const T& item) {
  // get a new ListNode to hold the item
  // it points back to NULL and ahead to the first node in current list
  ListNode<T> *node = new ListNode<T>(item, NULL, first);
  //assert(node != NULL);

  // if the list is not empty, have the first node point back to the new node.
  if (first != NULL) {
    first->prev = node;
  }
  else {
    // otherwise, this new node is also the last node
    last = node;
  }

  // update the first node
  first = node;
  listSize++;
}


template <typename T>
void LinkedList<T>::insertBack(const T& item) {
  // similar to insertFront

  ListNode<T> *node = new ListNode<T>(item, last, NULL);
  //assert(node != NULL);

  if (last != NULL) {
    last->next = node;
  }
  else {
    first = node;
  }

  last = node;
  listSize++;
}

template <typename T>
void LinkedList<T>::insertBefore(const T& item, ListNode<T> *link) {
  // if the link is the start of the list, just call insertFront
  if (link == first) {
    insertFront(item);
    return;
  }

  // get a new node to hold this item
  ListNode<T> *node = new ListNode<T>(item, link->prev, link);
  //assert(node != NULL);

  // redirect surrounding links, the order you do this is important!
  link->prev->next = node;
  link->prev = node;
  listSize++;
}

template <typename T>
void LinkedList<T>::removeFront() {
  //assert(first != NULL);

  ListNode<T> *toDelete = first;

  // if this is not the only item in the list, redirect
  // the prev pointer of the 2nd item to NULL
  if (first != last) {
    first->next->prev = NULL;
  }
  else {
    // otherwise, the list will become empty
    last = NULL;
  }

  // works even if the list had size 1
  first = first->next;

  delete toDelete;
  listSize--;
}

template <typename T>
void LinkedList<T>::removeBack() {
  //assert(first != NULL);

  ListNode<T> *toDelete = last;

  // if this is not the only item in the list, redirect
  // the next pointer of the 2nd last item to NULL
  if (first != last) {
    last->prev->next = NULL;
  }
  else {
    // the list will become empty
    first = NULL;
  }

  // works even if the list had size 1
  last = last->prev;

  delete toDelete;
  listSize--;
}


template <typename T>
void LinkedList<T>::removeNode(ListNode<T>* node) {
  // if we are removing the first or the last item, call that function
  if (node == first) {
    removeFront();
    return; // something followed the first node
  }

  if (node == last) {
    removeBack();
    return; // nothing followed the last node
  }

  // bypass the node we are deleting by redirecting pointers of surrounding nodes
  node->prev->next = node->next;
  node->next->prev = node->prev;

  delete node;
  listSize--;
}

template <typename T>
ListNode<T>* LinkedList<T>::find(const T& item) const {
  // crawl along the list until the item is found
  ListNode<T>* node = first;
  while (node != NULL && node->item != item) {
    node = node->next;
  }

  // returns NULL if the item was not found
  return node;
}

#endif
