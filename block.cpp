#include "block.h"
#include <cmath>

extern ostringstream oss;

BlockBuilder BlockBuilder::instance;

int BlockBuilder::id_counter = 0;

Block* BlockBuilder::build(int sbid, const string& id_name, const string& micro_type, TKPTVCIT pBegin, int current_instruction)
{
    // demultiplexing
    if (micro_type == "moving_average")
	return new MovingAverage(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);
    else if (micro_type == "sin")
	return new Sin(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);
    else if (micro_type == "simple_scope")
	return new SimpleScope(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);
    else if (micro_type == "sum")
	return new Sum(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);
    else if (micro_type == "step")
	return new Step(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction); 
    else if (micro_type == "ramp")
	return new Ramp(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);           
    else if (micro_type == "impulse")
	return new Impulse(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);  
    else if (micro_type == "integrator")
	return new Integrator(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction); 
    else if (micro_type == "derivator")
	return new Derivator(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction); 
    else if (micro_type == "genericT")
	return new genericT(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction); 
    else if (micro_type == "genericNT")
	return new genericNT(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);             
    else if (micro_type == "twins")
	return new Twins(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);
    else if (micro_type == "delay")
	return new Delay(id_counter++, sbid, id_name, micro_type, pBegin, current_instruction);
    else 
    {
	oss << "You must add a row for type '" << micro_type << "' in the demultiplexing function BlockBuilder::build()";
	throw BlockError(oss.str());
    }
}

Block::Block(const Block& model) 
{ 
    ID = BlockBuilder::id_counter++;
    name = ""; // non è significativo
    microType = model.microType;
    if (DEBUG) cout << "Ho copiato un tale " << model.getName() << "\n";
}

bool Block::inoutCheckAndClear()
{
    bool r1 = inCheckAndClear();
    bool r2 = outCheckAndClear();
    return r1 && r2;
}

vector<Block*> Block::blockList; 

void Block::freeTable()
{
    return;
    for (int h=0; h<Block::blockList.size(); h++)
	delete Block::blockList[h];
    Block::blockList.clear(); // per sicurezza
}

map<string, SuperBlock::SuperBlockElement*> SuperBlock::superBlocksTable;
vector<Block*> SuperBlock::blockModelsList; 

void SuperBlock::freeTables()
{
    SuperBlockElement* sbep;
    for (map<string,SuperBlockElement*>::iterator mis = SuperBlock::superBlocksTable.begin(); mis != SuperBlock::superBlocksTable.end(); mis++)
	delete mis->second;
    SuperBlock::superBlocksTable.clear();
    for (int i=0; i<SuperBlock::blockModelsList.size(); i++)
	delete SuperBlock::blockModelsList[i];
}

Block* SuperBlock::clone(bool superBlockInstance) 
{ 
    // ricerca la descrizione del proprio tipo nella tabella dei superblocchi
    map<string, SuperBlockElement*>::iterator mit = SuperBlock::superBlocksTable.find(getType());
    SuperBlockElement* sbep = mit->second;
    int j = 0,
	sz = sbep->superBlockBlocks.size();
    Block** match = new Block*[ sz ];
    for (; j<sz; j++)
	match[ j ] = sbep->superBlockBlocks[j]->clone(superBlockInstance);   

    if (DEBUG)for (j=0; j<sz; j++) cout << "match["<< j << "] = " << match[j]->getID() << "\n";

    int isz = sbep->internalConnectionsTable.size() ;
    for (j=0; j<isz; j++)
	create_connection(match[ sbep->internalConnectionsTable[j].srcSuperBlockID ], sbep->internalConnectionsTable[j].srcIndex, match[ sbep->internalConnectionsTable[j].destSuperBlockID ], sbep->internalConnectionsTable[j].destIndex, -1);

    SuperBlock* newb = new SuperBlock(*this); // usa il meccanismo del costruttore di copia per richiamare il costruttore di copia della classe base

    // a questo punto deve creare un nuovo blocco di tipo SuperBlock
    int irsz = sbep->inputSuperBlockMatchingTable.size(),
	orsz = sbep->outputSuperBlockMatchingTable.size();        
    newb->inputRedirectTable.resize(irsz);
    newb->outputRedirectTable.resize(orsz);
    for (j=0; j<irsz; j++)
    {
	newb->inputRedirectTable[j].sbIndex = sbep->inputSuperBlockMatchingTable[j].sbIndex;
	newb->inputRedirectTable[j].blockPointer = match[ sbep->inputSuperBlockMatchingTable[j].sbBlockId ];
	newb->inputRedirectTable[j].sbBlockIndex = sbep->inputSuperBlockMatchingTable[j].sbBlockIndex;
    }
    for (j=0; j<orsz; j++)
    {
	newb->outputRedirectTable[j].sbIndex = sbep->outputSuperBlockMatchingTable[j].sbIndex;
	newb->outputRedirectTable[j].blockPointer = match[ sbep->outputSuperBlockMatchingTable[j].sbBlockId ];
	newb->outputRedirectTable[j].sbBlockIndex = sbep->outputSuperBlockMatchingTable[j].sbBlockIndex;
    }
    for (j=0; j<sz; j++)
	if (!superBlockInstance && match[j]->getRoleType() != SUPERBLOCK)
	    Block::blockList.push_back(match[j]);
	else
	    SuperBlock::blockModelsList.push_back(match[j]);

    delete [] match;

    return newb;
}

