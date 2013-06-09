#include <algorithm>
#include "syntax.h"

extern ostringstream oss;

map<string,BlockInfo> Syntax::blockLibrary; // mantiene informazioni sulle varie tipologie di blocchi facenti parte della libreria

Symbol* symbolTablePointer; // punta alla tabella dei simboli utilizzata in fase di analisi

BlockBuilder* blockBuilderPointer; // puntatore all'istanza di BlockBuilder

// funzione di utilità
void readNextWord( const string& source, int& esi, string& dest  )
{
    dest.clear();
    while ( esi < source.size() && isspace( source[esi] ) )
	esi++;
    while ( esi < source.size() && !isspace( source[esi] ) )
	dest.push_back( source[esi++] );
}

void Syntax::init()
{
    /* Legge per intero il file di configurazione 'standard_blocks.conf',
       caricandole in memoria in una mappa associativa. Tali informazioni
       serviranno nella fase di analisi sintattica.
       */
    ifstream libFile( "standard_blocks.txt" );
    if ( !libFile ) throw LexError( "Can't find 'standard_blocks.conf' file" );
    string wstr, name, macro_type, mimo, num_param;
    BlockInfo info;
    int line_counter = 0;
    for ( ;; )
    {
	int i = 0;
	getline( libFile, wstr );
	line_counter++;
	readNextWord( wstr, i, name );
	readNextWord( wstr, i, macro_type );
	readNextWord( wstr, i, mimo );
	readNextWord( wstr, i, num_param );
	if ( macro_type == "STATEFUL" )
	    info.mType = STATEFUL;
	else if ( macro_type == "STATELESS" )
	    info.mType = STATELESS;      
	else if ( macro_type != "-" )
	{
	    oss << "In 'standard_blocks.txt' : unrecogized macro-type '" << macro_type << "' at line " << line_counter << "\n";
	    oss << "Use 'STATEFUL', 'STATELESS' or '-' (if the block type is a source or a sink)";
	    throw SyxError( oss.str() );
	}
	const int NUM_CLASSES = 8; // le possibili classi input-output sono cablate per semplicità
	string classes[NUM_CLASSES] = { "NISO", "NIMO", "SINO", "SISO", "SIMO", "MINO", "MISO", "MIMO" };
	bool found = false;
	for ( int i=0; i<NUM_CLASSES; i++ )
	    if ( mimo == classes[i] )
		found = true;
	if ( !found )
	{
	    oss << "In 'standard_blocks.txt' : you must use one of the following classes:\n"; 
	    oss << "{ NISO, NIMO, SINO, SISO, SIMO, MINO, MISO, MIMO } instead of '" << mimo << "' at line " << line_counter;
	    throw SyxError( oss.str() );
	}
	info.mi = ( mimo == "MINO" || mimo == "MISO" || mimo == "MIMO" );
	info.mo = ( mimo == "NIMO" || mimo == "SIMO" || mimo == "MIMO" );
	if ( num_param == "V" ) // caso in cui i parametri sono variabili
	    info.numParam = -1; // '-1' significa convenzionalmente "numero imprecisato di parametri, tuttavia maggiore di 1"
	else
	{
	    info.numParam = atoi( num_param.c_str() );
	    if ( !info.numParam && num_param != "0" ) // se la stringa non si può convertire in intero
	    {
		oss << "In 'standard_blocks.txt' : you must use an integer instead of '" << num_param << "' at line " << line_counter;
		throw SyxError( oss.str() );
	    }
	}
	blockLibrary.insert( make_pair( name, info ) );
	if ( libFile.eof() )
	    break;   
    }
    libFile.close();

    symbolTablePointer = Symbol::getInstance(); //acquisisce l'oggetto Symbol ( DA RIVEDERE !!!! )

    if ( DEBUG ) { cout << "LIBRERIA DI BLOCCHI\n"; for ( map<string,BlockInfo>::iterator j=blockLibrary.begin(); j!=blockLibrary.end(); j++) cout << j->first << " "<<j->second.numParam<<"\n"; cout << "\n"; }

}

