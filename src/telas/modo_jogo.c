#include "raylib.h"
#include "telas.h"
#include "menu.h"
#include "selecao.h"


void AtualizarTelaModoJogo(GameScreen *telaAtual, ModoDeJogo *modo) {
    Vector2 mouse = GetMouseVirtual();

    int btnWidth = 300;
    int btnHeight = 100;
    int spacing = 20;
    int baseY = 350;

    Rectangle btnSolo = { (SCREEN_WIDTH - btnWidth) / 2.0f, baseY, btnWidth, btnHeight };
    Rectangle btnDois = { (SCREEN_WIDTH - btnWidth) / 2.0f, baseY + btnHeight + spacing, btnWidth, btnHeight };

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        
        if (CheckCollisionPointRec(mouse, btnSolo)){
            *telaAtual = SCREEN_SELECAO; 
            *modo = MODO_SOLO;          
        } 
        else if (CheckCollisionPointRec(mouse, btnDois)) {
            *telaAtual = SCREEN_SELECAO; 
            *modo = MODO_PVP;           
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        *telaAtual = SCREEN_MENU;
    }
}



void DesenharTelaModoJogo(MenuOpcao resoucers) {
   
    DrawTexture(resoucers.background, 0, 0, WHITE);

    
    char titulo[] = "Escolha o modo de jogo";
    int fontSize = 60;
    int posXtitulo = (SCREEN_WIDTH - MeasureText(titulo, fontSize)) / 2;
    DrawText(titulo, posXtitulo, 150, fontSize, RAYWHITE);

    
    int btnWidth = 300;
    int btnHeight = 100;
    int spacing = 20;
    int baseY = 350;

    Rectangle btnSolo = { (SCREEN_WIDTH - btnWidth) / 2.0f, baseY, btnWidth, btnHeight };
    Rectangle btnDois = { (SCREEN_WIDTH - btnWidth) / 2.0f, baseY + btnHeight + spacing, btnWidth, btnHeight };

   
    Vector2 mouse = GetMouseVirtual();

    
    Rectangle texSource = { 0, 0, (float)resoucers.btnSoloTex.width, (float)resoucers.btnSoloTex.height };
    Vector2 origin = { 0, 0 };
    
  
    Color tintSolo = CheckCollisionPointRec(mouse, btnSolo) ? LIGHTGRAY : WHITE;
    DrawTexturePro(
        resoucers.btnSoloTex, 
        texSource,            
        btnSolo,              
        origin,               
        0,                    
        tintSolo              
    );

    //botao 2 jogadores
    Color tintDois = CheckCollisionPointRec(mouse, btnDois) ? LIGHTGRAY : WHITE;
    DrawTexturePro(
        resoucers.btnPvPTex,  
        texSource,            
        btnDois,              
        origin,               
        0,
        tintDois
    );
}