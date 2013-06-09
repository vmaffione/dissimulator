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
