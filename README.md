# Bras Robotiques - Smart Factory

## Description
Simulation d'un entrepôt intelligent avec des bras robotiques qui partagent des outils.  
Chaque bras doit obtenir deux outils avant de pouvoir assembler.

## Fonctionnalités
- Communication TCP/IP client-serveur
- Serveur multithreadé
- Client avec 3 threads (réflexion, communication, assemblage)
- Mutex pour la synchronisation des outils
- Prévention des deadlocks (ordre fixe des outils)
- Journalisation avec timestamps

## Structure du projet
entrepot_intelligent/
├── client/
│ └── bras_robotique.c
├── serveur/
│ └── gestionnaire_outils.c
├── include/
│ └── commun.h
└── Makefile
