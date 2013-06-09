/* file "block.h"
   Dichiara la gerarchia di blocchi da cui è costituito il sistema.
*/

#ifndef BLOCK__H__
#define BLOCK__H__

#include "commons.h"
#include "token.h"
#include "error.h"
#include "time.h"

enum IOType { NONE = 0, SINGLE, MULTI };
enum RoleType { SOURCE = 0, STATEFUL, STATELESS, SINK, SUPERBLOCK  };
enum TransparenceType { TRANSPARENT = 0, NOT_TRANSPARENT };

class Block;

/* la classe BlockBuilder è un singleton, e si occupa sostanzialmente di inizializzare i vari tipi di blocco
   per conto del modulo "syntax": nella funzione "build" viene cablato il demultiplexing dei costruttori; tale classe
   è stata dunque introdotta per disaccoppiare, dal punto di vista della compilazione, i moduli "syntax" e "block",
   limitando quindi le modifiche necessarie per la definizione di un nuovo tipo di blocco al solo modulo "block"; */
class BlockBuilder
  {   
      friend class Block;
      friend class SuperBlock;
      static int id_counter;
      
      static BlockBuilder instance;
      BlockBuilder() { }
      BlockBuilder( const BlockBuilder& );
      Block& operator=( const BlockBuilder& );
    public:
      static BlockBuilder* getInstance() { return &instance; }
      Block* build( int sbid, const string& id_name, const string& micro_type, TKPTVCIT pBegin, int current_instruction );
  };

// classe base virtuale
class Block
  {   
      friend class SuperBlock;
      virtual bool addOutput( Block* dest, int dest_input_index, int output_index, int instructionCounter ) = 0; // !!! l'ultimo argomento è diventato obsoleto !!! ???
      virtual bool addInput( Block* src, int input_index, int src_output_index, int instructionCounter ) = 0;
      friend bool create_connection( Block* input, int input_index, Block* output, int output_index, int instructionCounter );
      virtual void real_print() const = 0; // completa la stampa chiamando la funzione ridefinita dalla classe completa
    protected:
      string microType;  // nome del tipo di blocco
      string name;  // identificatore del blocco
      int ID;  // identificatore numerico globale del blocco
      int sbID; // identificatore di superblocco
      virtual void initOutputDimension( int outDim ) = 0;
      virtual void initInputDimension( int outDim ) = 0;
      virtual bool outCheckAndClear() = 0;
      virtual bool inCheckAndClear() = 0;
      Block() {} // è necessario a causa delle classi del secondo livello della gerarchia, che non hanno costruttore
      Block( const Block& model );
    public:
      Block( int id, int sbid, string nm, string micro ): ID ( id ), sbID ( sbid ), name( nm ), microType( micro ) {}
      virtual ~Block() { /* cout << "Distruggo un oggetto blocco\n"; */ }  // è fondamentale per evitare un errore a run-time: questa definizione permette ai distruttori delle classi complete di essere correttamente implementati
      string getName() const { return name; }
      string getType() const { return microType; }
      int getID() const { return ID; }
      int getSuperBlockID() const { return sbID; }
      void setID( int id ) { ID = id; }
      virtual int outputDimension() const = 0; // restituisce la dimensione dell'uscita del blocco
      virtual int outputNumLinks() const = 0; // restituisce il numero di collegamenti in uscita del blocco
      virtual int inputDimension() const = 0; // restituisce il numero di collegamenti in ingresso del blocco e dimensione del blocco
      bool inoutCheckAndClear(); // controlla che input ed output siano stati assegnati e poi libera la memoria che non serve più
      virtual Block* getOutputPointer( int index ) const = 0; // resituisce il puntatore al blocco in uscita specificato dall'indice
      virtual vector<Block*> getOutputPointers() const = 0; // restituisce un vector contenente tutti i collegamenti in uscita del blocco
      virtual Block* getInputPointer( int index, int& src_index ) const = 0; // resituisce il puntatore al blocco in ingresso specificato dall'indice
      virtual IOType getOutputType() const = 0; // permette di distinguere i blocchi in base alla dimensione dell'uscita
      virtual IOType getInputType() const = 0;  // permette di distinguere i blocchi in base alla dimensione dell'ingresso
      virtual RoleType getRoleType() const = 0; // permette di distinguere i blocchi in base al loro "ruolo", ossia sorgente, blocco con o senza stato oppure uscita
      virtual TransparenceType getTransparenceType() const = 0; // permette di distinguere tra sistemi strettamente causali e sistemi causali ma non strettamente
      virtual double getInputValue( int index ) const = 0; // recupera dall'ingresso di indice 'index' il valore dell'input attuale
      virtual double getOutputValue( int src_index ) const = 0;
      virtual void refresh() = 0; // effettua un passo di evoluzione dell'uscita
      virtual void initOperation() = 0; // se necessario, inizializza il blocco
      virtual void endOperation() = 0; // se necessario, "assesta" il blocco
      virtual Block* clone( bool ) = 0; // clona sè stesso secondo una semantica adatta agli scopi
      void print() { if ( name != "" ) cout << "Name: " << name << ", "; cout << "ID: " << ID << ", "; this->real_print(); }
      
      static vector<Block*> blockList; // vettore adibito a contenere i puntatori ai blocchi che costituiscono il sistema
      static void freeTable();
  };

