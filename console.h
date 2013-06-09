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

#ifndef CONSOLE__H__
#define CONSOLE__H__

#include "commons.h"
#include "block.h"
#include "time.h"
#include "error.h"

class Console
{
    istream* pFin;

    static vector<string> funcNames;
    typedef int ( *IPF )( const string& );
    static vector<IPF> funcPointers;
    static int start( const string& );
    static int help( const string& );
    static int quit( const string& );

    static Console* instance;
    Console();
    class ConsoleDestructor
    {
	public:
	    ~ConsoleDestructor() { /*cout<<"Distruggo l'oggetto singleton Console\n";*/ }
    };
    friend class ConsoleDestructor;
    static ConsoleDestructor destr;
    public:
    static Console* getInstance();
    int readAndExecuteNext();
};

#endif
