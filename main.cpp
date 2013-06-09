#include "lexer.h"
#include "syntax.h"
#include "symbol.h"
#include "engine.h"
#include "console.h"

ostringstream oss;

int main()
{  
  list<Token*> TokenList;
  try
    {
      Lexer* LexAnalyzer = Lexer::getInstance(); // acquisice il puntatore all'unica istanza di Lexer   
      Token* nextToken;
      LexAnalyzer->setInputFile( "input.txt" );
      while ( nextToken = LexAnalyzer->getToken() )
        TokenList.push_back( nextToken );
      if ( DEBUG ) { TKPTVCIT it = TokenList.begin(); cout << "\nLista dei Token:\n"; for ( ; it != TokenList.end(); it++ ) { cout << ( *it )->getName(); if ( ( *it )->getName() == ";" ) cout << "\n"; } cout << "\n"; }    

      Syntax::init();
        
      Syntax SyxAnalyzer( &TokenList );
      TKPTVCIT b,e;
      
      while ( SyxAnalyzer.getInstruction( b, e ) )
        SyxAnalyzer.interpret( b, e );

      // la lista dei token non serve più, quindi si libera la memoria occupata
      for ( list<Token*>::iterator j = TokenList.begin(); j != TokenList.end(); j++ )
        delete ( *j );
      TokenList.clear();
      
      Symbol* symbolTablePointer = Symbol::getInstance();
      
      if ( DEBUG ) symbolTablePointer->printAll();
      
      // travaso i blocchi dalla tabella dei simboli nel vettore Block::blockList
      Syntax::getBlocks();      
      if ( DEBUG ) { cout << "Lista dei blocchi da ordinare:\n"; for ( int j = 0; j<Block::blockList.size(); j++ ) cout << "id= " << Block::blockList[j]->getID() << " " << Block::blockList[j]->getName() << "  " << Block::blockList[j]->getType() << "\n"; cout << "\n"; }
        
      SortEngine* sortEnginePointer = SortEngine::getInstance();
      // ordino il vettore Block::blockList
      sortEnginePointer->arrangeBlocks();
      if ( DEBUG ) { cout << "Lista ordinata:\n"; for ( int h=0; h<Block::blockList.size(); h++ ) cout << h << ") id = " << Block::blockList[h]->getID() << ", " <<  Block::blockList[h]->getName() << "\n"; cout << "\n"; }
        
      // lancio la console che determina l'interazione con l'utente
      Console* console = Console::getInstance();
      while ( console->readAndExecuteNext() ) { }
      
      // viene liberata la memoria relativa ai blocchi
      Block::freeTable();
    }
  catch ( const GenericError& e ) 
    {
      if ( e.getType() == LEX )
        {
          // dealloca la memoria occupata dalla lista di token
          for ( list<Token*>::iterator j = TokenList.begin(); j != TokenList.end(); j++ )
            delete ( *j );
        }
      else
        Block::freeTable();
    }
  return 0;
}