// implementa l'astrazione di superblocco
class SuperBlock: public Block
  {   
    public:
      struct RedirectElement
        {
          int sbIndex;
          Block* blockPointer; // puntatore al blocco del modello di superblocco a cui corrisponde l'input
          int sbBlockIndex;
          bool operator<( const RedirectElement& r ) const { return ( blockPointer < r.blockPointer ); }
        };
      /* questa struttura è la riga di una tabella che serve a creare le corrispondenze tra ingressi (o uscite)
         di un superblocco e ingressi (o uscite) dei blocchi che costituiscono il modello di tale superblocco  */  
      struct SuperBlockMatchingRow
        {
          int sbIndex;  // indice relativo all'ingresso o all'uscita del superblocco
          int sbBlockId;  // Id del blocco facente parte il modello di superblocco
          int sbBlockIndex; // indice relativo all'ingresso o all'uscita del blocco costituente il modello di superblocco
        };   
      struct SuperBlockElement
        {
          public:
            vector<Block*> superBlockBlocks; // contiene i puntatori ai blocchi modello che costituiscono il superblocco
            vector<SuperBlockMatchingRow> inputSuperBlockMatchingTable; // tiene traccia delle corrispondenze tra gli input del superblocco e gli input dei blocchi che costituiscono il modello
            vector<SuperBlockMatchingRow> outputSuperBlockMatchingTable; // tiene traccia delle corrispondenze tra gli output del superblocco e gli output dei blocchi che costituiscono il modello
            struct InternalConnectionElement
              {
                int srcSuperBlockID;
                int srcIndex;
                int destSuperBlockID;
                int destIndex;
              };
            vector<InternalConnectionElement> internalConnectionsTable;
        };
      static void insertModel( Block* bp ) { SuperBlock::blockModelsList.push_back( bp ); }
      static SuperBlockElement* findSBEP( const string& superBlockName ) { map<string, SuperBlock::SuperBlockElement*>::iterator mit = SuperBlock::superBlocksTable.find( superBlockName ); return ( ( mit == SuperBlock::superBlocksTable.end() ) ? null : mit->second ); }
      static void insertSBEP( const string& superBlockName, SuperBlockElement* sbep ) { SuperBlock::superBlocksTable.insert( make_pair( superBlockName, sbep ) ); }
      static void freeTables();
      
    private:
      static map<string, SuperBlockElement*> superBlocksTable; // tabella dei modelli di superblocco
      static std::vector<Block*> blockModelsList; // adibito a contenere tutti i puntatori a blocco modello, permettendo di liberare la relativa memoria
      
      vector<RedirectElement> inputRedirectTable;
      vector<RedirectElement> outputRedirectTable;
      friend bool create_connection( Block* input, int input_index, Block* output, int output_index, int instructionCounter );
      virtual bool addOutput( Block* dest, int dest_input_index, int output_index, int instructionCounter ) { }
      virtual bool addInput( Block* src, int input_index, int src_output_index, int instructionCounter ) { }
      SuperBlock( const SuperBlock& model ): Block( model ) { }  
    protected:
      virtual void initOutputDimension( int outDim ) { };
      virtual void initInputDimension( int outDim ) { };
      virtual bool outCheckAndClear() { }
      virtual bool inCheckAndClear() { }
      virtual void real_print() const { cout << "Superblock type " << this->getType() << "\n"; }
    public:  
      SuperBlock( int sbid, string nm, string superBlockName, const vector<RedirectElement>& irt, const vector<RedirectElement>& ort ): Block( BlockBuilder::id_counter++, sbid, nm, superBlockName ), inputRedirectTable( irt ), outputRedirectTable( ort ) { }
      virtual int outputDimension() const { }
      virtual int outputNumLinks() const { }
      virtual int inputDimension() const { }
      virtual Block* getOutputPointer( int index ) const { }
      virtual vector<Block*> getOutputPointers() const { }
      virtual Block* getInputPointer( int index, int& src_index ) const { }
      virtual IOType getOutputType() const { }
      virtual IOType getInputType() const { }
      virtual RoleType getRoleType() const { return SUPERBLOCK; }
      virtual TransparenceType getTransparenceType() const { }
      virtual double getInputValue( int index ) const { }
      virtual double getOutputValue( int src_index ) const { }
      virtual void refresh() { }
      virtual void initOperation() { }
      virtual void endOperation() { }
      virtual Block* clone( bool superBlockInstance ); // la clone per un superblocco è ridefinita in modo diverso da quella della altre classi
      void translateSourceAddress( Block*& src, int& src_output_index );
      void translateDestinationAddress( int dest_input_index, vector<Block*>& translatedDestinations, vector<int>& translatedDestInputIndexes );
  };

