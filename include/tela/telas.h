#ifndef TELAS_H
#define TELAS_H

#include "raylib.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

typedef enum GameScreen { 
    SCREEN_MENU = 0, 
    SCREEN_SELECAO, 
    SCREEN_BATALHA,
    SCREEN_PERSONAGENS, 
    SCREEN_SOBRE,
    SCREEN_MODO_JOGO,
    SCREEN_SAIR 
} GameScreen;

typedef enum {
    MODO_SOLO,
    MODO_PVP
} ModoDeJogo;

Vector2 GetMouseVirtual(void);
// ------------------

#endif