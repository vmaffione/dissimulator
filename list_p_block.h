/*
 *  DisSimulator - Discrete Systems simulator
 *
 *  Copyright (C) 2010  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