/* qui seguono le sei classi che definiscono i diversi tipi di comportamento dei blocchi, in base alla cardinalità del loro
   ingresso e della loro uscita; è stato ridefinito il costruttore di copia per dimensionare opportunamente i vector che
   costituisco il campo dati di tali classi; questa ridefinizione del costruttore di copia fa sì che non vengano chiamati
   i costruttori di copia della classe vector, ma solamente i costruttori di default; */

class NoOutput: virtual public Block
  {
      virtual bool addOutput( Block* dest, int dest_input_index, int output_index, int instructionCounter ) { return false; }
    protected:
      virtual void initOutputDimension( int outDim );
      virtual bool outCheckAndClear() { return true; }
    public:
      virtual int outputDimension() const  { return 0; }
      virtual int outputNumLinks() const { return 0; }
      virtual Block* getOutputPointer( int index ) const;
      virtual vector<Block*> getOutputPointers() const { return vector<Block*>(); }
      virtual double getOutputValue( int src_index ) const;
      virtual IOType getOutputType() const { return NONE; }
      virtual RoleType getRoleType() const { return SINK; } // se un blocco non ha output allora è per forza un pozzo
      virtual TransparenceType getTransparenceType() const { return TRANSPARENT; } // non è importante, comunque in linea di principio un'uscita è trasparente
  };

class SingleOutput: virtual public Block
  {
      vector<Block*> out;
      virtual bool addOutput( Block* dest, int dest_input_index, int output_index, int instructionCounter );
    protected:
      double yReg; // è il registro che conserva il l'uscita attuale   
      virtual void initOutputDimension( int outDim );
      virtual bool outCheckAndClear();
    public:
      SingleOutput() { }
      SingleOutput( const SingleOutput& model ) { out.clear(); }
      virtual int outputDimension() const { return 1; }
      virtual int outputNumLinks() const { return out.size(); }
      virtual Block* getOutputPointer( int index ) const;
      virtual vector<Block*> getOutputPointers() const { return out;  }
      virtual double getOutputValue( int src_index ) const;
      virtual IOType getOutputType() const { return SINGLE; }
  };

class MultiOutput: virtual public Block
  {
      vector< vector<Block*> > out; // ogni piedino di uscita può essere connesso a più piedini di ingresso di altri blocchi (o di se stesso)
      vector<bool> used; // marca i piedini usati
      virtual bool addOutput( Block* dest, int dest_input_index, int output_index, int instructionCounter );
      virtual bool outCheckAndClear();
      class IntWrapper { public: int value; IntWrapper(): value(0) { } };
      IntWrapper totNumLinks; // tiene traccia del numero totale di collegamenti in uscita
    protected:
      vector<double> yReg;
      virtual void initOutputDimension( int outDim );
    public:
      MultiOutput() { }
      MultiOutput( const MultiOutput& model );
      virtual int outputDimension() const { return yReg.size(); }
      virtual int outputNumLinks() const { return totNumLinks.value; }
      virtual Block* getOutputPointer( int index ) const;
      virtual vector<Block*> getOutputPointers() const;
      virtual double getOutputValue( int src_index ) const;
      virtual IOType getOutputType() const { return MULTI; }
  };

