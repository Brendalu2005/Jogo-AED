#ifndef MENU_H
#define MENU_H

#include "telas.h" 
typedef struct MenuOpcao {
    Texture2D background;
    Texture2D btnJogarTex;
    Texture2D btnPersonagensTex;
    Texture2D btnSobreTex;
    Texture2D btnVoltarTex;
    Texture2D btnSoloTex;
    Texture2D btnPvPTex;

    Font fontPressStart;
} MenuOpcao;


MenuOpcao LoadMenuResources(void);


void UnloadMenuResources(MenuOpcao resources);

void AtualizarTelaMenu(GameScreen *telaAtual);

void DesenharTelaMenu(MenuOpcao resources);

#endif