void SuperBlock::translateSourceAddress(Block*& src, int& src_output_index)
{
    for (int j=0; j<outputRedirectTable.size(); j++)
	if (outputRedirectTable[j].sbIndex == src_output_index)
	{
	    src = outputRedirectTable[j].blockPointer;
	    src_output_index = outputRedirectTable[j].sbBlockIndex;
	    if (src->getRoleType() == SUPERBLOCK)
		static_cast<SuperBlock*>(src)->translateSourceAddress(src, src_output_index);
	    return;
	}
    src = null;
}

void SuperBlock::translateDestinationAddress(int dest_input_index, vector<Block*>& translatedDestinations, vector<int>& translatedDestInputIndexes)
{
    for (int j=0; j<inputRedirectTable.size(); j++)
	if (inputRedirectTable[j].sbIndex == dest_input_index)
	{
	    if (inputRedirectTable[j].blockPointer->getRoleType() == SUPERBLOCK)
		static_cast<SuperBlock*>(inputRedirectTable[j].blockPointer)->translateDestinationAddress(inputRedirectTable[j].sbBlockIndex, translatedDestinations, translatedDestInputIndexes);
	    else
	    {
		translatedDestinations.push_back(inputRedirectTable[j].blockPointer);
		translatedDestInputIndexes.push_back(inputRedirectTable[j].sbBlockIndex);
	    } 
	}
}

Block* NoOutput::getOutputPointer(int index) const
{
    oss << "'" << name << "' is a no-output block: can't get output pointers";
    throw BlockError(oss.str());
}

double NoOutput::getOutputValue(int src_index) const
{
    oss << "Bad getOutputValue request: block '" << name << "' doesn't have any output";
    throw BlockError(oss.str());
}

void NoOutput::initOutputDimension(int outDim)
{
    oss << "You can't set output dimension of no-output block '" << microType << "'";
    throw BlockError(oss.str());
}

bool SingleOutput::addOutput(Block* dest, int dest_input_index, int output_index, int instructionCounter)
{
    if (output_index != 0)
	return false;
    // chiamato almeno una volta se e solo se out.size() != 0
    out.push_back(dest);
    return true;
}

Block* SingleOutput::getOutputPointer(int index) const
{
    if (index >= out.size())
    {
	oss << "Out of range with single-output block '" << name << "'";
	throw BlockError(oss.str());
    }
    return out[index];
}

double SingleOutput::getOutputValue(int src_index) const
{
    /* qui ci potremmo fidare (forse)... */
    if (src_index != 0)
    {
	oss << "Bad getOutputValue request: block '" << name << "' is single-input: index must be 0";
	throw BlockError(oss.str());
    }
    return yReg;
}

void SingleOutput::initOutputDimension(int outDim)
{
    oss << "You can't set output dimension of single-output block '" << microType << "'";
    throw BlockError(oss.str());
}

