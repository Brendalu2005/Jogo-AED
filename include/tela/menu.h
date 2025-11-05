#ifndef MENU_H
#define MENU_H

#include "telas.h" 
typedef struct MenuOpcao {
    Texture2D background;
} MenuOpcao;


MenuOpcao LoadMenuResources(void);


void UnloadMenuResources(MenuOpcao resources);

void AtualizarTelaMenu(GameScreen *telaAtual);

void DesenharTelaMenu(MenuOpcao resources);

#endif