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

#ifndef ENGINE__H__
#define ENGINE__H__

#include "commons.h"
#include "block.h"
#include "error.h"
#include "symbol.h"
#include "list_p_block.h"

class SortEngine
{
    static void reassignId();

    struct ItAndInt
    {
	Node* pt;
	int level;
    };
    static SortEngine instance;
    SortEngine() { }
    SortEngine( const SortEngine& );
    SortEngine& operator=( const SortEngine& );
    bool sortBlocks();
    public:
    static SortEngine* getInstance() { return &instance; }
    bool arrangeBlocks();
};

#endif
