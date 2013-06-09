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

/* file "error.h"  */

#ifndef ERROR__H__
#define ERROR__H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

enum ErrorType { LEX = 0, SYX, BLOCK, ENGINE, CONSOLE };

class GenericError
{
    public:
	virtual ErrorType getType() const = 0;
};

class LexError: public GenericError
{
    public:
	LexError( string msg ) { cout << "Lexical Error: " << msg << ".\n"; }
	virtual ErrorType getType() const { return LEX; }

};

class SyxError: public GenericError
{
    public:
	SyxError( string msg ) { cout << "Syntactical Error: " << msg << ".\n"; }
	virtual ErrorType getType() const { return SYX; }
};

class BlockError: public GenericError
{
    public:
	BlockError( string msg ) { cout << "Block Error: " << msg << ".\n"; }
	virtual ErrorType getType() const { return BLOCK; }
};

class EngineError: public GenericError
{
    public:
	EngineError( string msg ) { cout << "Engine Error: " << msg << ".\n"; }
	virtual ErrorType getType() const { return ENGINE; }
};

class ConsoleError: public GenericError
{
    public:
	ConsoleError( string msg ) { cout << "Console Error: " << msg << ".\n"; }
	virtual ErrorType getType() const { return CONSOLE; }
};

#endif
