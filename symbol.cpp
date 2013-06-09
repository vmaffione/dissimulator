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

#include "symbol.h"

Symbol* Symbol::instance = null;

Symbol::Inner Symbol::inn;

Symbol* Symbol::getInstance()
{
    if (instance == null)
	instance = new Symbol;
    return instance;
}

/* Inserisce la tiga richiesta nella tabella dei simboli: restituisce true se
   l'inserimento è andato a buon fine, false se invece nella tabella esisteva
   già un elemento con la stessa chiave.
   */
bool Symbol::insertData(string key, Block* bp)
{
    pair< map<string,Block*>::iterator, bool> p = table.insert(make_pair(key, bp));
    return p.second;
}

/* cerca la riga nella tabella corrispondente ad una determinata chiave; se la
   trova restituisce il puntatore al blocco (cioè il contenuto della tabella),
   altrimenti restituisce il valore nullo; */
Block* Symbol::findData(const string& key)
{
    map<string,Block*>::iterator mit = table.find(key);
    if (mit == table.end())
	return null;
    return mit->second;
}

/* elimina una riga della tabella, preoccupandosi anche di liberare la memoria
   ai blocchi puntati */
bool Symbol::eraseData(const string& key)
{
    map<string,Block*>::iterator mit = table.find(key); // questa ricerca va a colpo sicuro
    delete mit->second;
    table.erase(mit);
    return true;
}

bool Symbol::getBlocks()
{
    map<string,Block*>::iterator mit = table.begin();
    for (; mit!=table.end(); mit++)
	/* i blocchi che sono modello di superblocco e i superblocchi con identificatore non devono essere messi nella lista dei
	   blocchi da processare, bensì nell'altra lista */
	if (mit->second->getRoleType() != SUPERBLOCK && mit->second->getName().substr(0,1) != "6")
	    Block::blockList.push_back(mit->second);
	else
	    SuperBlock::insertModel(mit->second);
    if (Block::blockList.size() == 0) return false;
    return true;
}

void Symbol::printAll()
{
    cout << " IDENTIFICATORI NELLA TABELLA DEI SIMBOLI:\n";
    map<string,Block*>::iterator mit = instance->table.begin();
    for (; mit!=instance->table.end(); mit++)
	mit->second->print();
    cout << "\n";
}
