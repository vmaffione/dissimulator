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
