#include "personagens.h"
#include "raylib.h"
#include "database.h"
#include <stdio.h> 
#include <string.h> 
#include "telas.h"


static int animFrame[9] = {0};
static int animTimer[9] = {0};
static int animVelocidade = 10; 
static int animFrameSelecionado = 0;
static int animTimerSelecionado = 0;

static Rectangle rectPersonagens[9];
static Color corTituloLinha = { 100, 255, 100, 255 }; 
static Color corNomePersonagem = RAYWHITE;
static Color corNomeSelecionado = YELLOW;
static Color corPainel = { 50, 50, 50, 200 };
static Color corBordaPainel = { 200, 0, 0, 255 }; 

static Texture2D backgroundTexture;

void CarregarRecursosPersonagens(void) {
    int linhaFrenteY = 200;
    int linhaMeioY = 450;
    int linhaTrasY = 700;
    
    int coluna1X = 100;
    int coluna2X = 450;
    int coluna3X = 800;
    
    int hitboxWidth = 300;
    int hitboxHeight = 180;

    for (int i = 0; i < 9; i++) {
        int linha = i / 3;
        int col = i % 3;
        
        int yPos = linhaFrenteY;
        if (linha == 1) yPos = linhaMeioY;
        if (linha == 2) yPos = linhaTrasY;
        
        int xPos = coluna1X;
        if (col == 1) xPos = coluna2X;
        if (col == 2) xPos = coluna3X;
        
        rectPersonagens[i] = (Rectangle){ (float)xPos, (float)yPos, (float)hitboxWidth, (float)hitboxHeight };
    }
    
    Image bgImg = LoadImage("sprites/background/background5.png");
    ImageResize(&bgImg, SCREEN_WIDTH, SCREEN_HEIGHT); 
    backgroundTexture = LoadTextureFromImage(bgImg);
    UnloadImage(bgImg);
}

void DescarregarRecursosPersonagens(void) {
    UnloadTexture(backgroundTexture);
}

void AtualizarTelaPersonagens(GameScreen *telaAtual, int *personagemSelecionado, SpriteDatabase* db) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        *telaAtual = SCREEN_MENU;
        *personagemSelecionado = -1;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMouseVirtual();
        
        *personagemSelecionado = -1;
        for (int i = 0; i < db->numPersonagens; i++) {
            if (CheckCollisionPointRec(mousePos, rectPersonagens[i])) {
                *personagemSelecionado = i;
                animFrameSelecionado = 0;
                animTimerSelecionado = 0;
                break;
            }
        }
    }
    
    for (int i = 0; i < db->numPersonagens; i++) {
        animTimer[i]++;
        if (animTimer[i] > animVelocidade) {
            animTimer[i] = 0;
            animFrame[i]++;
        }
    }
    
    if (*personagemSelecionado != -1) {
        animTimerSelecionado++;
        if (animTimerSelecionado > animVelocidade) {
            animTimerSelecionado = 0;
            animFrameSelecionado++;
            AnimacaoData* anim = &db->personagens[*personagemSelecionado].animIdle;
            if (animFrameSelecionado >= anim->def.numFrames) {
                animFrameSelecionado = 0;
            }
        }
    }
}

static void DesenharPainelDetalhes(int idPersonagem, SpriteDatabase* db) {
    Rectangle painelDireito = { 1150, 100, 400, 750 };
    
    DrawRectangleRec(painelDireito, corPainel);
    DrawRectangleLinesEx(painelDireito, 5.0f, corBordaPainel);
    
    int posX = (int)painelDireito.x + 25;
    int posYBase = (int)painelDireito.y + 400;
    int tamFonteTitulo = 20;
    int tamFonteTexto = 18;

    PersonagemData* pData = NULL;
    if (idPersonagem >= 0 && idPersonagem < db->numPersonagens) {
        pData = &db->personagens[idPersonagem];
    }
    
    if (pData != NULL) {
        Texture2D thumb = pData->thumbnail;
        Rectangle thumbSource = { 0, 0, (float)thumb.width, (float)thumb.height };
        Vector2 previewCenter = { painelDireito.x + painelDireito.width / 2, painelDireito.y + 220 };
        Rectangle thumbDest = { previewCenter.x, previewCenter.y, 300, 300 };
        Vector2 thumbOrig = { thumbDest.width / 2, thumbDest.height / 2 };
        DrawTexturePro(thumb, thumbSource, thumbDest, thumbOrig, 0.0f, (Color){ 255, 255, 255, 80 });
        
        AnimacaoData* anim = &pData->animIdle;
        if (anim->def.numFrames > 0) {
            if (animFrameSelecionado >= anim->def.numFrames) animFrameSelecionado = 0;
            Rectangle frame = anim->def.frames[animFrameSelecionado];
            float zoom = pData->painelZoom;
            
            Rectangle animDest = { previewCenter.x, previewCenter.y, frame.width * zoom, frame.height * zoom };
            Vector2 animOrig = { animDest.width / 2, animDest.height / 2 };
            DrawTexturePro(anim->textura, frame, animDest, animOrig, 0, WHITE);
        }

        DrawText(pData->nome, posX, posYBase, 30, corNomeSelecionado);
        DrawText(pData->descricao, posX, posYBase + 35, 16, LIGHTGRAY);
        
        int posYBlocoStats = posYBase + 80;
        DrawText("PV (Pontos de Vida):", posX, posYBlocoStats, tamFonteTitulo, corTituloLinha);
        DrawText(TextFormat("%d", pData->hpMax), posX + 220, posYBlocoStats, tamFonteTexto, LIGHTGRAY);
        DrawText("Velocidade:", posX, posYBlocoStats + 25, tamFonteTitulo, corTituloLinha);
        DrawText(TextFormat("%d", pData->velocidade), posX + 220, posYBlocoStats + 25, tamFonteTexto, LIGHTGRAY);

        int posYBlocoAtaques = posYBase + 140;
        DrawText("Ataques:", posX, posYBlocoAtaques, tamFonteTitulo, corTituloLinha);
        
        DrawText(pData->ataque1.nome, posX, posYBlocoAtaques + 25, tamFonteTexto, LIGHTGRAY);
        DrawText(TextFormat("Dano: %d", pData->ataque1.dano), posX + 250, posYBlocoAtaques + 25, tamFonteTexto, LIGHTGRAY);
        
        DrawText(pData->ataque2.nome, posX, posYBlocoAtaques + 55, tamFonteTexto, LIGHTGRAY);
        DrawText(TextFormat("Dano: %d", pData->ataque2.dano), posX + 250, posYBlocoAtaques + 55, tamFonteTexto, LIGHTGRAY);
        
    } else {
        DrawText("Selecione um personagem", (int)painelDireito.x + 40, (int)painelDireito.y + 400, 25, LIGHTGRAY);
    }
}


