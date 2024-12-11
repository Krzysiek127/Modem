CC=gcc 
#CFLAGS=-MMD -Wall -Wextra -pedantic -std=c11
CFLAGS=-g3

SRC=$(wildcard src/*.c)

OBJ=$(SRC:%.c=%.o)
DEP=$(OBJ:%.o=%.d)

EXE=Modem
LIBS=$(addprefix -l,m ws2_32)

TARGET=/usr/local

all: debug

debug: CFLAGS += -g
debug: $(EXE)

remake: clean debug
.NOTPARALLEL: remake

release: CFLAGS += -O3 -DNDEBUG
release: clean $(EXE)
.NOTPARALLEL: release

clean:
	del $(OBJ) $(DEP) $(EXE)

install: all
	cp $(EXE) $(TARGET)/bin

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

-include $(DEP)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<