class NoInput: virtual public Block
  {
      virtual bool addInput( Block* src, int src_output_index, int input_index, int instructionCounter ) { return false; }
    protected:
      virtual void initInputDimension( int inDim );
      virtual bool inCheckAndClear() { return true; }
    public:
      virtual int inputDimension() const { return 0; }
      virtual Block* getInputPointer( int index, int& src_index ) const;
      virtual double getInputValue( int index ) const;
      virtual IOType getInputType() const { return NONE; }
      virtual RoleType getRoleType() const { return SOURCE; } // se un blocco non ha input allora è sicuramente una sorgente
      virtual TransparenceType getTransparenceType() const { return NOT_TRANSPARENT; } // una sorgente è non trasparente, perchè non ha ingresso (possiamo pensarlo come un sistema con ingresso sempre nullo e uno stato che evolvendosi produce l'uscita)
  };

class SingleInput: virtual public Block
  {
      Block* in;
      /* necessario per effettuare la getInputValue, ed in fase di replica del superblocco;
         conserva l'indice dell'uscita del blocco sorgente collegato ( *in ) */
      int src_out;
      class boolWrapper { public: bool b; boolWrapper() : b( false ) { } }; // potrei definire "bool operator()" e "boolWrapper& operator=( .. )"
      boolWrapper used; // usando un wrapper siamo sicuri che il booleano venga correttamente inizializzato
      virtual bool addInput( Block* src, int src_output_index, int input_index, int instructionCounter );
    protected:
      virtual void initInputDimension( int inDim );
      virtual bool inCheckAndClear();
    public:
      SingleInput() {}
      SingleInput( const SingleInput& model ) { in = null; }
      virtual int inputDimension() const { return 1; }
      virtual Block* getInputPointer( int index, int& src_index ) const;      
      virtual double getInputValue( int index ) const;
      virtual IOType getInputType() const { return SINGLE; }
  };
  
class MultiInput: virtual public Block
  {
      vector<Block*> in;
      /* alla posizione 'i' memorizza l'indice dell'uscita relativa al blocco *(in[i]) a cui è collegato l'i-esimo ingresso;
         necessario per effettuare la getInputValue, ed in fase di replica del superblocco */
      vector<int> src_out; 
      vector<bool> used;  // probabilmente non serve: controllare!
      virtual bool addInput( Block* src, int src_output_index, int input_index, int instructionCounter );
    protected:
      virtual void initInputDimension( int inDim );
      virtual bool inCheckAndClear();
    public:
      MultiInput() { }
      MultiInput( const MultiInput& model );
      virtual int inputDimension() const { return in.size(); }
      virtual Block* getInputPointer( int index, int& src_index ) const;
      virtual double getInputValue( int index ) const;
      virtual IOType getInputType() const { return MULTI; }
  };

// invece di definire xReg in questa classe è meglio lasciare più libertà a chi progetta il blocco, in modo che
// lo stato possa essere qualcosa di arbitrariamente complicato
class Stateful: virtual public Block
  {
    public:
      virtual RoleType getRoleType() const { return STATEFUL; }
  };

class Stateless: virtual public Block
  {
    public:
      virtual RoleType getRoleType() const { return STATELESS; }
  };

// è una rete combinatoria oppure una rete della categoria delle reti sequenziali sincronizzate di Mealy
class Transparent: virtual public Block
  {
    public:
      virtual TransparenceType getTransparenceType() const { return TRANSPARENT; }
  };

// appartiene alla categoria delle reti sequenziali sincronizzate di Moore
class NotTransparent: virtual public Block
  {
    public:
      virtual TransparenceType getTransparenceType() const { return NOT_TRANSPARENT; }
  };
  

// da qui cominciano le classi non astratte, costituenti la libreria di blocchi standard

/* Nota sulla funzione initOperation(), che viene chiamata su tutti i blocchi prima di far partire
   una simulazione: se il blocco è con stato e non trasparente in essa va necessariamente
   inizializzato il registro (o i registri) yReg. In essa va comunque inizalizzato lo stato
   qualora ve ne sia uno. */

