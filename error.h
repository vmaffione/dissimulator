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
