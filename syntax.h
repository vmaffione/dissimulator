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

/* file "syntax.h"

*/

#ifndef SYNTAX__H__
#define SYNTAX__H__

#include "commons.h"
#include "token.h"
#include "error.h"
#include "symbol.h"
#include "block.h"
#include "lexer.h"

struct BlockInfo
{
    int numParam;
    bool mi; // multi input
    bool mo; // multi output
    RoleType mType;  // candidato ad essere eliminato
};

/* la funzione Syntax::interpret restituisce un puntatore a questa struttura, che �
   significativo solo nel caso in cui si stia processando un file di definizione di superblocco */
struct ConnectionInfo
{
    bool internal; // ext oppure int
    union
    {
	struct
	{
	    Block* blockPointer;
	    bool in; // in oppure out
	    int bInd;
	    int sbInd;
	} externalIO;
	struct
	{
	    Block* srcPointer;
	    int srcIndex;
	    Block* destPointer;
	    int destIndex;
	} internalIO;
    } data;
};

class Syntax
{
    // puntatore alla lista di token da processare
    list<Token*>* tokenListPointer;
    // puntatore al token attualmente processato
    TKPTVCIT pointer;
    // numero dell'istruzione attualmente processata
    int current_instruction;
    /* per distinguere l'istanza relativa al file principale (istanza globale
       di Syntax dalle istanze chiamate a processare i file descrittori di
       superblocco  */
    bool superBlockInstance;
    /* viene mantenuto un contatore che serve ad associare ad ogni blocco del modello di superblocco
       un identificatore numerico da utilizzare nel matching */
    int superBlockIdCounter;

    string fileName; // nome del file che si sta analizzando

    static map<string,BlockInfo> blockLibrary;

    Syntax();
    Syntax( const Syntax& );
    Syntax( list<Token*>* tklst, string fn, bool sbi );
    Syntax& operator=( const Syntax& );
    public:
    static void init(); // inizializza il puntatore alla lista di blocchi
    static void getBlocks(); // restituisce la lista di blocchi risultato del parsing

    Syntax( list<Token*>* tklst );
    bool getInstruction( TKPTVCIT& begin, TKPTVCIT& end );
    ConnectionInfo* interpret( TKPTVCIT begin, const TKPTVCIT& end ); // restituisce null se l'istruzione non crea un blocco
};

#endif
