#include "personagens.h"
#include "raylib.h"
#include "database.h"
#include <stdio.h> 
#include <string.h> 
#include "telas.h"

#define MAX_PERSONAGENS 50

static int animFrame[MAX_PERSONAGENS] = {0};
static int animTimer[MAX_PERSONAGENS] = {0};
static int animVelocidade = 10; 
static int animFrameSelecionado = 0;
static int animTimerSelecionado = 0;

static int personagemHover = -1;


static Color corTituloLinha = { 100, 255, 100, 255 }; 
static Color corNomePersonagem = RAYWHITE;
static Color corNomeSelecionado = YELLOW;
static Color corPainel = { 50, 50, 50, 200 };
static Color corBordaPainel = { 200, 0, 0, 255 }; 

static Texture2D backgroundTexture;

void CarregarRecursosPersonagens(void) {
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

    int hitboxWidth = 130;
    int hitboxHeight = 180;


    for (int i = 0; i < db->numPersonagens; i++) {
        if (i < MAX_PERSONAGENS) { 
            animTimer[i]++;
            if (animTimer[i] > animVelocidade) {
                animTimer[i] = 0;
                animFrame[i]++;

                AnimacaoData* anim = &db->personagens[i].animIdle;
                if (anim->def.numFrames > 0) {
                    if (animFrame[i] >= anim->def.numFrames) {
                        animFrame[i] = 0;
                    }
                } else {
                    animFrame[i] = 0;
                }
            }
        }
    }


    Vector2 mousePos = GetMouseVirtual();
    personagemHover = -1; // Reseta o hover a cada frame

    for (int c = 0; c < 3; c++) {
        ClassePersonagem classe = (ClassePersonagem)c;
        int yPos = 200 + 250 * c;
        int col = 0;
        int startX = 50;
        int horizontalSpacing = 150;

        for (int i = 0; i < db->numPersonagens; i++) {
            if (db->personagens[i].classe == classe) {
                int xPos = startX + horizontalSpacing * col;
                Rectangle hitbox = { (float)xPos, (float)yPos, (float)hitboxWidth, (float)hitboxHeight };

                if (CheckCollisionPointRec(mousePos, hitbox)) {
                    personagemHover = i; // Define o personagem sob o cursor
                    break; 
                }
                col++; 
            }
        }
        if (personagemHover != -1) {
            break; 
        }
    }


    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

        *personagemSelecionado = -1;

        for (int c = 0; c < 3; c++) {
            ClassePersonagem classe = (ClassePersonagem)c;
            int yPos = 200 + 250 * c;
            int col = 0;

            int startX = 50;
            int horizontalSpacing = 150;


            for (int i = 0; i < db->numPersonagens; i++) {
                
                if (db->personagens[i].classe == classe) {
                    
                    int xPos = startX + horizontalSpacing * col;
                
                    Rectangle hitbox = { (float)xPos, (float)yPos, (float)hitboxWidth, (float)hitboxHeight };

                    if (CheckCollisionPointRec(mousePos, hitbox)) { // --- MODIFICADO --- (usa 'mousePos')
                        *personagemSelecionado = i;
                        animFrameSelecionado = 0;
                        animTimerSelecionado = 0;
                        break; 
                    }
                    col++; 
                }
            }
            
            if (*personagemSelecionado != -1) {
                break;
            }
        }
    }
    
    if (*personagemSelecionado != -1) {
        animTimerSelecionado++;
        if (animTimerSelecionado > animVelocidade) {
            animTimerSelecionado = 0;
            animFrameSelecionado++;
            AnimacaoData* anim = &db->personagens[*personagemSelecionado].animIdle;
            if (anim->def.numFrames > 0) {
                if (animFrameSelecionado >= anim->def.numFrames) {
                    animFrameSelecionado = 0;
                }
            } else {
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
            if (animFrameSelecionado >= anim->def.numFrames) {
                animFrameSelecionado = 0;
            }
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
    int tamFonteNome = 20;
    
    int hitboxWidth = 130; 
    int hitboxHeight = 180;
    
    const char* titulosClasses[] = {"LINHA DE FRENTE", "LINHA DO MEIO", "LINHA DE TRAS"};
    
    for (int c = 0; c < 3; c++) {
        ClassePersonagem classe = (ClassePersonagem)c;
        int yPos = 200 + 250 * c;

        int startX = 50;
        int horizontalSpacing = 150;
        
        DrawText(titulosClasses[c], startX, yPos - 60, tamFonteTituloLinha, corTituloLinha);
        
        DrawRectangleRec((Rectangle){ startX - 10, yPos - 10, 1050.0f, 170.0f }, (Color){0, 0, 0, 100});
        
        int col = 0;
        for (int i = 0; i < db->numPersonagens; i++) {
            if (db->personagens[i].classe == classe) {
                
                int xPos = startX + horizontalSpacing * col;
                
                Rectangle hitbox = { (float)xPos, (float)yPos, (float)hitboxWidth, (float)hitboxHeight }; 


                // 1. Define o tamanho fixo para a foto
                float frameWidth = 120.0f;
                float frameHeight = 110.0f;
                float borderSize = 6.0f; // Tamanho da borda
                

                float frameX = hitbox.x + (hitbox.width - frameWidth) / 2.0f;
                float frameY = hitbox.y + 15.0f;
                
                Rectangle borderRect = { frameX, frameY, frameWidth, frameHeight };
                
                // 3. Define a área interna da foto (dentro da moldura)
                Rectangle photoRect = { 
                    frameX + borderSize, 
                    frameY + borderSize, 
                    frameWidth - (borderSize * 2), 
                    frameHeight - (borderSize * 2) 
                };

                // 4. Define a cor da borda
                Color borderColor;
                if (personagemSelecionado == i) {
                    borderColor = corNomeSelecionado;
                } else if (personagemHover == i) { // Adiciona a verificação de hover
                    borderColor = LIGHTGRAY;
                } else {
                    borderColor = (Color){30, 30, 30, 255};
                }

                // 5. Desenha a borda
                DrawRectangleRec(borderRect, borderColor); 

                Texture2D thumb = db->personagens[i].thumbnail;
                if (thumb.id <= 0) {
                    DrawRectangleRec(photoRect, BLACK);
                } else {
                    // 6. Calcula o zoom para a foto caber na área interna (photoRect)
                    Rectangle thumbSource = { 0, 0, (float)thumb.width, (float)thumb.height };
                    
                    float zoom = photoRect.height / thumbSource.height;
                    float scaledWidth = thumbSource.width * zoom;
                    
                    if (scaledWidth > photoRect.width) {
                        zoom = photoRect.width / thumbSource.width;
                    }
                    
                    float photoWidth = thumbSource.width * zoom;
                    float photoHeight = thumbSource.height * zoom;

                    // 7. Define o destino da textura, centralizada dentro de photoRect
                    Rectangle photoDest = {
                        photoRect.x + (photoRect.width - photoWidth) / 2.0f,
                        photoRect.y + (photoRect.height - photoHeight) / 2.0f,
                        photoWidth,
                        photoHeight
                    };

                    // 8. Desenha a foto
                    DrawTexturePro(thumb, thumbSource, photoDest, (Vector2){0, 0}, 0.0f, WHITE);
                }
                
                Color cor;
                if (personagemSelecionado == i) {
                    cor = corNomeSelecionado;
                } else if (personagemHover == i) {
                    cor = LIGHTGRAY;
                } else {
                    cor = corNomePersonagem;
                }

                int textWidth = MeasureText(db->personagens[i].nome, tamFonteNome);
                int textY = (int)(frameY + frameHeight + 10);
                
                DrawText(
                    db->personagens[i].nome, 
                    (int)(hitbox.x + (hitbox.width - textWidth) / 2.0f),
                    textY, 
                    tamFonteNome, 
                    cor
                );


                col++; 
            }
        }
    }
    
    const char *aviso = "Pressione ESC ou ENTER para voltar ao Menu";
    int tamAviso = 20;
    int posXAviso = 1110;
    DrawText(aviso, posXAviso, SCREEN_HEIGHT - 40, tamAviso, LIGHTGRAY);
}