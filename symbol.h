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

/* file "symbol.h"
   Contiene le dichiarazioni relative alla tabella dei simboli dell'analizzatore.
   Questo modulo fa da servente per il modulo dell'analizzatore sintattico
   ( "sintax" );
   La tabella dei simboli è implementata in questo caso mediante una map STL.
   L'interfaccia di questo modulo verso il modulo "syntax" è costituita (oltre
   alla funzione per ottenere l'istanza di Symbol) dalla funzione di inserimento
   e dalla funzione di ricerca.
   */

#ifndef SYMBOL__H__
#define SYMBOL__H__


#include "commons.h"
#include "block.h"

class Symbol
{
    static Symbol* instance;

    map<string,Block*> table;

    Symbol() { /*cout << "Creato singleton Symbol\n";*/ }
    Symbol( const Symbol& );
    Symbol& operator=( const Symbol& );
    class Inner
    {
	public:
	    Inner() {}
	    ~Inner() { /*cout << "Distrutto singleton Symbol\n";*/ }
    };
    static Inner inn;
    friend class Inner;
    public:
    static Symbol* getInstance();
    bool insertData( string key, Block* bp );
    Block* findData( const string& key );
    bool eraseData( const string& key );
    bool getBlocks();  // restituisce tutti i blocchi nella mappa copiandoli nell'array blockList
    void printAll(); // stampa il contenuto dell'intera mappa
    int tableSize() { return table.size(); }
};

#endif