bool SingleOutput::outCheckAndClear()
{
    if (out.size() == 0)
    {
	oss << "Output of single-output block '" << name << "' not assigned";
	throw SyxError(oss.str());
    }
    return true;
}

bool MultiOutput::addOutput(Block* dest, int dest_input_index, int output_index, int instructionCounter)
{
    if (yReg.size() == 0) // se ciò si verifica vuol dire che nel costruttore del blocco non è stata chiamata la funzione initOutputDimension()
    {
	oss << "At instruction # " << instructionCounter << ": can't add output to block '" << name << "': use function initOutputDimension() to set output dimension of type " << microType << "'"; 
	throw SyxError(oss.str());
    }
    cout << "dest: " << dest->getName() << "; dest_input index " << dest_input_index << "; output index " << output_index << "\n"; 
    if (output_index >= yReg.size())
    {
	oss << "At instruction # " << instructionCounter << ": output dimension of block '" << name << "' is " << outputDimension() << ": you can't use index " << output_index;
	throw SyxError(oss.str());
    }
    used[ output_index ] = true; // non deve segnalare l'assegnamento multiplo degli output, perchè ne è prevista la possibilità
    out[ output_index ].push_back(dest);
    totNumLinks.value++;
    return true;
}

// l'indice si riferisce ai collegamenti "fisici" in uscita e non alla dimensione dell'uscita
Block* MultiOutput::getOutputPointer(int index) const
{
    if (index >= out.size())
    {
	oss << "Out of range with multi-output block '" << name << "'";
	throw BlockError(oss.str());
    }


    return null;  
    //return out[index];
}

vector<Block*> MultiOutput::getOutputPointers() const
{
    vector<Block*> pts(totNumLinks.value);
    int i = 0,
	k = 0,
	j;
    for (; i<out.size(); i++)
	for (j=0; j<out[i].size(); j++)
	    pts[k++] = out[i][j];
    return pts;

}

double MultiOutput::getOutputValue(int src_index) const
{
    if (src_index >= yReg.size())
    {
	oss << "Bad getOutputValue request: out of range with block '" << name << "'";
	throw BlockError(oss.str());
    }
    return yReg[ src_index ];
}

void MultiOutput::initOutputDimension(int outDim)
{
    if (yReg.size() != 0)
    {
	oss << "Output dimension of multi-output block '" << microType << "' set more than once";
	throw BlockError(oss.str());
    }
    if (outDim < 1)
    {
	oss << "Output dimension of multi-output block '" << microType << "' must be > 0";
	throw BlockError(oss.str());
    }
    out.resize(outDim);
    yReg.resize(outDim);
    used.resize(outDim);
    for (int j=0; j<outDim; j++)
	used[j] = false;
}

bool MultiOutput::outCheckAndClear()
{
    if (yReg.size() == 0)
    {
	oss << "Output of block '" << name << "' is completely disconnected";          
	throw SyxError(oss.str());
    }
    if (yReg.size() && !used.size()) throw SyxError("You can't call function outCheckAndClear() more than once");
    for (int j=0; j<yReg.size(); j++)
	if (!used[j])
	{
	    oss << "Output #" << j << " of multi-output block '" << getType() << "' not assigned";          
	    throw SyxError(oss.str());
	}
    return true;
}

MultiOutput::MultiOutput(const MultiOutput& model)
{
    int d = model.outputDimension();
    out.resize(d);
    used.resize(d);
    for (int i=0; i<used.size(); i++)
	used[i] = false;
    totNumLinks.value = 0; // ridondante
    yReg.resize(d);
}

Block* NoInput::getInputPointer(int index, int& src_index) const
{
    oss << "'" << name << "' is a no-input block: can't get input pointers";
    throw BlockError(oss.str());
}

double NoInput::getInputValue(int index) const
{
    oss << "Bad getInputValue request: block '" << name << "' doesn't have any input";
    throw BlockError(oss.str());
}

void NoInput::initInputDimension(int inDim)
{
    oss << "You can't set input dimension of no-input block '" << microType << "'";
    throw BlockError(oss.str());
}

