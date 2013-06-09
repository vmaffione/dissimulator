/* file "lexer.h"
   Contiene la classe Lexer, che � un Singleton: essa rappresenta il modulo che si occupa di
   effettuare l'analisi lessicale dell'input.

   In una prima fase vengono letti alcuni file di configurazione: 'characters.conf',
   'standard_blocks.conf' e 'basic_keywords.conf' che permettono
   di personalizzare la grammatica dell'input, anche in modo limitato: in
   particolare devono esistere solo quattro tipi di token, cio� identificatori, keywords,
   nomi di blocchi standard della libreria e caratteri separatori
   (che corrispondono a elementi grammaticali a s� stanti). Non � possibile aggiungere altre
categorie: questo aspetto del sistema � molto rigido.
L'idea � invece che deve essere il pi� facile possibile aggiungere nuovi blocchi
alla libreria standard.
Nel file 'characters.conf' sono specificati su un'unica riga tutti i caratteri
ammessi come token a s� stanti.
Nel file 'basic_keywords.conf' sono presenti le keyword riconosciute dagli analizzatori:
ogni riga contiene una keyword diversa; il sistema non � pensato per rendere
semplice l'inserimento di una nuova keyword.
Il file 'standard_blocks.conf' � costituito da un insieme di righe; ogni riga
� composta di quattro parole:
- la prima parola indica il nome del blocco standard;
- la seconda parola indica la tipologia di blocco che pu� essere
STATEFUL (blocco con stato interno)
STATELESS (blocco senza stato interno)
SOURCE (blocco sorgente di segnale)
SINK (blocco di registrazione dell'uscita)
Valgono le seguenti regole:
- gli identificatori possono contere solo caratteri alfanumerici,
oltre al carattere '_', e non possono cominciare con una cifra;
- le keyword possono essere costituite esclusivamente da caratteri
alfabetici deve esistere almeno una keyword;
- i nomi dei blocchi standard possono essere costituiti esclusivamente da caratteri
alfabetici e dal carattere '_'; deve esistere almeno un blocco standard;

La classe Token rappresenta il generico token dell input. Da essa derivano tre
specializzazioni che implementano la sua interfaccia, le quali corrispondono
ai tre tipi di token di cui sopra.
*/

#ifndef LEXER__H__
#define LEXER__H__

#include "commons.h"
#include "error.h"

const int NUM_CHARS = 128; // numero di caratteri riconosciuti dall'analizzatore lessicale ( ASCII standard )

//class Token;
#include "token.h"

/* la classe Lexer � un Singleton che implementa l'interfaccia con cui questo
   modulo si presenta al resto dell'applicazione */
class Lexer
{
    private:
	static Lexer instance;

	string fileName;
	ifstream inputFile;
	bool characters[ NUM_CHARS ]; // mappa i caratteri di punteggiatura ammessi come elementi grammaticali a s� stanti
	vector<string> keywords; // keywords riconosciute dalla grammatica
	vector<string> standardBlocks; // nomi dei blocchi standard riconosciuti

	Lexer();
	Lexer( const Lexer& ); // nascondo
	Lexer& operator=( const Lexer& ); // nascondo
    public:
	~Lexer() { inputFile.close(); /*cout << "Distrutto Singleton Lexer!\n";*/ }
	static Lexer* getInstance() { return &instance; }
	Token* getToken(); // � l'unica vera funzione di interfaccia verso il modulo principale
	bool setInputFile( string fn );
};




#endif
