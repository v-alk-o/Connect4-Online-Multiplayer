CC = gcc
CFLAGS = -pthread

SERVER_SRC = src/server.c
CLIENT_SRC = src/client.c

FILES_SRC = src/connect4-utils.c src/display-utils.c src/string-utils.c

SERVER_EXE = bin/server
CLIENT_EXE = bin/client

DEMO_SCRIPT = tools/LocalDemo.sh


all: run

run: compile
	./$(DEMO_SCRIPT) 2>/dev/null

compile:
	$(CC) $(CFLAGS) $(SERVER_SRC) $(FILES_SRC) -o $(SERVER_EXE)
	$(CC) $(CFLAGS) $(CLIENT_SRC) $(FILES_SRC) -o $(CLIENT_EXE)

clean:
	rm $(SERVER_EXE) $(CLIENT_EXE)
