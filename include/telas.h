#ifndef TELAS_H
#define TELAS_H

#include "raylib.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// vamo gerenciar as telas por aqui, acho que vai ficar melhor
typedef enum GameScreen { 
    SCREEN_MENU = 0, 
    SCREEN_BATALHA,
    SCREEN_PERSONAGENS, 
    SCREEN_SOBRE,       
    SCREEN_SAIR 
} GameScreen;

#endif