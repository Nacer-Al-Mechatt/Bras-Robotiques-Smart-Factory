CC = gcc
CFLAGS = -Wall -pthread -Iinclude
LDFLAGS = -pthread -lws2_32

TARGET_SERVEUR = serveur_gestionnaire.exe
TARGET_CLIENT = bras_robotique.exe

all: $(TARGET_SERVEUR) $(TARGET_CLIENT)

serveur: $(TARGET_SERVEUR)

client: $(TARGET_CLIENT)

$(TARGET_SERVEUR): serveur/gestionnaire_outils.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_CLIENT): client/bras_robotique.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	del /Q *.exe 2>nul

run-serveur:
	$(TARGET_SERVEUR)

.PHONY: all serveur client clean run-serveur