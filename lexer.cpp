#include <algorithm>
#include "lexer.h"

extern ostringstream oss;

Lexer Lexer::instance; // richiama il costruttore dell'oggetto Singleton (indipendentemente dal fatto che venga usato o meno )

/* il costruttore di Lexer apre il file di configurazione della grammatica,
   carica la grammatica in memoria e poi apre il file di input del programma */
Lexer::Lexer()
{ 
    try
    {
	//cout << "Creato Singleton Lexer!\n";
	// apre, legge, carica in memmoria e chiude il file characters.conf
	ifstream grammar( "characters.txt" );
	if ( !grammar ) throw LexError( "Can't find 'characters.conf' file" );
	int i = 0, sz;
	for ( ; i<NUM_CHARS; i++ )
	    characters[i] = false;   
	string wstr0;
	getline( grammar, wstr0 ); // legge la prima linea che contiene i caratteri ammessi
	sz = wstr0.size();
	for ( i=0; i<sz; i++ )  // inizializza il vettore "characters"
	    if ( !isspace( wstr0[i] ) )
		characters[ static_cast<int>( wstr0[i] ) ] = true;
	wstr0.clear();
	grammar.close();
	grammar.clear(); // fondamentale!

	if ( DEBUG ) { cout << "Caratteri ammessi:  "; for ( i = 0; i<NUM_CHARS; i++ ) if ( characters[i] ) cout << static_cast<char>( i ); cout << "\n"; }

	// apre, legge, carica in memoria e chiude il file basic_keywords.conf
	grammar.open( "basic_keywords.txt" );
	if ( !grammar ) throw LexError( "Can't find 'basic_keywords.conf' file" );
	int line_counter = 0;
	string wstr1;
	for( ;; ) 
	{
	    getline( grammar, wstr0 );
	    line_counter++;
	    sz = wstr0.size();
	    i = 0;
	    while ( i < sz && isspace( wstr0[i] ) ) { i++; }
	    if ( i == sz ) //caso in cui la linea è vuota oppure un insieme di spazi
	    {
		cout << ">> Warning: 'basic_keywords.conf' contains an empty row <<\n";
		continue;
	    }
	    while ( i < sz && !isspace( wstr0[i] ) )
	    {
		if ( !isalpha( wstr0[i] ) )
		{
		    oss << "Keyword at line " << line_counter << " contains non alphabetic character '" << wstr0[i] << "'";
		    throw LexError( oss.str() );
		}
		wstr1.push_back( wstr0[i++] );
	    }
	    keywords.push_back( wstr1 );
	    wstr1.clear();
	    if ( grammar.eof() )
		break;
	}
	if ( keywords.size() == 0 ) throw LexError( "No keyword individualized in 'basic_keywords.conf'" );
	sort( keywords.begin(), keywords.end() );
	grammar.close();
	grammar.clear();

	if ( DEBUG ) { cout << "Keywords:  "; for ( i = 0; i<keywords.size(); i++ ) cout << keywords[i] << " "; cout << "\n"; }

	// apre, legge, carica in memoria solo la prima parola di ogni riga e poi chiude il file standard_blocks.conf
	grammar.open( "standard_blocks.txt" );
	if ( !grammar ) throw LexError( "Can't find 'standard_blocks.conf' file" );
	line_counter = 0;
	for( ;; ) 
	{
	    getline( grammar, wstr0 );
	    line_counter++;
	    sz = wstr0.size();
	    i = 0;
	    while ( i < sz && isspace( wstr0[i] ) ) { i++; }
	    if ( i == sz ) //caso in cui la linea è vuota oppure un insieme di spazi
	    {
		cout << ">> Warning: 'standard_blocks.conf' contains an empty row <<\n";
		continue;
	    }
	    while ( i < sz && !isspace( wstr0[i] ) )
	    {
		if ( !isalpha( wstr0[i] ) && wstr0[i] != '_' )
		{
		    oss << "in 'standard_blocks.conf': standard block name at line " << line_counter << " does not contain '_' or an alphabetic character";
		    throw SyxError( oss.str() ) ;
		}
		wstr1.push_back( wstr0[i++] );
	    }
	    standardBlocks.push_back( wstr1 );
	    wstr1.clear();
	    if ( grammar.eof() )
		break;
	}
	if ( standardBlocks.size() == 0 ) throw LexError( "No keyword individualized in 'basic_keywords.conf'" );
	sort( standardBlocks.begin(), standardBlocks.end() );
	grammar.close();
	grammar.clear();
	if ( DEBUG ) { cout << "Blocchi standard:  "; for ( i = 0; i<standardBlocks.size(); i++ ) cout << standardBlocks[i] << " "; cout << "\n"; }         
    }
    catch ( const GenericError& ) 
    { 
	exit( 0 );
    }
}