void Syntax::getBlocks()
{
    Symbol* symbolTablePointer = Symbol::getInstance();
    symbolTablePointer->getBlocks();
    Syntax::blockLibrary.clear();
    SuperBlock::freeTables();
}

Syntax::Syntax( list<Token*>* tklst ) : pointer( null ), current_instruction( 0 ), fileName( "input" ), superBlockInstance( false ), superBlockIdCounter( 0 )
{
    if ( symbolTablePointer == null )
	throw SyxError( "You must call Syntax::init() before using a Syntax object" );
    tokenListPointer = tklst;
    pointer = tokenListPointer->begin();
}

Syntax::Syntax( list<Token*>* tklst, string fn, bool sbi ) : pointer( null ), current_instruction( 0 ), fileName( fn ), superBlockInstance( sbi ), superBlockIdCounter( 0 )
{
    if ( symbolTablePointer == null )
	throw SyxError( "You must call Syntax::init() before using a Syntax object" );
    tokenListPointer = tklst;
    pointer = tokenListPointer->begin();
}

/* Legge una intera istruzione dalla lista e configura i parametri passati
   in modo che puntino rispettivamente al primo token della prossima
   istruzione ed al token contenente il ";" che indica la fine dell'struzione. 
   */
bool Syntax::getInstruction( TKPTVCIT& begin, TKPTVCIT& end )
{
    TKPTVCIT end_of_list = tokenListPointer->end();
    if ( pointer == end_of_list )
    {
	begin = end = end_of_list;
	return false;
    }
    begin = end = pointer;
    do
	pointer++;
    while ( pointer != end_of_list && ( *pointer )->getName() != ";" );
    current_instruction++;
    if ( pointer == end_of_list )
    {
	oss << "';' expected at the end of instruction #" << current_instruction;
	throw SyxError( oss.str() );
    }
    end = pointer++;
    return true;
}

/* interpreta ed esegue un'istruzione: se questa causa la creazione di un blocco
   allora ritorna il puntatore a detto blocco, altrimenti ritorna il valore nullo;
   l'iteratore 'scanner' punta al primo token dell'istruzione, mentre l'iteratore
   'end' punta al token contenente il ';' che termina l'istruzione;  */