bool SingleInput::addInput(Block* src, int src_output_index, int input_index, int instructionCounter)
{
    if (input_index != 0)
	return false;
    if (name != "" && used.b) // il warning non è significativo quando stiamo clonando le connessioni di un superblocco
	cout << ">>warning: multiple assignment of " << name << ".input at instruction #" << instructionCounter << "\n";
    else
	used.b = true;
    in = src;
    src_out = src_output_index;
    return true;
}

Block* SingleInput::getInputPointer(int index, int& src_index) const
{
    if (index !=0)
    {
	oss << "Single-input block '" << name << "' require input index to be 0";
	throw BlockError(oss.str());
    }
    if (!used.b)
	return null;
    src_index = src_out;
    return in;
}

double SingleInput::getInputValue(int index) const
{
    if (index != 0)
    {
	oss << "Bad getInputValue request: block '" << name << "' is single-input: index must be 0";
	throw BlockError(oss.str());
    }
    return in->getOutputValue(src_out);
}

void SingleInput::initInputDimension(int inDim)
{
    oss << "You can't set input dimension of single-input block '" << microType << "'";
    throw BlockError(oss.str());
}

bool SingleInput::inCheckAndClear()
{
    if (used.b == 0)
    {
	oss << "Input of single-input block '" << getID() << "' not assigned";
	throw SyxError(oss.str());
    }
    return true;
}

/* < SPIEGAZIONE SU src_out > */
bool MultiInput::addInput(Block* src, int src_output_index, int input_index, int instructionCounter)
{
    if (in.size() == 0) // se ciò si verifica vuol dire che nel costruttore del blocco non è stata chiamata la funzione initInputDimension()
    {
	oss << "At instruction # " << instructionCounter << ": can't add input to block '" << name << "': use function initInputDimension() to set input dimension of type '" << microType << "'"; 
	throw SyxError(oss.str());
    }
    if (input_index >= in.size())
    {
	oss << "At instruction # " << instructionCounter << ": input dimension of block' " << name << "' is " << inputDimension() << ": you can't use index " << input_index;
	throw SyxError(oss.str());
    }
    if (name != "" && used[ input_index ]) // il warning non è significativo quando stiamo clonando le connessioni di un superblocco
	cout << ">>warning: multiple assignment of " << name << ".input[" << input_index << "] at instruction #" << instructionCounter << "\n";
    else
	used[ input_index ] = true;
    in[ input_index ] = src;
    src_out[ input_index ] = src_output_index;
    return true;
}

Block* MultiInput::getInputPointer(int index, int& src_index) const
{
    if (index >= in.size())
    {
	oss << "Out of range with multi-input block '" << name << "'";
	throw BlockError(oss.str());
    }
    if (!used[ index ])
	return null;
    src_index = src_out[index];
    return in[index];
} 

double MultiInput::getInputValue(int index) const
{
    if (index >= in.size())
    {
	oss << "Bad getInputValue request: out of range with block '" << name << "'";
	throw BlockError(oss.str());
    }
    return in[index]->getOutputValue(src_out[index]); 
}

void MultiInput::initInputDimension(int inDim)
{
    if (in.size() != 0)
    {
	oss << "Input dimension of multi-input block '" << microType << "' set more than once";
	throw BlockError(oss.str());
    }
    if (inDim < 1)
    {
	oss << "Input dimension of multi-input block '" << microType << "' must be > 0";
	throw BlockError(oss.str());
    }
    in.resize(inDim);
    src_out.resize(inDim);
    used.resize(inDim);
    for (int j=0; j<inDim; j++)
    {
	used[j] = false;
	in[j] = null;
    }
}

bool MultiInput::inCheckAndClear()
{
    if (in.size() == 0)
    {
	oss << "Input of block '" << name << "' is completely disconnected";          
	throw SyxError(oss.str());
    }
    if (in.size() && !used.size()) throw SyxError("You can't call function inCheckAndClear() more than once");
    for (int j=0; j<in.size(); j++)
	if (!used[j])
	{
	    oss << "Input #" << j << " of multi-input block '" << ID << "' not assigned"; // name
	    throw SyxError(oss.str());
	}
    return true;
}

