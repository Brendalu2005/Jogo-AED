#include "raylib.h"
#include "menu.h" 
#include <stdio.h>
#include "selecao.h" 

static Rectangle btnJogar;
static Rectangle btnPersonagens;
static Rectangle btnSobre;
static Color corTexto = { 100, 255, 100, 255 };
static Color corFundoBtn = { 50, 50, 50, 220 };
static Color corBordaBtn = { 10, 10, 10, 220 };
static Color corFundoHover = { 120, 255, 120, 220 }; 
static Color corTextoHover = BLACK;


MenuOpcao LoadMenuResources(void) {
    MenuOpcao res;
    
    const char *imagePath = "sprites/background/menu.png";

    Image imagem = LoadImage(imagePath);

    ImageResize(&imagem, SCREEN_WIDTH, SCREEN_HEIGHT);
    res.background = LoadTextureFromImage(imagem);
    UnloadImage(imagem);

    int btnWidth = 300;
    int btnHeight = 70;
    int spacing = 20;
    int baseY = 450; 

    btnJogar = (Rectangle){ 
        (SCREEN_WIDTH - btnWidth) / 2.0f, baseY, btnWidth, btnHeight 
    };
    btnPersonagens = (Rectangle){ 
        (SCREEN_WIDTH - btnWidth) / 2.0f, baseY + btnHeight + spacing, btnWidth, btnHeight 
    };
    btnSobre = (Rectangle){ 
        (SCREEN_WIDTH - btnWidth) / 2.0f, baseY + (btnHeight + spacing) * 2, btnWidth, btnHeight 
    };

    return res;
}

void UnloadMenuResources(MenuOpcao resources) {
    UnloadTexture(resources.background);
}

void AtualizarTelaMenu(GameScreen *telaAtual) {
    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, btnJogar)) {
            // --- ATUALIZADO ---
            *telaAtual = SCREEN_SELECAO; 
            // ------------------
        } else if (CheckCollisionPointRec(mousePos, btnPersonagens)) {
            *telaAtual = SCREEN_PERSONAGENS;
        } else if (CheckCollisionPointRec(mousePos, btnSobre)) {
            *telaAtual = SCREEN_SOBRE;
        }
    }
}

void DesenharTelaMenu(MenuOpcao resources) {
    Vector2 mousePos = GetMousePosition();
    
    DrawTexture(resources.background, 0, 0, WHITE);

    const char *titulo = "Ecos da Infância";
    int tamTitulo = 100;
    int posXtitulo = (SCREEN_WIDTH - MeasureText(titulo, tamTitulo)) / 2;
    DrawText(titulo, posXtitulo + 4, 154, tamTitulo, (Color){0, 0, 0, 150});
    DrawText(titulo, posXtitulo, 150, tamTitulo, corTexto); 

    float btnRoundness = 0.5f; 
    int btnSegments = 10;
    float lineThick = 4.0f;

    // botão Jogar 
    if (CheckCollisionPointRec(mousePos, btnJogar)) {
        DrawRectangleRounded(btnJogar, btnRoundness, btnSegments, corFundoHover);
        DrawRectangleRoundedLinesEx(btnJogar, btnRoundness, btnSegments, lineThick, corBordaBtn);
        DrawText("JOGAR", btnJogar.x + (btnJogar.width - MeasureText("JOGAR", 30)) / 2, btnJogar.y + 20, 30, corTextoHover);
    } else {
        DrawRectangleRounded(btnJogar, btnRoundness, btnSegments, corFundoBtn);
        DrawRectangleRoundedLinesEx(btnJogar, btnRoundness, btnSegments, lineThick, corBordaBtn);
        DrawText("JOGAR", btnJogar.x + (btnJogar.width - MeasureText("JOGAR", 30)) / 2, btnJogar.y + 20, 30, corTexto);
    }

    // botão Personagens 
    if (CheckCollisionPointRec(mousePos, btnPersonagens)) {
        DrawRectangleRounded(btnPersonagens, btnRoundness, btnSegments, corFundoHover);
        DrawRectangleRoundedLinesEx(btnPersonagens, btnRoundness, btnSegments, lineThick, corBordaBtn);
        DrawText("PERSONAGENS", btnPersonagens.x + (btnPersonagens.width - MeasureText("PERSONAGENS", 30)) / 2, btnPersonagens.y + 20, 30, corTextoHover);
    } else {
        DrawRectangleRounded(btnPersonagens, btnRoundness, btnSegments, corFundoBtn);
        DrawRectangleRoundedLinesEx(btnPersonagens, btnRoundness, btnSegments, lineThick, corBordaBtn);
        DrawText("PERSONAGENS", btnPersonagens.x + (btnPersonagens.width - MeasureText("PERSONAGENS", 30)) / 2, btnPersonagens.y + 20, 30, corTexto);
    }

    // botão Sobre 
    if (CheckCollisionPointRec(mousePos, btnSobre)) {
        DrawRectangleRounded(btnSobre, btnRoundness, btnSegments, corFundoHover);
        DrawRectangleRoundedLinesEx(btnSobre, btnRoundness, btnSegments, lineThick, corBordaBtn);
        DrawText("SOBRE", btnSobre.x + (btnSobre.width - MeasureText("SOBRE", 30)) / 2, btnSobre.y + 20, 30, corTextoHover);
    } else {
        DrawRectangleRounded(btnSobre, btnRoundness, btnSegments, corFundoBtn);
        DrawRectangleRoundedLinesEx(btnSobre, btnRoundness, btnSegments, lineThick, corBordaBtn);
        DrawText("SOBRE", btnSobre.x + (btnSobre.width - MeasureText("SOBRE", 30)) / 2, btnSobre.y + 20, 30, corTexto);
    }
}