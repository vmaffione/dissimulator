#ifndef LIST_P_BLOCK__HH__
#define LIST_P_BLOCK__HH__

#include "block.h"

class Node
  {
    //private:
              public:
      Block* blockPointer; // dato
      Node* next;
      Node* prev;
    friend class listPBlock; 
    public:
      Block* operator*() { return blockPointer; } // attualmente non usata da nessuno
  };

class listPBlock
  {
    private:
      Node* head;
      Node* tail;
      int sz;
    public:
      listPBlock(): head( null ), tail( null ), sz( 0 ) { }
      int size() const { return sz; }
      Block* front() const { return head->blockPointer; }
      Node* push_back( Block* pb );
      void erase( Node* npt );
      void pop_front();
      //Block* getElement( const indicator& ind ) const { return ind.nodeP->blockPointer; }
      void print() const;
      ~listPBlock();
  };
  
#endif
