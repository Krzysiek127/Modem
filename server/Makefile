CC=gcc
CFLAGS=-MMD -Wall -Wextra# -Wpedantic -Wconversion

SRC=../src/crc.c broadcaster.c
EXE=Broad

all: debug

debug: CFLAGS +=-g -g3
debug: $(EXE)

release: CFLAGS +=-O3 -DNDEBUG -s
release: $(EXE)
release: del /f /q /S *.d

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -lws2_32 -o $(EXE)