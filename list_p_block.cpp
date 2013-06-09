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

#include "list_p_block.h"

Node* listPBlock::push_back(Block* pb)
{
    if (head == null) // lista vuota
    {
	head = tail = new Node;
	tail->blockPointer = pb;
	tail->next = tail->prev = null;
    }
    else
    {
	tail->next = new Node;
	tail->next->prev = tail;
	tail = tail->next;
	tail->blockPointer = pb;
	tail->next = null;
    }
    sz++;
    return tail;
}

void listPBlock::erase(Node* npt)
{
    if (npt == null)
	return;
    if (npt->prev == null)
	head = npt->next;
    else
	npt->prev->next = npt->next;
    if (npt->next == null)
	tail = npt->prev;
    else
	npt->next->prev = npt->prev;
    sz--;
}

void listPBlock::pop_front()
{
    Node* tmp = head;
    head = head->next;
    delete tmp;
    sz--;
}

listPBlock::~listPBlock()
{
    bool empty = (head == null);
    while (head != null)
    {
	cout << head->blockPointer->getName() << " ";
	tail = head; // si utilizza tail come variabile di lavoro, tanto non serve più
	head = head->next;
	delete tail;
    }
    if (!empty) cout << "Distruggo lista!\n";
}

void listPBlock::print() const
{
    for(Node* w=head; w!=null; w=w->next)
	cout << w->blockPointer->getName() << " ";
    cout << "\n";
}
