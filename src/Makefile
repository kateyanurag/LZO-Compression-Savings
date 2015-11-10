CC = gcc 
INCLUDE = ../include
CFLAGS = -g -m64 -c -Wall -pthread -I$(INCLUDE)

_HEADER = $(INCLUDE)/*.h
HEADER = $(patsubst %,$(INCLUDE)/%,$(_HEADER))

OBJ_DIR =../obj
_OBJ =  minilzo.o lzo_compress.o lzo_main.o
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

$(OBJ_DIR)/%.o: %.c $(HEADER)
	@$(CC) -c -o $@ $< $(CFLAGS) 

lzo: $(OBJ)
	@$(CC)  -pthread -o lzo_compress $^ -lm
	@cp lzo_compress ../output/