#include "raylib.h"
#include "menu.h" 
#include <stdio.h>
#include "selecao.h" 
#include "telas.h"
#include <math.h> 

static Rectangle btnJogar;
static Rectangle btnPersonagens;
static Rectangle btnSobre;

static Color corTexto = { 100, 255, 100, 255 };

MenuOpcao LoadMenuResources(void) {
    MenuOpcao res;
    
    const char *imagePath = "sprites/background/background6.png";
    Image imagem = LoadImage(imagePath);
    ImageResize(&imagem, SCREEN_WIDTH, SCREEN_HEIGHT);
    res.background = LoadTextureFromImage(imagem);
    UnloadImage(imagem);

    res.btnJogarTex = LoadTexture("sprites/botoes/jogar.png");
    res.btnPersonagensTex = LoadTexture("sprites/botoes/personagens.png");
    res.btnSobreTex = LoadTexture("sprites/botoes/sobre.png");
    res.btnVoltarTex = LoadTexture("sprites/botoes/voltar.png");
    res.btnSoloTex = LoadTexture("sprites/botoes/jogadorxia.png");
    res.btnPvPTex = LoadTexture("sprites/botoes/jogadorxjogador.png");

    res.fontPressStart = LoadFont("resources/fonts/PressStart2P.ttf");

    int btnWidth = 300;
    int btnHeight = 100;
    int spacing = 20;
    int baseY = 400; 

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
    
    UnloadTexture(resources.btnJogarTex);
    UnloadTexture(resources.btnPersonagensTex);
    UnloadTexture(resources.btnSobreTex);
    UnloadTexture(resources.btnVoltarTex);
    UnloadTexture(resources.btnSoloTex);
    UnloadTexture(resources.btnPvPTex);

    UnloadFont(resources.fontPressStart);
}

void AtualizarTelaMenu(GameScreen *telaAtual) {
    Vector2 mousePos = GetMouseVirtual(); 

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, btnJogar)) {
            *telaAtual = SCREEN_MODO_JOGO; 
        } else if (CheckCollisionPointRec(mousePos, btnPersonagens)) {
            *telaAtual = SCREEN_PERSONAGENS;
        } else if (CheckCollisionPointRec(mousePos, btnSobre)) { // Corrigi o typo 'CheckCollisionDPointRec'
            *telaAtual = SCREEN_SOBRE;
        }
    }
}

void DesenharTelaMenu(MenuOpcao resources) {
    
    Vector2 mousePos = GetMouseVirtual();
    
    DrawTexture(resources.background, 0, 0, WHITE);
    
    const char *titulo = "Ecos da Infância";
    int tamTitulo = 100;
    int posXtitulo = (SCREEN_WIDTH - MeasureText(titulo, tamTitulo)) / 2;
    DrawText(titulo, posXtitulo + 4, 154, tamTitulo, (Color){0, 0, 0, 150});
    DrawText(titulo, posXtitulo, 150, tamTitulo, corTexto); 

    Rectangle texSource = { 0, 0, resources.btnJogarTex.width, resources.btnJogarTex.height };
    Vector2 origin = { 0, 0 };

    Color tintJogar = CheckCollisionPointRec(mousePos, btnJogar) ? LIGHTGRAY : WHITE;
    DrawTexturePro(resources.btnJogarTex, texSource, btnJogar, origin, 0, tintJogar);

    Color tintPersonagens = CheckCollisionPointRec(mousePos, btnPersonagens) ? LIGHTGRAY : WHITE;
    DrawTexturePro(resources.btnPersonagensTex, texSource, btnPersonagens, origin, 0, tintPersonagens);

    Color tintSobre = CheckCollisionPointRec(mousePos, btnSobre) ? LIGHTGRAY : WHITE;
    DrawTexturePro(resources.btnSobreTex, texSource, btnSobre, origin, 0, tintSobre);
}