/* file "token.h"
   Contiene le dichiarazioni relative alla gerarchia di elementi grammaticali
   utilizzata dal simulatore.
   */

#ifndef TOKEN__H__
#define TOKEN__H__

#include <stdlib.h>
#include "commons.h"
#include "error.h"

/* la classe Token rappresenta un elemento grammaticale: la gerarchia non è pensata
   per essere espansa in futuro */      
class Token
{
    public:
	virtual string getName() = 0;
	virtual bool isIdentifier() = 0;
	virtual bool isKeyword() = 0;
	virtual bool isStandardBlock() = 0;
	virtual bool isSeparator() = 0;
	virtual bool isNumber() = 0;
	virtual ~Token() { /*cout << "Distrutto oggetto token\n";*/ }
};

class Identifier: public Token
{
    string name;
    public:
    Identifier( string nm ) : name( nm ) {}
    virtual string getName() { return name; }
    virtual bool isIdentifier() { return true; }
    virtual bool isKeyword() { return false; }
    virtual bool isStandardBlock() { return false; }
    virtual bool isSeparator() { return false; }
    virtual bool isNumber() { return false; }
};

class Keyword: public Token
{
    string name;
    public:
    Keyword( string nm ) : name( nm ) {}
    virtual string getName() { return name; }
    virtual bool isIdentifier() { return false; }
    virtual bool isKeyword() { return true; }
    virtual bool isStandardBlock() { return false; }
    virtual bool isSeparator() { return false; }
    virtual bool isNumber() { return false; }
};

class StandardBlock: public Token
{
    string name;
    public:
    StandardBlock( string nm ): name( nm ) {}
    virtual string getName() { return name; }
    virtual bool isIdentifier() { return false; }
    virtual bool isKeyword() { return false; }
    virtual bool isStandardBlock() { return true; }
    virtual bool isSeparator() { return false; }
    virtual bool isNumber() { return false; }
};

class Separator: public Token
{
    char name;
    public:
    Separator( char nm ) : name( nm ) {}
    virtual string getName() { string s; s.push_back( name ); return s; }
    virtual bool isIdentifier() { return false; }
    virtual bool isKeyword() { return false; }
    virtual bool isStandardBlock() { return false; }
    virtual bool isSeparator() { return true; }
    virtual bool isNumber() { return false; }
};

class Number: public Token
{
    public:
	virtual string getName() = 0;
	virtual bool isIdentifier() { return false; }
	virtual bool isKeyword() { return false; }
	virtual bool isStandardBlock() { return false; }
	virtual bool isSeparator() { return false; }
	virtual bool isNumber() { return true; }
	virtual bool isInteger() = 0;
	virtual bool isDouble() = 0;
};

class Integer: public Number
{
    int value;
    string repr;
    public:
    Integer( string nm ) : repr( nm ), value( atoi( nm.c_str() ) ) {}
    virtual string getName() { return repr; }
    int getValue() { return value; }
    virtual bool isInteger() { return true; }
    virtual bool isDouble() { return false; }
};

class Double: public Number
{
    double value;
    string repr;
    public:
    Double( string nm ) : repr( nm ), value( atof( nm.c_str() ) ) {}
    virtual string getName() { return repr; }
    double getValue() { return value; }
    virtual bool isInteger() { return false; }
    virtual bool isDouble() { return true; }
};

typedef list<Token*>::iterator TKPTVCIT; // non sono sicuro che sia veramente un miglioramento..!

#endif
