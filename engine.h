#ifndef ENGINE__H__
#define ENGINE__H__

#include "commons.h"
#include "block.h"
#include "error.h"
#include "symbol.h"
#include "list_p_block.h"

class SortEngine
{
    static void reassignId();

    struct ItAndInt
    {
	Node* pt;
	int level;
    };
    static SortEngine instance;
    SortEngine() { }
    SortEngine( const SortEngine& );
    SortEngine& operator=( const SortEngine& );
    bool sortBlocks();
    public:
    static SortEngine* getInstance() { return &instance; }
    bool arrangeBlocks();
};

#endif