void DesenharTelaPersonagens(int personagemSelecionado, SpriteDatabase* db) {

    DrawTexture(backgroundTexture, 0, 0, (Color){100, 100, 100, 255});
    
    DesenharPainelDetalhes(personagemSelecionado, db);

    DrawText("Personagens:", 50, 50, 60, corTituloLinha);

    int tamFonteTituloLinha = 40;
    int tamFonteNome = 30;
    
    const char* titulosClasses[] = {"LINHA DE FRENTE", "LINHA DO MEIO", "LINHA DE TRAS"};
    
    for (int c = 0; c < 3; c++) {
        ClassePersonagem classe = (ClassePersonagem)c;
        int yPos = 200 + 250 * c;
        int xPos = 100;
        
        DrawText(titulosClasses[c], xPos, yPos - 60, tamFonteTituloLinha, corTituloLinha);
        
        DrawRectangleRec((Rectangle){ 80, yPos - 10, 1000, 200 }, (Color){0, 0, 0, 100});
        
        int col = 0;
        for (int i = 0; i < db->numPersonagens; i++) {
            if (db->personagens[i].classe == classe) {
                xPos = 100 + 350 * col;

                
                Rectangle hitbox = rectPersonagens[i]; 
                Vector2 cardCenter = { hitbox.x + hitbox.width / 2, hitbox.y + hitbox.height / 2 };

                Texture2D thumb = db->personagens[i].thumbnail;
                if (thumb.id <= 0) continue; 
                
                Rectangle thumbSource = { 0, 0, (float)thumb.width, (float)thumb.height };
                float rotation = 5.0f;
                float borderSize = 8.0f;
                Color borderColor = (personagemSelecionado == i) ? corNomeSelecionado : (Color){30, 30, 30, 255};

                float zoom = (hitbox.height - 40) / thumbSource.height;
                if (thumbSource.width * zoom > (hitbox.width - 40)) {
                    zoom = (hitbox.width - 40) / thumbSource.width;
                }
                float photoWidth = thumbSource.width * zoom;
                float photoHeight = thumbSource.height * zoom;

                Rectangle borderDest = { cardCenter.x, cardCenter.y, photoWidth + borderSize, photoHeight + borderSize };
                Vector2 borderOrigin = { borderDest.width / 2, borderDest.height / 2 };
                Rectangle photoDest = { cardCenter.x, cardCenter.y, photoWidth, photoHeight };
                Vector2 photoOrigin = { photoWidth / 2, photoHeight / 2 };

                DrawRectanglePro(borderDest, borderOrigin, rotation, borderColor);
                DrawTexturePro(thumb, thumbSource, photoDest, photoOrigin, rotation, WHITE);
                

                Color cor = (personagemSelecionado == i) ? corNomeSelecionado : corNomePersonagem;
                DrawText(db->personagens[i].nome, xPos + 10, yPos + 10, tamFonteNome, cor);

                col++;
            }
        }
    }
    
    const char *aviso = "Pressione ESC ou ENTER para voltar ao Menu";
    int tamAviso = 20;
    int posXAviso = 1110;
    DrawText(aviso, posXAviso, SCREEN_HEIGHT - 40, tamAviso, LIGHTGRAY);
}