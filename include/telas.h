#ifndef TELAS_H
#define TELAS_H

#include "raylib.h"

// Ajustado para a resolução do seu log
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

// vamo gerenciar as telas por aqui, acho que vai ficar melhor
typedef enum GameScreen { 
    SCREEN_MENU = 0, 
    SCREEN_SELECAO,      // <--- ADICIONADO
    SCREEN_BATALHA,
    SCREEN_PERSONAGENS, 
    SCREEN_SOBRE,
    SCREEN_SAIR 
    // SCREEN_TESTE_ANIMACAO foi removido
} GameScreen;

#endif