bool Lexer::setInputFile( string fn )
{
    fileName = fn;
    inputFile.open( fileName.c_str() ); // apre il file di ingresso dei dati dell'utente
    if ( !inputFile ) 
    {
	oss << "Can't find file '" << fileName << "'";
	throw LexError( oss.str() );
    }
}


Token* Lexer::getToken()
{
    if ( fileName == "" )
	throw LexError( "Lexer need a file to extract tokens" );
    if ( inputFile.eof() ) // prima di chiudere effettua le operazioni di pulizia 
    {
	inputFile.close();
	inputFile.clear();
	fileName = "";
	return null;
    }
    string wstr;
    int sz = 0;
    char c = inputFile.get();
    while ( !inputFile.eof() && isspace( c ) ) { c = inputFile.get(); } // spazi iniziali
    if ( inputFile.eof() ) // caso in cui ci siano solo spazi fino alla fine del file
    {
	inputFile.close();
	inputFile.clear();
	fileName = "";
	return null;
    }

    // se il primo carattere letto (diverso dallo spazio) non è compatibile con un identificatore, un numero oppure una keyword...  
    if ( !isalpha( c ) && !isdigit( c ) && c != '_' )
	if ( characters[c] )
	    return new Separator( c );
	else
	{
	    oss << "Unrecognized token: '" << c << "'";
	    throw LexError( oss.str() );
	}
    // se il primo carattere letto (diverso dallo spazio) è una cifra allora vuol dire che ci apprestiamo a leggere un numero
    if ( isdigit( c ) )
    {
	bool isDouble = false;
	char p;
	while ( !inputFile.eof() && ( isdigit( c ) || ( c == '.' && !isDouble ) ) ) //legge solo cifre e al massimo un punto ( '.' )
	{
	    if ( c == '.' ) // quando trova un punto sbircia in avanti per vedere se il successivo carattere p è una cifra
	    {

		p = inputFile.peek();
		if ( !isdigit( p ) ) // se p non è una cifra deve rimettere il '.' nello stream
		    break;
		isDouble = true;
	    }
	    wstr.push_back( c );        
	    c = inputFile.get();
	}
	/* se ha letto il primo carattere del prossimo token (dopo l'ultima cifra)
	   lo rimette nello stream */
	if ( !inputFile.eof() && !isspace( c ) )
	    inputFile.putback( c );
	if ( isDouble )
	    return new Double( wstr );
	else
	    return new Integer( wstr );
    }
    // altrimenti siamo nel caso di keyword, blocco standard, oppure identificatore
    bool canBeKeyword = true; // vero se il token che si sta attualmente estraendo potrà essere una keyword o un nome di blocco standard
    while ( !inputFile.eof() && ( isalpha( c ) || isdigit( c ) || c == '_' ) )
    {
	if ( !isalpha( c ) && c != '_' )
	    canBeKeyword = false;
	wstr.push_back( c );
	c = inputFile.get();
    }
    if ( !inputFile.eof() && !isspace( c ) )
	inputFile.putback( c );
    if ( canBeKeyword )
    {
	vector<string>::iterator vit = find( keywords.begin(), keywords.end(), wstr );
	if ( vit != keywords.end() )
	    return new Keyword( wstr );
	vit = find( standardBlocks.begin(), standardBlocks.end(), wstr );
	if ( vit != standardBlocks.end() )
	    return new StandardBlock( wstr );
    }
    return new Identifier( wstr );
}