class Sin: public NoInput, public SingleOutput  // NISO (no input, single output)
  {
      double amplitude;
      double pulse;
      double phase;
      virtual void initOperation() { }
      virtual void endOperation() { }
      virtual void real_print() const { cout << amplitude << "*sin(" << phase << " + " << pulse << "*k); valore corrente = " << yReg << "\n"; }
      Sin( const Sin& model ): Block( model ), SingleOutput( model ) { amplitude = model.amplitude; pulse = model.pulse; phase = model.phase; }
    public:
      Sin( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual Block* clone( bool ) { return new Sin( *this ); }
  };

class Step: public NoInput, public SingleOutput  // NISO (no input, single output)
  {
      int stepTime;
      double initValue;
      double finalValue;
      virtual void initOperation() { }
      virtual void endOperation() { }
      virtual void real_print() const { cout << "Step time " << stepTime << ", initial Value "<< initValue <<", final Value " << finalValue <<  "; valore corrente = " << yReg << "\n"; }
      Step( const Step& model ): Block( model ), SingleOutput( model ) { stepTime = model.stepTime; initValue = model.initValue; finalValue = model.finalValue; }
    public:
      Step( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual Block* clone( bool ) { return new Step( *this ); }
  };
  
class Ramp: public NoInput, public SingleOutput  // NISO (no input, single output)
  {
      double slope;
      int startTime;
      virtual void initOperation() { yReg = 0; }
      virtual void endOperation() { }
      virtual void real_print() const { cout << slope << "*(k - t0), t0 = " << startTime << "; valore corrente = " << yReg << "\n"; }
      Ramp( const Ramp& model ): Block( model ), SingleOutput( model ) { slope = model.slope; startTime = model.startTime; }
    public:
      Ramp( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual Block* clone( bool ) { return new Ramp( *this ); }
  };
  
class Impulse: public NoInput, public SingleOutput  // NISO (no input, single output)
  {
      double value;
      int time;
      virtual void initOperation() { }
      virtual void endOperation() { }
      virtual void real_print() const { cout << "Impulse time " << time << ", value "<< value <<  "; valore corrente = " << yReg << "\n"; }
      Impulse( const Impulse& model ): Block( model ), SingleOutput( model ) { value = model.value; time = model.time; }
    public:
      Impulse( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual Block* clone( bool ) { return new Impulse( *this ); }
  };

class Delay: public Stateful, public Transparent, public SingleInput, public SingleOutput  // SISO
  {
      double* storedValues;
      int delayTime;
      virtual void initOperation() { for ( int i=0; i<delayTime; i++ ) storedValues[i] = 0.0; }
      virtual void endOperation() { }
      virtual void real_print() const { cout << "Delay di " << delayTime << " campioni " << "; valore corrente =  " << yReg << "\n"; }
      Delay( const Delay& model ): Block( model ), SingleInput( model ), SingleOutput( model ) { delayTime = model.delayTime; storedValues = new double[ delayTime ];  }
    public:
      Delay( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      ~Delay() { delete [] storedValues; }
      virtual void refresh();
      virtual Block* clone( bool ) { return new Delay( *this ); }
  };
 
 class Integrator: public Stateful, public Transparent, public SingleInput, public SingleOutput  // SISO
  {
      virtual void initOperation() { yReg = 0; }
      virtual void endOperation() { }
      virtual void real_print() const { cout << "valore corrente =  " << yReg << "\n"; }
      Integrator( const Integrator& model ): Block( model ), SingleInput( model ), SingleOutput( model ) { }
    public:
      Integrator( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter ): Block( id, sbid, nm, micro ) { }
      virtual void refresh();
      virtual Block* clone( bool ) { return new Integrator( *this ); }
  };

class Derivator: public Stateful, public Transparent, public SingleInput, public SingleOutput  // SISO
  {
      double memory;
      virtual void initOperation() { memory = 0; }
      virtual void endOperation() { }
      virtual void real_print() const { cout << "current value =  " << yReg << "\n"; }
      Derivator( const Derivator& model ): Block( model ), SingleInput( model ), SingleOutput( model ) { }
    public:
      Derivator( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter ): Block( id, sbid, nm, micro ) {  }
      virtual void refresh();
      virtual Block* clone( bool ) { return new Derivator( *this ); }
  };
          
class MovingAverage: public Stateful, public Transparent, public SingleInput, public SingleOutput  // SISO
  {
      double* storedValues;
      int dim;
      virtual void initOperation() { for ( int i=0; i<dim; i++ ) storedValues[i] = 0.0; }
      virtual void endOperation() { }
      virtual void real_print() const { cout << "Media mobile.stato: "; for ( int j=0; j<dim; j++ ) cout << storedValues[j] << " "; cout << "; valore corrente =  " << yReg << "\n"; }
      MovingAverage( const MovingAverage& model ): Block( model ), SingleInput( model ), SingleOutput( model ) { dim = model.dim; storedValues = new double[ dim ]; }
    public:
      MovingAverage( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      ~MovingAverage() { delete [] storedValues; }
      virtual void refresh();
      virtual Block* clone( bool ) { return new MovingAverage( *this ); }
  };

class SimpleScope: public SingleInput, public NoOutput  // SINO (single-input, no output)
  {
      static ostringstream wk; // serve per effettuare la conversione da intero a stringa nel costruttore di copia
      static set<string> usedFileNames; // conserva i nomi di file già dichiarati
      
      string fileName;
      int copyNumber;
      ofstream file;
      virtual void real_print() const { cout << "Simple scope: output file '" << fileName << "'\n"; }
      SimpleScope( const SimpleScope& model ): Block( model ), SingleInput( model ) { copyNumber = model.copyNumber + 1; wk.str( "" ); wk << copyNumber; fileName = "c" + wk.str() + model.fileName; }
    public:
      SimpleScope( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual void initOperation() { file.open( fileName.c_str() ); }
      virtual void endOperation() { file.close(); }
      virtual Block* clone( bool ) { return new SimpleScope( *this ); }
      ~SimpleScope() { file.close(); }
  };

class Sum: public Stateless, public Transparent, public MultiInput, public SingleOutput
  {
      vector<int> signs;
      virtual void initOperation() { }
      virtual void endOperation() { }
      virtual void real_print() const  { cout << "Sum.stato: "; for ( int j=0; j<inputDimension(); j++ ) cout << signs[j] << " "; cout << "; valore corrente =  " << yReg << "\n"; }
      Sum( const Sum& model ): Block( model ), MultiInput( model ), SingleOutput( model ) { signs = model.signs; }
    public:
      Sum( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual Block* clone( bool ) { return new Sum( *this ); }
  };

class genericT: public Stateful, public Transparent, public SingleInput, public SingleOutput
  {
      double state;
      double A;
      double B;
      double C;
      double D;
      double IC; //condizione iniziale
      //genericT() {}
      virtual void initOperation() { state = IC; }
      virtual void endOperation() { }
      virtual void real_print() const  { cout <<"x(k+1)="<<A<<"x(k)+"<<B<<"u(k), y(k)="<<C<<"x(k)+"<<D<<"u(k), x(0)="<<IC<<"; valore corrente =  " << yReg << "\n"; }
      genericT( const genericT& model ): Block( model ), SingleInput( model ), SingleOutput( model ) { }
    public:
      genericT( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual Block* clone( bool ) { return new genericT( *this ); }
  };
  
class genericNT: public Stateful, public NotTransparent, public SingleInput, public SingleOutput
  {
      double state;
      double A;
      double B;
      double C;
      double IC; //condizione iniziale
      virtual void initOperation() { state = IC; }
      virtual void endOperation() { }
      virtual void real_print() const  { cout <<"x(k+1)="<<A<<"x(k)+"<<B<<"u(k), y(k)="<<C<<"x(k), x(0)="<<IC<<"; valore corrente =  " << yReg << "\n"; }
      genericNT( const genericT& model ): Block( model ), SingleInput( model ), SingleOutput( model ) { }
    public:
      genericNT( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual Block* clone( bool ) { return new genericNT( *this ); }
  };

class Twins: public Stateless, public Transparent, public SingleInput, public MultiOutput
  {
      virtual void initOperation() { }
      virtual void endOperation() { }
      virtual void real_print() const  { cout << "A block which duplicate his input, as a biforcated wire\n"; }
      Twins( const Twins& model ): Block( model ), SingleInput( model ), MultiOutput( model ) { }
    public:
      Twins( int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter );
      virtual void refresh();
      virtual Block* clone( bool ) { return new Twins( *this ); }
      
  };

bool create_connection( Block* src, int src_output_index, Block* dest, int dest_input_index, int instructionCounter );
#endif
