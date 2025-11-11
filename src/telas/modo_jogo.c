#include "raylib.h"
#include "telas.h"
#include "menu.h"
#include "selecao.h"

// --- MODIFICADO ---
// A assinatura da função agora aceita os 2 argumentos (ponteiros)
void AtualizarTelaModoJogo(GameScreen *telaAtual, ModoDeJogo *modo) {
// ------------------
    Vector2 mouse = GetMouseVirtual();

    int btnWidth = 300;
    int btnHeight = 70;
    int spacing = 20;
    int baseY = 350;

    Rectangle btnSolo = { (SCREEN_WIDTH - btnWidth) / 2.0f, baseY, btnWidth, btnHeight };
    Rectangle btnDois = { (SCREEN_WIDTH - btnWidth) / 2.0f, baseY + btnHeight + spacing, btnWidth, btnHeight };

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        
        if (CheckCollisionPointRec(mouse, btnSolo)){
            *telaAtual = SCREEN_SELECAO; // Vai para a seleção
            *modo = MODO_SOLO;          // Define o modo para Solo
        } 
        // --- LÓGICA ATIVADA ---
        else if (CheckCollisionPointRec(mouse, btnDois)) {
            *telaAtual = SCREEN_SELECAO; // Vai para a seleção
            *modo = MODO_PVP;           // Define o modo para PvP
        }
        // ------------------
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
    int btnHeight = 70;
    int spacing = 20;
    int baseY = 350;

    Rectangle btnSolo = { (SCREEN_WIDTH - btnWidth) / 2.0f, baseY, btnWidth, btnHeight };
    Rectangle btnDois = { (SCREEN_WIDTH - btnWidth) / 2.0f, baseY + btnHeight + spacing, btnWidth, btnHeight };

    Vector2 mouse = GetMouseVirtual();

    Color corTexto = { 100, 255, 100, 255 };
    Color corFundoBtn = { 50, 50, 50, 220 };
    Color corBordaBtn = { 10, 10, 10, 220 };
    Color corFundoHover = { 120, 255, 120, 220 };
    Color corTextoHover = BLACK;

    float roundness = 0.5f;
    int segments = 10;
    float lineThick = 4.0f;


    // Botão SOLO
    if (CheckCollisionPointRec(mouse, btnSolo)) {
        DrawRectangleRounded(btnSolo, roundness, segments, corFundoHover);
        DrawRectangleRoundedLinesEx(btnSolo, roundness, segments, lineThick, corBordaBtn);
        DrawText("SOLO", btnSolo.x + (btnSolo.width - MeasureText("SOLO", 30)) / 2, btnSolo.y + 20, 30, corTextoHover);
    } else {
        DrawRectangleRounded(btnSolo, roundness, segments, corFundoBtn);
        DrawRectangleRoundedLinesEx(btnSolo, roundness, segments, lineThick, corBordaBtn);
        DrawText("SOLO", btnSolo.x + (btnSolo.width - MeasureText("SOLO", 30)) / 2, btnSolo.y + 20, 30, corTexto);
    }

    // Botão 2 JOGADORES
    if (CheckCollisionPointRec(mouse, btnDois)) {
        DrawRectangleRounded(btnDois, roundness, segments, corFundoHover);
        DrawRectangleRoundedLinesEx(btnDois, roundness, segments, lineThick, corBordaBtn);
        DrawText("2 JOGADORES", btnDois.x + (btnDois.width - MeasureText("2 JOGADORES", 30)) / 2, btnDois.y + 20, 30, corTextoHover);
    } else {
        DrawRectangleRounded(btnDois, roundness, segments, corFundoBtn);
        DrawRectangleRoundedLinesEx(btnDois, roundness, segments, lineThick, corBordaBtn);
        DrawText("2 JOGADORES", btnDois.x + (btnDois.width - MeasureText("2 JOGADORES", 30)) / 2, btnDois.y + 20, 30, corTexto);
    }
}