MultiInput::MultiInput(const MultiInput& model)
{
    int d = model.inputDimension();
    in.resize(d);
    src_out.resize(d);
    used.resize(d);
    for (int i=0; i<used.size(); i++)
	used[i] = false;
}

bool create_connection(Block* src, int src_output_index, Block* dest, int dest_input_index, int instructionCounter)
{
    /* nel caso in cui i blocchi da connettere siano superblocchi, bisogna opportunamente
       redirezionarli, utilizzando le strutture dati di redirezione di superblocco;
       per "indirizzo" si intende la coppia (indirizzo in memoria del blocco, indice della porta di ingresso o uscita) */
    /* come prima cosa, viene tradotto l'indirizzo sorgente: la traduzione è sempre univoca */
    SuperBlock* sbp;
    if (src->getRoleType() == SUPERBLOCK)
	static_cast<SuperBlock*>(src)->translateSourceAddress(src, src_output_index);

    /* se il blocco destinatario non è un superblocco, allora si procede normalmente,
       senza effettuare alcuna traduzione */
    if (dest->getRoleType() != SUPERBLOCK)
    {
	src->addOutput(dest, dest_input_index, src_output_index, instructionCounter); // il controllo su src_output lo fa qui (serve anche per la addInput)
	dest->addInput(src, src_output_index, dest_input_index, instructionCounter);  
    }
    /* altrimenti in generale esistono più indirizzi di blocchi destinatari che
       corrispondono ad una certa porta di ingresso del superblocco destinatario;
       è necessario simulare tutte le connessioni */
    else
    {  
	vector<Block*> translatedDestinations;
	vector<int> translatedDestInputIndexes;
	static_cast<SuperBlock*>(dest)->translateDestinationAddress(dest_input_index, translatedDestinations, translatedDestInputIndexes);

	for (int j=0; j<translatedDestinations.size(); j++)
	{
	    src->addOutput(translatedDestinations[j], translatedDestInputIndexes[j], src_output_index, instructionCounter); // il controllo su src_output lo fa qui (serve anche per la addInput)
	    translatedDestinations[j]->addInput(src, src_output_index, translatedDestInputIndexes[j], instructionCounter);  
	}
    }
}  


// Libreria di blocchi standard

Sin::Sin(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    if (!(*pBegin)->isNumber())
	throw SyxError("Amplitude number expected at instruction #" + instructionCounter);
    amplitude = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Pulse number expected at instruction #" + instructionCounter);
    pulse = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Phase number expected at instruction #" + instructionCounter);
    phase = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
}

void Sin::refresh()
{
    yReg = amplitude * sin(Time::getTime() * pulse + phase);
}

