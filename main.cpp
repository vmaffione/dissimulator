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

#include "lexer.h"
#include "syntax.h"
#include "symbol.h"
#include "engine.h"
#include "console.h"


ostringstream oss;


int main(int argc, char **argv)
{
    list<Token*> TokenList;

    if (argc != 2) {
	cout << "Usage: dissimulator INPUTFILE\n";
	return -1;
    }

    try
    {
	Lexer* LexAnalyzer = Lexer::getInstance(); // acquisice il puntatore all'unica istanza di Lexer
	Token* nextToken;
	LexAnalyzer->setInputFile(argv[1]);
	while (nextToken = LexAnalyzer->getToken())
	    TokenList.push_back(nextToken);
	if (DEBUG) { TKPTVCIT it = TokenList.begin(); cout << "\nLista dei Token:\n"; for (; it != TokenList.end(); it++) { cout << (*it)->getName(); if ((*it)->getName() == ";") cout << "\n"; } cout << "\n"; }

	Syntax::init();

	Syntax SyxAnalyzer(&TokenList);
	TKPTVCIT b,e;

	while (SyxAnalyzer.getInstruction(b, e))
	    SyxAnalyzer.interpret(b, e);

	// la lista dei token non serve più, quindi si libera la memoria occupata
	for (list<Token*>::iterator j = TokenList.begin(); j != TokenList.end(); j++)
	    delete (*j);
	TokenList.clear();

	Symbol* symbolTablePointer = Symbol::getInstance();

	if (DEBUG) symbolTablePointer->printAll();

	// travaso i blocchi dalla tabella dei simboli nel vettore Block::blockList
	Syntax::getBlocks();
	if (DEBUG) { cout << "Lista dei blocchi da ordinare:\n"; for (int j = 0; j<Block::blockList.size(); j++) cout << "id= " << Block::blockList[j]->getID() << " " << Block::blockList[j]->getName() << "  " << Block::blockList[j]->getType() << "\n"; cout << "\n"; }

	SortEngine* sortEnginePointer = SortEngine::getInstance();
	// ordino il vettore Block::blockList
	sortEnginePointer->arrangeBlocks();
	if (DEBUG) { cout << "Lista ordinata:\n"; for (int h=0; h<Block::blockList.size(); h++) cout << h << ") id = " << Block::blockList[h]->getID() << ", " <<  Block::blockList[h]->getName() << "\n"; cout << "\n"; }

	// lancio la console che determina l'interazione con l'utente
	Console* console = Console::getInstance();
	while (console->readAndExecuteNext()) { }

	// viene liberata la memoria relativa ai blocchi
	Block::freeTable();
    }
    catch (const GenericError& e)
    {
	if (e.getType() == LEX)
	{
	    // dealloca la memoria occupata dalla lista di token
	    for (list<Token*>::iterator j = TokenList.begin(); j != TokenList.end(); j++)
		delete (*j);
	}
	else
	    Block::freeTable();
    }
    return 0;
}
