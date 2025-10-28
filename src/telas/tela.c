#include "raylib.h"
#include "tela.h" 
void AtualizarTelaBatalha(int *ataqueSelecionado){
    
    Vector2 mousePos = GetMousePosition();

    int arenaY = 80;
    int arenaHeight = 450;
    int menuY = arenaY + arenaHeight + 10;
    int textoYBase = menuY + 20;
    int colAtaquesX = 35;

    Rectangle btnSoco = { colAtaquesX, textoYBase + 35, 150, 40 };
    Rectangle btnCuspe = { colAtaquesX, textoYBase + 90, 150, 40 };

    if (CheckCollisionPointRec(mousePos, btnSoco)){
        *ataqueSelecionado = 0; 
    }
    
    if (CheckCollisionPointRec(mousePos, btnCuspe)){
        *ataqueSelecionado = 1; 
    }
}


void DesenharTelaBatalha(int ataqueSelecionado){

    DrawText("Jogador 1", 20, 15, 20, RAYWHITE);
    DrawRectangle(130, 10, 200, 30, GREEN); 
    DrawRectangleLines(130, 10, 200, 30, BLACK); 
    DrawText("Round", (SCREEN_WIDTH / 2) - 40, 15, 20, RAYWHITE);
    DrawText("3", (SCREEN_WIDTH / 2) - 10, 40, 30, RAYWHITE);
    
    int iaBarX = SCREEN_WIDTH - 20 - 200;
    DrawText("IA", iaBarX - 35, 15, 20, RAYWHITE);
    DrawRectangle(iaBarX, 10, 200, 30, GREEN);
    DrawRectangleLines(iaBarX, 10, 200, 30, BLACK);

    
    int arenaY = 80;
    int arenaHeight = 450;
    DrawRectangleLines(10, arenaY, SCREEN_WIDTH - 20, arenaHeight, LIGHTGRAY);



    int menuY = arenaY + arenaHeight + 10;
    int menuHeight = SCREEN_HEIGHT - menuY - 10;
    Color menuBG = (Color){ 40, 40, 40, 255 };
    DrawRectangle(10, menuY, SCREEN_WIDTH - 20, menuHeight, menuBG);
    DrawRectangleLines(10, menuY, SCREEN_WIDTH - 20, menuHeight, RAYWHITE);

    
    int colAtaquesX = 35;
    int colSpecsX = (SCREEN_WIDTH / 2) - 100; 
    int textoYBase = menuY + 20;

    DrawText("Ataque:", colAtaquesX, textoYBase, 20, GREEN);
    DrawText("especificações:", colSpecsX, textoYBase, 20, GREEN);

    Color corBotaoNormal = LIGHTGRAY;
    Color corBotaoSelecionado = YELLOW;
    Color corTexto = BLACK;
    float espessuraBorda = 2.0f;
    
    Rectangle btnSoco = { colAtaquesX, textoYBase + 35, 150, 40 };
    Rectangle btnCuspe = { colAtaquesX, textoYBase + 90, 150, 40 };
    
    Color corSoco, corCuspe;

    if (ataqueSelecionado == 0){
        corSoco = corBotaoSelecionado;
        corCuspe = corBotaoNormal;
    } else {
        corSoco = corBotaoNormal;
        corCuspe = corBotaoSelecionado;
    }

    // Botão 1: Soco
    DrawRectangleRounded(btnSoco, 0.2f, 4, corSoco);
    DrawRectangleRoundedLinesEx(btnSoco, 0.2f, 4, espessuraBorda, BLACK);
    DrawText("Soco", btnSoco.x + (btnSoco.width / 2) - (MeasureText("Soco", 20) / 2), btnSoco.y + 10, 20, corTexto);

    // Botão 2: Cuspe
    DrawRectangleRounded(btnCuspe, 0.2f, 4, corCuspe);
    DrawRectangleRoundedLinesEx(btnCuspe, 0.2f, 4, espessuraBorda, BLACK);
    DrawText("Cuspe", btnCuspe.x + (btnCuspe.width / 2) - (MeasureText("Cuspe", 20) / 2), btnCuspe.y + 10, 20, corTexto);

    if (ataqueSelecionado == 0){
        DrawText("Um soco direto no oponente.", colSpecsX, textoYBase + 40, 20, RAYWHITE);
        DrawText("Causa 10 de Dano.", colSpecsX, textoYBase + 70, 20, RAYWHITE);
    } else {
        DrawText("Um ataque nojento que diminui", colSpecsX, textoYBase + 40, 20, RAYWHITE);
        DrawText("a defesa do oponente.", colSpecsX, textoYBase + 70, 20, RAYWHITE);
    }
}