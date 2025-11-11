#include "raylib.h"
#include "sobre.h"
#include "menu.h"
#include "telas.h"

void AtualizarTelaSobre(GameScreen *telaAtual){
    if(IsKeyPressed(KEY_ESCAPE)){
        *telaAtual = SCREEN_MENU;
    }

    Vector2 mouse = GetMousePosition();
    Rectangle btnVoltar = { 50, 50, 150, 50 };

    
    if (CheckCollisionPointRec(mouse, btnVoltar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
        *telaAtual = SCREEN_MENU;
    }
}


void DesenharTelaSobre(MenuOpcao resoucers) {
    DrawTexture(resoucers.background, 0, 0, WHITE);

    char titulo[] = "Sobre o Jogo";
    int fontSize = 60;
    int posXtitulo = (SCREEN_WIDTH - MeasureText(titulo, fontSize)) / 2;
    DrawText(titulo, posXtitulo, 120, fontSize, RAYWHITE);

    char texto[] =
        "Ecos da Infância é um jogo educativo.\n"
        "O objetivo é resgatar memórias da infância.\n"
        "O jogador deve escolher um time e derrotar o oponente.\n";

    int fontSizeTexto = 28;
    int larguraReferencia = MeasureText("O jogador deve escolher um time e derrotar o oponente.", fontSizeTexto);
    int posXtexto = (SCREEN_WIDTH - larguraReferencia) / 2;
    int posYtexto = 260;

    // fundo preto transparente do texto
    int larguraFundo = larguraReferencia + 60;
    int alturaFundo = 130; // altura pro texto
    int posXFundo = posXtexto - 30;
    int posYFundo = posYtexto - 20;

    DrawRectangleRounded(
        (Rectangle){ posXFundo, posYFundo, larguraFundo, alturaFundo },
        0.1f, 8,
        (Color){ 0, 0, 0, 150 } // preto transparente
    );


    DrawText(texto, posXtexto, posYtexto, fontSizeTexto, LIGHTGRAY);

    Rectangle btnVoltar = { 50, 50, 150, 50 };
    DrawRectangleRounded(btnVoltar, 0.3f, 10, (Color){80, 80, 80, 255});
    DrawRectangleRoundedLinesEx(btnVoltar, 0.3f, 10, 2, WHITE);

    char textoBotao[] = "VOLTAR";
    DrawText(
        textoBotao,
        btnVoltar.x + (btnVoltar.width - MeasureText(textoBotao, 25)) / 2,
        btnVoltar.y + 12,
        25,
        RAYWHITE
    );
}
