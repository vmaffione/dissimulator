#include "console.h"

Console* Console::instance = null;
vector<string> Console::funcNames;
vector<Console::IPF> Console::funcPointers;

Console::ConsoleDestructor Console::destr;

// funzione di utilità a disposizione delle funzioni che implementano i comandi interni alla console e necessitano di leggere dei parametri
string readNextParameter(const string& param, int& p)
{
    int l = param.size(),
	h = p,
	q;
    while (p<l && !isspace(param[p])) { p++; }
    q = p;
    while (p<l && isspace(param[p])) { p++; }
    return param.substr(h, q - h);
}

int Console::start(const string& param)
{
    int kBegin, kStop, p = 0;
    string par1 = readNextParameter(param, p),
	   par2 = readNextParameter(param, p),
	   par3 = readNextParameter(param, p);
    if (par2.substr(0, 1) == "-")
    {
	par3 = par2;
	par2 = "";
    }
    if (par1 == "")
    {
	cout << "  usage: 'start BEGIN STOP' or 'start STOP', where BEGIN and STOP are integers\n    option -d enables debug mode\n";
	return 2;
    }
    if (par2 == "")
    {
	kBegin = 0;
	kStop = atoi(par1.c_str());
	if (par1 != "0" && kStop == 0)
	{
	    cout << "  usage: 'start BEGIN STOP' or 'start STOP', where BEGIN and STOP are integers\n    option -d enables debug mode\n";
	    return 2;
	}
    }
    else
    {
	kBegin = atoi(par1.c_str());
	if (par1 != "0" && kBegin == 0)
	{
	    cout << "  usage: 'start BEGIN STOP' or 'start STOP', where BEGIN and STOP are integers\n    option -d enables debug mode\n";
	    return 2;
	}
	kStop = atoi(par2.c_str());
	if (par2 != "0" && kStop == 0)
	{
	    cout << "  usage: 'start BEGIN STOP' or 'start STOP', where BEGIN and STOP are integers\n    option -d enables debug mode\n";
	    return 2;
	}
	if (kBegin > kStop)
	{
	    cout << "  STOP must be greater than STOP\n";
	    return 2;
	}
    }
    bool debugMode = (par3 == "-d" || par3 == "-D");
    int i;    
    // esegue le inizializzazioni per tutti i blocchi
    for (i=0; i<Block::blockList.size(); i++)
	Block::blockList[i]->initOperation();
    Time::K = kBegin;
    int j;
    for (i=kBegin; i<=kStop; i++)
    {
	if (debugMode)
	    cout << "Time #" << i << ": ";
	for (j=0; j<Block::blockList.size(); j++)
	{
	    Block::blockList[j]->refresh();
	    if (debugMode)
	    {
		cout << "  ";
		Block::blockList[j]->print();
	    }
	}
	if (debugMode)
	    getchar();
	Time::K++;
	//if (DEBUG) { cout << "Time: " << Time::K << "\n"; for (j=0; j<Block::blockList.size(); j++) Block::blockList[j]->print(); cout << "\n"; }
    }
    // esegue le operazioni di chiusura (in particolare chiude gli stream relativi ad i blocchi pozzo)
    for (i=0; i<Block::blockList.size(); i++)
	Block::blockList[i]->endOperation();
    cout << "Simulation completed!\n";  
    return 1;
}

int Console::help(const string& param)
{
    cout << "  The following functions are available:\n";
    for (int i=0; i<funcNames.size(); i++)
	cout << "   " << i << ") " << funcNames[i] << "\n";
    return 1;
}

int Console::quit(const string& param)
{
    cout << "Thanks for using me!\nBye\n";
    return 0;
}

Console::Console()
{
    pFin = &cin;
    funcNames.push_back("start");
    funcPointers.push_back(&start);
    funcNames.push_back("help");
    funcPointers.push_back(&help);
    funcNames.push_back("quit");
    funcPointers.push_back(&quit);
}

Console* Console::getInstance()
{
    if (instance == null)
    {
	cout << "********* DisSimulator console *********\n";
	cout << "  HINT: type 'help' to get the command list\n";
	instance = new Console;
    }
    return instance;
}

/* legge un comando dall'input predefinito e lo esegue; un comando è sempre
   costituito da una riga dell'input; questa funzione ritorna 0 se il comando
   è di uscita, altrimenti restituisce una valore diverso da 0; */
int Console::readAndExecuteNext()
{
    cout << "DisConsole >> "; // dà il prompt
    string wstr;
    // legge una stringa dall'input
    getline(cin, wstr);
    int i = 0, l = wstr.size(), b;
    while (i < l && isspace(wstr[i])) { i++; }
    b = i;
    while (i < l)
    {
	if (isspace(wstr[i])) break;
	if (!isalpha(wstr[i]))
	{
	    cout << "  Invalid command: character " << wstr[i] << " is illegal\n";
	    return 2;
	}
	i++;
    }
    string cmd = wstr.substr(b, i - b);
    while (i < l && isspace(wstr[i])) { i++; }
    string param = wstr.substr(i);

    for (i=0; i<funcPointers.size(); i++)
	if (funcNames[i] == cmd)
	    return (*funcPointers[i])(param);
    if (i == funcPointers.size())
    {
	cout << "  command '" << cmd << "' does not exist.\n";
	cout << "    HINT: type 'help' to get the command list\n";  
	return 2; 
    }
}
