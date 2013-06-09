CC=g++

OBJS=main.o lexer.o syntax.o symbol.o block.o engine.o console.o time.o list_p_block.o
BIN=dissimulator

all: $(BIN)

clean:
	-rm *.o $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS)


main.o: lexer.h syntax.h symbol.h engine.h console.h

lexer.o: lexer.h

syntax.o: syntax.h

symbol.o: symbol.h

block.o: block.h

engine.o: engine.h

console.o: console.h

time.o: time.h

list_p_block.o: list_p_block.h