Step::Step(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    if (!(*pBegin)->isNumber())
	throw SyxError("Step time expected at instruction #" + instructionCounter);
    stepTime = (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Initial value of the step function expected at instruction #" + instructionCounter);
    initValue = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Final value of the step function expected at instruction #" + instructionCounter);
    finalValue = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
}

void Step::refresh()
{
    if (Time::getTime() < stepTime)
	yReg = initValue;
    else
	yReg = finalValue;
}

Ramp::Ramp(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    if (!(*pBegin)->isNumber())
	throw SyxError("Slope of the ramp expected at instruction #" + instructionCounter);
    slope = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Start time of the ramp expected at instruction #" + instructionCounter);
    startTime = (static_cast<Integer*>(*pBegin))->getValue();
}

void Ramp::refresh()
{
    if (Time::getTime() > startTime)
	yReg += slope;
}

Impulse::Impulse(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    if (!(*pBegin)->isNumber())
	throw SyxError("Start time of the impulse expected at instruction #" + instructionCounter);
    time = (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Value of the ramp expected at instruction #" + instructionCounter);
    value = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
}

void Impulse::refresh()
{
    if (Time::getTime() == time)
	yReg = value;
    else                
	yReg = 0 ;
}

Delay::Delay(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    if (!(*pBegin)->isNumber() || (static_cast<Number*>(*pBegin))->isDouble())
	throw SyxError("Integer number expected at instruction #" + instructionCounter);
    delayTime = (static_cast<Integer*>(*pBegin))->getValue();
    storedValues = new double[ delayTime ];
}

void Delay::refresh() 
{
    yReg = storedValues[0];
    for (int i=0; i<delayTime-1; i++)
	storedValues[ i ] = storedValues[ i + 1 ];
    storedValues[ delayTime - 1 ] = getInputValue(0) ;
}

void Integrator::refresh() 
{
    yReg += getInputValue(0);
}

void Derivator::refresh() 
{
    yReg = getInputValue(0) - memory;
    memory = getInputValue(0);
}

MovingAverage::MovingAverage(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    if (!(*pBegin)->isNumber() || (static_cast<Number*>(*pBegin))->isDouble())
	throw SyxError("Integer number expected at instruction #" + instructionCounter);
    dim = (static_cast<Integer*>(*pBegin))->getValue();
    storedValues = new double[ dim ];
}

void MovingAverage::refresh() // l'implementazione è un po' inefficiente, si può migliorare con un indice del prossimo elemento da sostituire
{
    yReg = 0;
    for (int i=0; i<dim-1; i++)
    {
	storedValues[i] = storedValues[i+1];
	yReg += storedValues[i+1];
    }
    yReg += (storedValues[dim - 1] = getInputValue(0));
    yReg /= dim;
}

ostringstream SimpleScope::wk;
set<string> SimpleScope::usedFileNames;

SimpleScope::SimpleScope(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instruction_counter): Block(id, sbid, nm, micro)
{
    fileName = (*pBegin)->getName();
    if (SimpleScope::usedFileNames.find(fileName) != SimpleScope::usedFileNames.end())
    {
	oss << "File name '" << fileName << " used more than once\n";
	throw SyxError(oss.str());
    }
    SimpleScope::usedFileNames.insert(fileName);
    fileName += ".txt";
    copyNumber = 0;
}

void SimpleScope::refresh()
{
    file << "k = " << Time::getTime() << "   value = " << getInputValue(0) << "\n";
}

Sum::Sum(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    while ((*pBegin)->getName() != ")")
    {              
	if ((*pBegin)->getName() == "+")
	{  signs.push_back(1); pBegin++; }
	else if ((*pBegin)->getName() == "-")        
	{  signs.push_back(-1); pBegin++; }        
	else
	    throw SyxError("Symbols + or - expected at instruction " + instructionCounter);    
    }
    initInputDimension(signs.size());
}

void Sum::refresh()
{
    yReg = 0;
    for (int i=0; i<inputDimension(); i++)
	yReg += signs[i] * getInputValue(i);  
}

genericT::genericT(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    if (!(*pBegin)->isNumber())
	throw SyxError("Number of parameter A expected at instruction #" + instructionCounter);
    A = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Number of parameter B expected at instruction #" + instructionCounter);
    B = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Number of parameter C expected at instruction #" + instructionCounter);
    C = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Number of parameter D expected at instruction #" + instructionCounter);
    D = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Initial condition expected at instruction #" + instructionCounter);
    IC = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
}

void genericT::refresh()
{
    yReg = C * state + D * getInputValue(0);                        
    state = A * state + B * getInputValue(0);
}

genericNT::genericNT(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block(id, sbid, nm, micro)
{
    if (!(*pBegin)->isNumber())
	throw SyxError("Number of parameter A expected at instruction #" + instructionCounter);
    A = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Number of parameter B expected at instruction #" + instructionCounter);
    B = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Number of parameter C expected at instruction #" + instructionCounter);
    C = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
    pBegin++;
    if (!(*pBegin)->isNumber())
	throw SyxError("Initial condition expected at instruction #" + instructionCounter);
    IC = ((static_cast<Number*>(*pBegin))->isDouble()) ?  (static_cast<Double*>(*pBegin))->getValue() : (static_cast<Integer*>(*pBegin))->getValue();
}

void genericNT::refresh()
{
    yReg = C * state ;                        
    state = A * state + B * getInputValue(0);
}

Twins::Twins(int id, int sbid, string nm, string micro, TKPTVCIT pBegin, int instructionCounter): Block (id, sbid, nm, micro)
{
    initOutputDimension(2);
}

void Twins::refresh()
{
    yReg[0] = getInputValue(0);
    yReg[1] = yReg[0];
}
