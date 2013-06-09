#include "engine.h"

extern ostringstream oss;

SortEngine SortEngine::instance;

void SortEngine::reassignId()
{
    int counter = 0;
    for (; counter<Block::blockList.size(); counter++)
	Block::blockList[counter]->setID(counter);
}

/* riordina il vettore passato per parametro che contiene tutti i blocchi del
   sistema in modo da stabilire un ordine di aggiornamento delle uscite dei
   vari blocchi congruente con l'applicazione; l'algoritmo implementato ha 
   complessità O(E*logN), dove N è il numero di blocchi del sistema ed E il
   numero di collegamenti del sistema, nell'ipotesi che sia N = O(E) */
bool SortEngine::sortBlocks()
{
    /* per il corretto funzionamento dell'algoritmo di ordinamento è necessario che gli ID
       dei blocchi partano da 0 e non ci siano buchi; siccome a questo punto dell'esecuzione
       questa condizione è probabilmente non rispettata (a causa dei superblocchi) si
       provvede riassegnare gli ID;  */
    SortEngine::reassignId();
    cout << "Arranging blocks...\n";
    vector<Block*> blockListBackup = Block::blockList; // serve per pulire la memoria in caso di cicli trasparenti

    int numBlock = Block::blockList.size();
    int numI;
    int maxLevel = 1;

    int i = 0;
    for (; i<numBlock; i++)
	if (Block::blockList[i]->getTransparenceType() == TRANSPARENT && Block::blockList[i]->inputDimension() > maxLevel)
	    maxLevel = Block::blockList[i]->inputDimension();

    vector<listPBlock> levelsList(maxLevel + 1);

    vector<ItAndInt> blockTracks(numBlock);
    ItAndInt iai;

    /* costruiamo il vettore che contiene il numero degli ingressi  O(N) */

    for (i=0 ; i<numBlock; i++)
    {
	if (Block::blockList[i]->getTransparenceType() == TRANSPARENT)
	{
	    numI = Block::blockList[i]->inputDimension();
	    iai.pt = levelsList[numI].push_back(Block::blockList[i]);
	    iai.level = numI;
	    blockTracks[ Block::blockList[i]->getID() ] = iai;
	}
	else
	{
	    iai.pt = levelsList[0].push_back(Block::blockList[i]); // ritorna l'indicatore
	    iai.level = 0;
	    blockTracks[ Block::blockList[i]->getID() ] = iai;
	}
    }

    int blockCounter = 0; //conta tutti i blocchi del sistema
    vector<Block*> childs;
    for (;;)  /* O(E) */
    {
	if (levelsList[0].size() == 0) // se non ci sono blocchi rimasti senza input..
	{
	    if (blockCounter == numBlock)  // se i blocchi sono stati tutti estratti allora l'algoritmo termina correttamente
		break;
	    else  // altrimenti vuol dire che esiste almeno un ciclo di blocchi tutti trasparenti
	    {
		oss << "Transparent cycle found";
		// DA FARE: si può trovare il ciclo e stamparlo
		levelsList.clear(); // scatena la chiamata dei distruttori delle liste, a questo punto ancora piene
		Block::blockList = blockListBackup;
		throw EngineError(oss.str());
	    }
	}
	// seleziona un blocco a cui non sono rimasti ingressi
	Block* pfb = levelsList[0].front();
	//Block* child;
	int child_ID, child_lev;

	if (DEBUG) { cout<<"VETTORE DI LISTE:\n"; for (i=0; i<=maxLevel; i++) { levelsList[i].print(); } cout << "\n"; cout << "Selezionato: " << pfb->getName() << ", numlinks = " << pfb->outputNumLinks() << "\n"; }  

	// aggiorna il numero di blocchi rimasti a tutit i suoi figli, ed effettua lo spostamento nella lista
	childs = pfb->getOutputPointers();
	for (i=0; i<childs.size(); i++)
	{
	    ItAndInt& track_ref = blockTracks[ childs[i]->getID() ];
	    if (track_ref.level)
	    { 
		levelsList[ track_ref.level-- ].erase(track_ref.pt);
		track_ref.pt = levelsList[ track_ref.level ].push_back(childs[i]);
	    }
	}
	/* elimina il blocco selezionato sia dalla lista che dalla mappa e lo aggiunge al vettore risultato,
	   che si ottiene ordinando "in loco" il vettore di partenza */
	levelsList[0].pop_front();
	Block::blockList[ blockCounter++ ] = pfb;
    }
    return true; 
}

bool SortEngine::arrangeBlocks()
{
    // controlla che tutti gli ingressi e tutte le uscite di tutti i blocchi siano stati utilizzati
    for (int i=0; i<Block::blockList.size(); i++)
	if (!Block::blockList[i]->inoutCheckAndClear())
	    return false;
    return sortBlocks();
}