ConnectionInfo* Syntax::interpret( TKPTVCIT scanner, const TKPTVCIT& end )
{             
    if ( scanner == end )
	return null;
    TKPTVCIT pBegin;
    ConnectionInfo* result = null;
    /* se l'istruzione inizia con un identificatore, allora l'analizzatore si
       aspetta che l'istruzione sia un assegnamento */
    if ( ( *scanner )->isIdentifier() )
    {
	/* se si sta effettuando il parsing di un superblocco, per convenzione viene
	   aggiunto il carattere '6' in testa all'identificatore, per distinguere
	   i blocchi definiti in un file di definizione di superblocco dai blocchi
	   definiti nel file di input principale */
	string id_name = ( superBlockInstance ) ? ( string("6") + fileName + ( *scanner )->getName() ) : ( *scanner )->getName();
	Block* newb; // destinato a contenere il puntatore al nuovo blocco creato
	map<string,BlockInfo>::iterator mit;
	BlockInfo info;
	scanner++;
	if ( scanner == end || ( *scanner )->getName() != "=" )
	{
	    oss << "In file '" << fileName << "': assignment '=' expected after identifier '" << id_name << "' at instruction #" << current_instruction;
	    throw SyxError( oss.str() );
	}
	scanner++;

	/* se trova un altro identificatore dopo il token "=" allora lo 
	   interpreta come nome di superblocco */
	if ( scanner != end && ( *scanner )->isIdentifier() )
	{
	    string superBlockName = ( *scanner )->getName();
	    // controlla se il superblocco è già stato caricato in memoria (dunque se è presente nella mappa)
	    SuperBlock::SuperBlockElement* sbep = SuperBlock::findSBEP( superBlockName );

	    if ( sbep == null )  // se non c'è allora deve effettuare il caricamento
	    {
		Lexer* LexAnalyzer = Lexer::getInstance(); // acquisice il puntatore all'unica istanza di Lexer
		list<Token*> TokenList;
		Token* nextToken;
		try
		{
		    LexAnalyzer->setInputFile( superBlockName + ".diss" );
		    while ( nextToken = LexAnalyzer->getToken() )
			TokenList.push_back( nextToken );

		    Syntax SyxAnalyzer( &TokenList, superBlockName, true );
		    TKPTVCIT b,e;
		    ConnectionInfo* connInfo;
		    sbep = new SuperBlock::SuperBlockElement; // nuovo elemento della tabella dei superblocchi
		    while ( SyxAnalyzer.getInstruction( b, e ) )
		    {
			connInfo = SyxAnalyzer.interpret( b, e );
			if ( connInfo ) // se c'è un risultato
			    if ( connInfo->internal )  // se le informazioni di connessione sono relative a connessioni interne
			    {                              
				SuperBlock::SuperBlockElement::InternalConnectionElement ice = { connInfo->data.internalIO.srcPointer->getSuperBlockID(), connInfo->data.internalIO.srcIndex, connInfo->data.internalIO.destPointer->getSuperBlockID(), connInfo->data.internalIO.destIndex };
				sbep->internalConnectionsTable.push_back( ice );
				delete connInfo;
			    }
			    else  // se le connessioni di informazioni riguardano la creazione di un blocco oppure le connessioni esterne
				if ( connInfo->data.externalIO.bInd == -1 )// solo il campo blockPointer è significativo
				{                           
				    sbep->superBlockBlocks.push_back( connInfo->data.externalIO.blockPointer );
				    delete connInfo;
				}
				else // se l'esecuzione della connect riporta informazioni di ingresso/uscita relative al superblocco..
				{
				    SuperBlock::SuperBlockMatchingRow sbmr = { connInfo->data.externalIO.sbInd, connInfo->data.externalIO.blockPointer->getSuperBlockID(), connInfo->data.externalIO.bInd };
				    if ( connInfo->data.externalIO.in ) 
					sbep->inputSuperBlockMatchingTable.push_back( sbmr );
				    else
					sbep->outputSuperBlockMatchingTable.push_back( sbmr );
				    delete connInfo;
				}

		    }

		    // viene creato un nuovo elemento della tabella dei superblocchi
		    SuperBlock::insertSBEP( superBlockName, sbep );

		    TKPTVCIT i = TokenList.begin();
		    for ( ; i != TokenList.end(); i++ )
			delete ( *i );

		}
		catch ( const GenericError& ) 
		{
		    for ( list<Token*>::iterator j = TokenList.begin(); j != TokenList.end(); j++ )
			delete ( *j );
		    TokenList.clear();
		    throw;
		}    

	    }

	    // si crea una nuova istanza del superblocco riconosciuto
	    int j = 0,
		sz = sbep->superBlockBlocks.size();
	    /* a questo punto bisogna creare delle copie di tutti i blocchi che costituiscono
	       il modello di superblocco; per fare ciò viene chiamata la funzione virtuale "clone()"
	       che si occupa in sostanza di creare un nuovo oggetto dello stesso tipo chiamando
	       il costruttore di copia giusto; il corpo del costruttore di copia deve 
	       (se necessario) effettuare le operazioni di allocazione della memoria privata
	       del blocco (cioè quella eventualmente allocata al terzo livello della gerarchia
	       da chi ha progettato il tipo di blocco in questione) e richiamare
	       il costruttore di copia sulla classe base virtuale Block, tramite la lista
	       di inizializzazione; deve essere chiamato anche il corretto costruttore di
	       copia per le classi intermedie della gerarchia che si occupa di
	       inizializzare i vari vector dimensionandoli opportunamente a seconda del caso,
	       ma lasciando i puntatori in uno stato non significativo, perchè verranno
	       poi impostati successivamente da una "create_connection" */

	    Block** match = new Block*[ sz ];   
	    /* sfruttando il fatto che i blocchi modello di superblocco vengono creati con superblock ID
	       crescente (partendo da 0), si crea un array "match" che, per come viene inizializzato, permetterà
	       successivamente di reperire il blocco clone corrispondente ad un determinato blocco modello
	       a partire dal superblock ID di quest'ultimo
	       */
	    for ( ; j<sz; j++ )
		match[ j ] = sbep->superBlockBlocks[j]->clone( superBlockInstance );                             

	    if ( DEBUG ) for ( j=0; j<sz; j++ ) { cout << "match["<< j << "] = " << match[j]->getID() << "\n"; }

	    int isz = sbep->internalConnectionsTable.size() ;
	    /* partendo dalla tabella delle connessioni interne propria del superblocco in questione,
	       vengono create tutte le connessioni interne tra tutti i blocchi appena clonati,
	       per mezzo dell'array match */
	    for ( j=0; j<isz; j++ )
		create_connection( match[ sbep->internalConnectionsTable[j].srcSuperBlockID ], sbep->internalConnectionsTable[j].srcIndex, match[ sbep->internalConnectionsTable[j].destSuperBlockID ], sbep->internalConnectionsTable[j].destIndex, -1 );

	    /* a questo punto si crea un nuovo blocco di tipo SuperBlock e, sempre utilizzando
	       l'array match, si impostano le sue tabelle di redirezione di ingresso e uscita sulla base
	       delle informazioni presenti nelle tabelle di ingresso/uscita dell'elemento di superblocco  */
	    int irsz = sbep->inputSuperBlockMatchingTable.size(),
		orsz = sbep->outputSuperBlockMatchingTable.size();
	    vector<SuperBlock::RedirectElement> inputRedirectTable( irsz );
	    vector<SuperBlock::RedirectElement> outputRedirectTable( orsz );
	    for ( j=0; j<irsz; j++ )
	    {
		inputRedirectTable[j].sbIndex = sbep->inputSuperBlockMatchingTable[j].sbIndex;
		inputRedirectTable[j].blockPointer = match[ sbep->inputSuperBlockMatchingTable[j].sbBlockId ];
		inputRedirectTable[j].sbBlockIndex = sbep->inputSuperBlockMatchingTable[j].sbBlockIndex;
	    }
	    /* per esigenze della funzione SuperBlock::getInputPointers l'array inputRedirectTable deve essere ordinata per indirizzo di blocco */                    
	    sort( inputRedirectTable.begin(), inputRedirectTable.end() );

	    for ( j=0; j<orsz; j++ )
	    {
		outputRedirectTable[j].sbIndex = sbep->outputSuperBlockMatchingTable[j].sbIndex;
		outputRedirectTable[j].blockPointer = match[ sbep->outputSuperBlockMatchingTable[j].sbBlockId ];
		outputRedirectTable[j].sbBlockIndex = sbep->outputSuperBlockMatchingTable[j].sbBlockIndex;
	    }
	    /* se l'istanza corrente di Syntax è quella che analizza il file principale, allora
	       si devono travasare tutti i blocchi clonati (ma non i superblocchi clonati) nella
	       lista globale dei blocchi; se invece stiamo analizzando un file di definizione di
	       superblocco, travasiamo i blocchi clonati in un'altra lista, in modo da poter 
	       liberare la memoria al momento giusto */
	    for ( j=0; j<sz; j++ )
		if ( !superBlockInstance && match[j]->getRoleType() != SUPERBLOCK )
		    Block::blockList.push_back( match[j] );
		else
		    SuperBlock::insertModel( match[j] );

	    // elimina l'array match
	    delete [] match;  

	    if ( superBlockInstance )
		newb = new SuperBlock( superBlockIdCounter++, id_name, superBlockName, inputRedirectTable, outputRedirectTable );
	    else
		newb = new SuperBlock( 0, id_name, superBlockName, inputRedirectTable, outputRedirectTable );
	}



	else if ( !( *scanner )->isStandardBlock() )
	{
	    oss << "In file '" << fileName << "': standard block name or superblock name expected after '=' at instruction #" << current_instruction;
	    throw SyxError( oss.str() );
	}



	else  // ha riconosciuto il nome di un blocco della libreria
	{
	    // reperisce nella libreria le informazioni del tipo di blocco in questione
	    string micro_type = ( *scanner )->getName();
	    mit = blockLibrary.find( micro_type );
	    info = mit->second;
	    // legge la prima parentesi
	    scanner++;
	    if ( scanner == end || ( *scanner )->getName() != "(" )
	    {
		oss << "In file '" << fileName << "': expected '(' after block name at the end of instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    TKPTVCIT toDel;
	    pBegin = scanner;  // a questo punto scanner punta al primo token dopo la parentesi aperta
	    int rpm = 0; // numero di parametri

	    for ( ;; )
	    {
		if ( ( *scanner )->getName() == ")" ) // caso di costruttore di blocco senza parametri
		{
		    scanner++;
		    break;
		}
		if ( ( *scanner )->getName() == ","  )
		{
		    oss << "In file '" << fileName << "': parameter expected in block definition at instruction #" << current_instruction;
		    throw SyxError( oss.str() );
		}
		scanner++;
		rpm++;
		if ( ( *scanner )->getName() == "," )
		{
		    toDel = scanner++;
		    tokenListPointer->erase( toDel ); // cancella i token "," per rendere più agevole la scrittura dei costruttori dei blocchi
		}
		else if ( ( *scanner )->getName() == ")" )
		{
		    scanner++;
		    break;
		}
		else
		{
		    while ( scanner != end )
		    {
			if ( ( *scanner )->getName() == ")" )
			{
			    oss << "In file '" << fileName << "': illegal parameter at instruction #" << current_instruction;
			    throw SyxError( oss.str() );
			}
			scanner++;
		    }
		    oss << "In file '" << fileName << "': expected ')' at the end of instruction #" << current_instruction;
		    throw SyxError( oss.str() ) ;
		}
	    }
	    if ( info.numParam >= 0 && rpm != info.numParam ) // info.numParam >=0   <==>  i parametri sono in numero preciso
	    {  
		oss << "In file '" << fileName << "': block constructor at instruction #" << current_instruction << " needs exactly " << info.numParam << " parameter(s)";
		throw SyxError( oss.str() );
	    }
	    // ora scanner punta al token successivo alla parentesi chiusa ")"
	    if ( scanner != end )
	    {
		oss << "In file '" << fileName << "': expected ';' after ')' at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }

	    blockBuilderPointer = BlockBuilder::getInstance();
	    // se stiamo analizzando una definizione di superblocco, allora assegniamo il prossimo ID di superblocco disponibile
	    if ( superBlockInstance )
		newb = blockBuilderPointer->build( superBlockIdCounter++, id_name, micro_type, pBegin, current_instruction );        
	    else  // altrimenti per convenzione passiamo 0 ( non significativo )
		newb = blockBuilderPointer->build( 0, id_name, micro_type, pBegin, current_instruction );        
	}



	/* controlla nella tabella dei simboli se l'identificatore in questione è stato
	   già definito, ed in tal caso elimina il blocco precedente e crea uno nuovo,
	   sovrascrivendo l'entrata della tabella; se l'identificatore non era
	   ancora stato definito allora semplicemente lo aggiunge con relativo
	   blocco alla tabella dei simboli;  */

	Block* b = symbolTablePointer->findData( id_name );
	if ( b == null )
	    symbolTablePointer->insertData( id_name, newb );
	else
	{
	    symbolTablePointer->eraseData( id_name );
	    symbolTablePointer->insertData( id_name, newb );
	    cout << ">> Warning: Identifier <" << id_name << "> overwritten at instruction #" << current_instruction << "\n";
	}
	if ( superBlockInstance )
	{
	    result = new ConnectionInfo;
	    result->internal = false;
	    result->data.externalIO.blockPointer = newb;
	    result->data.externalIO.bInd = -1; // ad indicare che si tratta dell'istruzione di creazione di un nuovo blocco modello di superblocco
	}
	return result; // se siamo processando un superblocco è significativo solo il campo blockPointer
    }






    /* analizzatore del comando "connect" */
    else if ( ( *scanner )->isKeyword() && ( *scanner )->getName() == "connect" )
    { 
	string id_name_out, id_name_in;
	Block* pb_out;
	Block* pb_in;
	int index_out = 0, index_in = 0;
	/* lo zero di default è obbligatorio almeno per "index_out", per un corretto funzionamento
	   di tutto il meccanismo; infatti nella i-esima posizione dell'array "src_out" in un 
	   bloccho multi-input viene memorizzato l'indice dell'output del blocco sorgente a cui è
	   collegato l'i-esimo ingresso del blocco multi-input
	   */      
	map<string,BlockInfo>::iterator mit;
	bool mi, mo;
	scanner++;
	/* si aspetta una parentesi aperta */
	if ( scanner == end || ( *scanner )->getName() != "(" )
	{
	    oss << "In file '" << fileName << "': expected '(' after connect command name at instruction #" << current_instruction;
	    throw SyxError( oss.str() );
	}
	scanner++;

	/* ora si aspetta un identificatore, a meno che non stiamo processando una definizione di
	   superblocco, nel qual caso si aspetta un insieme di token del tipo "input[i]" */
	if ( superBlockInstance && ( *scanner )->getName() == "input" )
	{
	    scanner++;
	    int sbInputIndex;
	    // cerca '['
	    if ( ( *scanner )->getName() != "[" )
	    {
		oss << "In file '" << fileName << "': expected input index ( [ index ] ) in superblock definition file '" << fileName << "' in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    /* cerca un numero intero positivo*/
	    if ( scanner == end || !( *scanner )->isNumber() || !( static_cast<Number*>( *scanner ) )->isInteger() || ( sbInputIndex = ( static_cast<Integer*>( *scanner ) )->getValue() ) < 0 )
	    {
		oss << "In file '" << fileName << "', at instruction #" << current_instruction << ": index must be a positive integer";
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    // cerca ']'
	    if ( ( *scanner )->getName() != "]" )
	    {
		oss << "In file '" << fileName << "': expected ']' in index notation in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    // comincia a creare la struttura di ritorno, visto che ci troviamo in uno dei due casi
	    result = new ConnectionInfo;
	    result->internal = false;
	    result->data.externalIO.in = true;
	    result->data.externalIO.sbInd = sbInputIndex;
	}
	else if ( !( *scanner )->isIdentifier() )
	{
	    oss << "In file '" << fileName << "': expected identifier as first parameter of connect command at instruction #" << current_instruction;
	    throw SyxError( oss.str() );
	}
	else // se ha riconosciuto un identificatore
	{
	    id_name_out = ( superBlockInstance ) ? ( string( "6" ) + fileName + ( *scanner )->getName() ) : ( *scanner )->getName();
	    pb_out = symbolTablePointer->findData( id_name_out );
	    if ( pb_out == null )
	    {
		oss << "In file '" << fileName << "': unknown identifier '" << id_name_out << "' in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    else if ( pb_out->getRoleType() == SUPERBLOCK )
		mo = true;
	    else
	    {
		mit = blockLibrary.find( pb_out->getType() );
		mo = mit->second.mo;
	    }
	    scanner++;
	    /* poi si aspetta un punto '.' */
	    if ( scanner == end || ( *scanner )->getName() != "." )
	    {
		oss << "In file '" << fileName << "': expected output redirection (.output[i]) in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    /* si aspetta 'output' */
	    if ( scanner == end || ( *scanner )->getName() != "output" )
	    {
		oss << "In file '" << fileName << "': expected output redirection (.output[i]) in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    /* adesso, se il blocco è multi-output deve aspettarsi la notazione '[ index ]', altrimenti va semplicemente avanti */
	    if ( mo )
	    {
		/* cerca '[' */
		if ( scanner == end || ( *scanner )->getName() != "[" )
		{
		    oss << "In file '" << fileName << "': expected output index ( [ index ] ) with multi-output block '" << id_name_out << "' in connect command at instruction #" << current_instruction;
		    throw SyxError( oss.str() );
		}
		scanner++;
		/* cerca un numero intero positivo*/
		if ( scanner == end || !( *scanner )->isNumber() || !( static_cast<Number*>( *scanner ) )->isInteger() || ( index_out = ( static_cast<Integer*>( *scanner ) )->getValue() ) < 0 )
		{
		    oss << "In file '" << fileName << "': at instruction #" << current_instruction << ": index must be a positive integer";
		    throw SyxError( oss.str() );
		}
		scanner++;
		/* cerca ']' */
		if ( scanner == end || ( *scanner )->getName() != "]" )
		{
		    oss << "In file '" << fileName << "': expected ']' in index notation in connect command at instruction #" << current_instruction;
		    throw SyxError( oss.str() );
		}
		scanner++;
	    }
	}
	/* ora si aspetta la virgola ',' */
	if ( ( *scanner )->getName() != "," )
	{
	    if ( ( *scanner )->getName() == "[" )
		oss << "In file '" << fileName << "': at instruction #" << current_instruction << ": Block '" << id_name_out << "' is single-ouput: you can't use '[ index ]' notation";
	    else
		oss << "In file '" << fileName << "': expected second parameter in connect command at instruction #" << current_instruction;
	    throw SyxError( oss.str() );
	}
	scanner++;

	/* ora si aspetta il secondo identificatore, a meno che non stiamo processando la
	   definizione di un superblocco ed in tal caso possiamo riconoscere una espressione
	   del tipo "output[i]" ( e in quest'ultimo caso a meno che il primo parametro della
	   connect non fosse già un ingresso di superblocco, perchè non hanno senso comandi "connect"
	   del tipo "connect(input[i],output[j])" ) */
	if ( superBlockInstance && ( *scanner )->getName() == "output" && result == null )
	{
	    scanner++;
	    int sbOutputIndex;
	    // cerca '['
	    if ( ( *scanner )->getName() != "[" )
	    {
		oss << "In file '" << fileName << "': expected output index ( [ index ] ) in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    /* cerca un numero intero positivo*/
	    if ( scanner == end || !( *scanner )->isNumber() || !( static_cast<Number*>( *scanner ) )->isInteger() || ( sbOutputIndex = ( static_cast<Integer*>( *scanner ) )->getValue() ) < 0 )
	    {
		oss << "In file '" << fileName << "', at instruction #" << current_instruction << ": index must be a positive integer";
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    // cerca ']'
	    if ( ( *scanner )->getName() != "]" )
	    {
		oss << "In file '" << fileName << "': expected ']' in index notation in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    // comincia a creare la struttura di ritorno, visto che ci troviamo in uno dei due casi
	    result = new ConnectionInfo;
	    result->internal = false;
	    result->data.externalIO.in = false;
	    result->data.externalIO.sbInd = sbOutputIndex;
	}   
	else if ( !( *scanner )->isIdentifier() )
	{
	    oss << "In file '" << fileName << "': expected identifier as second parameter of connect command at instruction #" << current_instruction;
	    throw SyxError( oss.str() );
	}
	else // se ha riconosciuto un identificatore
	{
	    id_name_in = ( superBlockInstance ) ? ( string( "6" ) + fileName + ( *scanner )->getName() ) : ( *scanner )->getName();
	    pb_in = symbolTablePointer->findData( id_name_in );
	    if ( pb_in == null )
	    {
		oss << "In file '" << fileName << "': unknown identifier '" << id_name_in << "' in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    else if ( pb_in->getRoleType() == SUPERBLOCK )
		mi = true;
	    else
	    {
		mit = blockLibrary.find( pb_in->getType() );
		mi = mit->second.mi;
	    }
	    scanner++;
	    /* aspetta un punto '.' */
	    if ( ( *scanner )->getName() != "." )
	    {
		oss << "In file '" << fileName << "': expected input redirection (.input[i]) in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    /* si aspetta 'input' */
	    if ( scanner == end || ( *scanner )->getName() != "input" )
	    {
		oss << "In file '" << fileName << "': expected output redirection (.input[i]) in connect command at instruction #" << current_instruction;
		throw SyxError( oss.str() );
	    }
	    scanner++;
	    /* adesso, se il blocco è multi-input deve aspettarsi la notazione '[ index ]', altrimenti va semplicemente avanti */
	    if ( mi )
	    {
		/* cerca '[' */
		if ( scanner == end || ( *scanner )->getName() != "[" )
		{
		    oss << "In file '" << fileName << "':expected input index ( [ index ] ) with multi-input block '" << id_name_in << "' in connect command at instruction #" << current_instruction;
		    throw SyxError( oss.str() );
		}
		scanner++;
		/* cerca un numero intero positivo*/
		if ( scanner == end || !( *scanner )->isNumber() || !( static_cast<Number*>( *scanner ) )->isInteger() || ( index_in = ( static_cast<Integer*>( *scanner ) )->getValue() ) < 0 )
		{ 
		    cout<<( *scanner )->getName()<<( index_in = ( static_cast<Integer*>( *scanner ) )->getValue() );
		    oss << "In file '" << fileName << "': at instruction #" << current_instruction << ": index must be a positive integer";
		    throw SyxError( oss.str() );
		}
		scanner++;
		/* cerca ']' */
		if ( scanner == end || ( *scanner )->getName() != "]" )
		{
		    oss << "In file '" << fileName << "':expected ']' in index notation in connect command at instruction #" << current_instruction;
		    throw SyxError( oss.str() );
		}
		scanner++;
	    }
	}
	/* cerca la parentesi di chiusura */
	if ( ( *scanner )->getName() != ")" )
	{
	    if ( ( *scanner )->getName() == "[" )
		oss << "In file '" << fileName << "': at instruction #" << current_instruction << ": Block '" << id_name_in << "' is single-input: you can't use '[ index ]' notation";
	    else 
		oss << "In file '" << fileName << "': expected ')' at the end of connect command at instruction #" << current_instruction;
	    throw SyxError( oss.str() );
	}
	scanner++;
	if ( scanner != end )
	{
	    oss << "In file '" << fileName << "': expected ';' after ')' at instruction #" << current_instruction;
	    throw SyxError( oss.str() );
	}
	/* ora abbiamo estratto 'id_name_out', 'pb_out', 'index_out', 'id_name_in', 'pb_in' ed 'index_in' 
	   e dunque si può procedere con la connessione tra i due blocchi, a meno che non
	   stiamo analizzando una connect che si trova all'interno di una definizione di superblocco,
	   ed in tal caso non si deve creare la connessione; */

	if ( result == null ) // caso di connect nel file principale oppure connessione interna tra blocchi modello di un superblocco
	{
	    /* se la connect riguarda una connessione interna al superblocco (connessione tra blocchi modello), allora
	       alloca e prepara la struttura di ritorno, e non crea la connessione, in quanto non necessaria; */
	    if ( superBlockInstance )
	    {
		result = new ConnectionInfo;
		result->internal = true;
		result->data.internalIO.srcPointer = pb_out;
		result->data.internalIO.srcIndex = index_out;
		result->data.internalIO.destPointer = pb_in;
		result->data.internalIO.destIndex = index_in;
	    }
	    else  // altrimenti siamo nel caso di connessione nel blocc
		create_connection( pb_out, index_out, pb_in, index_in, current_instruction );
	}
	else // caso di connect nella definizione di superblocco che riguarda ingressi o uscite del superblocco stesso
	{
	    if ( result->data.externalIO.in ) // la connect riguarda l'input del superblocco
	    {
		result->data.externalIO.blockPointer = pb_in;
		result->data.externalIO.bInd = index_in;
	    }
	    else // la connect riguarda l'output del superblocco
	    {
		result->data.externalIO.blockPointer = pb_out;
		result->data.externalIO.bInd = index_out;
	    }
	}
	return result;
    }
    /* se entra qui vuol dire che non conosce il tipo di istruzione, quindi lancia una eccezione */
    else
    {
	oss << "What did you mean at instruction #" << current_instruction << "?";
	throw SyxError( oss.str() );
    }
}
