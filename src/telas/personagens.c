#include "personagens.h"
#include "raylib.h"
#include "database.h"
#include <stdio.h> 
#include <string.h> 


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

void CarregarRecursosPersonagens(void) {
    int linhaFrenteY = 200;
    int linhaMeioY = 450;
    int linhaTrasY = 700;
    
    int coluna1X = 100;
    int coluna2X = 450;
    int coluna3X = 800; 
    
    int hitboxWidth = 300; 
    int hitboxHeight = 180; 

    // Define as hitboxes (usando IDs de 0 a 8)
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
}

void DescarregarRecursosPersonagens(void) {
}

void AtualizarTelaPersonagens(GameScreen *telaAtual, int *personagemSelecionado, SpriteDatabase* db) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        *telaAtual = SCREEN_MENU;
        *personagemSelecionado = -1; 
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
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
            AnimacaoData* anim = &db->personagens[i].animIdle;
            if (animFrame[i] >= anim->def.numFrames) {
                animFrame[i] = 0;
            }
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
    Rectangle painelDireito = { 1150, 100, 400, 750 }; // Posição do painel
    
    DrawRectangleRec(painelDireito, corPainel);
    DrawRectangleLinesEx(painelDireito, 5.0f, corBordaPainel);
    
    // Área para a animação grande
    Rectangle areaAnimacao = { painelDireito.x + 25, painelDireito.y + 25, painelDireito.width - 50, 300 };
    DrawRectangleRec(areaAnimacao, PURPLE);

    int posX = (int)painelDireito.x + 25;
    int posYBase = (int)areaAnimacao.y + (int)areaAnimacao.height + 20; 
    int tamFonteTitulo = 20;
    int tamFonteTexto = 18;

    int espacamentoLinha1 = 25;
    int espacamentoLinha2 = 45;
    int espacamentoBloco = 100; 

    PersonagemData* pData = NULL;
    if (idPersonagem >= 0 && idPersonagem < db->numPersonagens) {
        pData = &db->personagens[idPersonagem];
    }
    
    if (pData != NULL) {
        AnimacaoData* anim = &pData->animIdle;
        if (anim->def.numFrames > 0) {
            Rectangle frame = anim->def.frames[animFrameSelecionado];
            float zoom = pData->painelZoom;
            
            DrawTexturePro(anim->textura, frame, 
                (Rectangle){ areaAnimacao.x + areaAnimacao.width / 2, areaAnimacao.y + areaAnimacao.height / 2, frame.width * zoom, frame.height * zoom },
                (Vector2){ (frame.width * zoom) / 2, (frame.height * zoom) / 2 }, 0, WHITE);
        }

        DrawText("Nome:", posX, posYBase, tamFonteTitulo, corTituloLinha);
        DrawText(pData->nome, posX, posYBase + espacamentoLinha1, tamFonteTexto, LIGHTGRAY);
        DrawText(pData->descricao, posX, posYBase + espacamentoLinha2, 16, LIGHTGRAY); 
        
        int posYBlocoAtaques = posYBase + espacamentoBloco;
        DrawText("Ataques:", posX, posYBlocoAtaques, tamFonteTitulo, corTituloLinha);
        DrawText(pData->ataque1.nome, posX, posYBlocoAtaques + espacamentoLinha1, tamFonteTexto, LIGHTGRAY);
        DrawText(pData->ataque1.descricao, posX + 10, posYBlocoAtaques + espacamentoLinha1 + 25, 16, LIGHTGRAY);
        DrawText(TextFormat("Dano: %d", pData->ataque1.dano), posX + 10, posYBlocoAtaques + espacamentoLinha1 + 45, 16, LIGHTGRAY);
        
        DrawText(pData->ataque2.nome, posX, posYBlocoAtaques + espacamentoLinha1 + 75, tamFonteTexto, LIGHTGRAY);
        DrawText(pData->ataque2.descricao, posX + 10, posYBlocoAtaques + espacamentoLinha1 + 100, 16, LIGHTGRAY);
        DrawText(TextFormat("Dano: %d", pData->ataque2.dano), posX + 10, posYBlocoAtaques + espacamentoLinha1 + 120, 16, LIGHTGRAY);
        
        int posYBlocoPV = posYBase + (espacamentoBloco * 2) + 100;
        DrawText("PV (Pontos de Vida):", posX, posYBlocoPV, tamFonteTitulo, corTituloLinha);
        DrawText(TextFormat("%d", pData->hpMax), posX, posYBlocoPV + espacamentoLinha1, tamFonteTexto, LIGHTGRAY);
    } else {
        DrawText("Selecione um personagem", (int)painelDireito.x + 40, (int)painelDireito.y + 400, 25, LIGHTGRAY);
    }
}


void DesenharTelaPersonagens(int personagemSelecionado, SpriteDatabase* db) {
    ClearBackground(DARKGRAY);
    
    DesenharPainelDetalhes(personagemSelecionado, db);

    DrawText("Personagens:", 50, 50, 60, corTituloLinha);

    int tamFonteTituloLinha = 40;
    int tamFonteNome = 30;
    
    const char* titulosClasses[] = {"LINHA DE FRENTE", "LINHA DO MEIO", "LINHA DE TRAS"};
    
    for (int c = 0; c < 3; c++) { // Loop de Classes (Linhas)
        ClassePersonagem classe = (ClassePersonagem)c;
        int yPos = 200 + 250 * c;
        int xPos = 100;
        
        DrawText(titulosClasses[c], xPos, yPos - 60, tamFonteTituloLinha, corTituloLinha);
        
        int col = 0;
        for (int i = 0; i < db->numPersonagens; i++) { 
            if (db->personagens[i].classe == classe) {
                xPos = 100 + 350 * col;
                
                // Define a hitbox (baseado no CarregarRecursosPersonagens)
                Rectangle card = rectPersonagens[i];
                DrawRectangleRec(card, (Color){ 30, 30, 30, 200 });
                if (personagemSelecionado == i) {
                    DrawRectangleLinesEx(card, 3.0f, corNomeSelecionado);
                }

                // Desenha a animação idle
                AnimacaoData* anim = &db->personagens[i].animIdle;
                if (anim->def.numFrames > 0) {
                    Rectangle frame = anim->def.frames[animFrame[i]];
                    float zoom = (card.height - 40) / frame.height; 
                    DrawTexturePro(anim->textura, frame,
                        (Rectangle){ card.x + card.width/2, card.y + card.height/2 - 10, frame.width * zoom, frame.height * zoom },
                        (Vector2){ (frame.width * zoom) / 2, (frame.height * zoom) / 2 }, 0, WHITE);
                }

                // Desenha o nome
                Color cor = (personagemSelecionado == i) ? corNomeSelecionado : corNomePersonagem;
                DrawText(db->personagens[i].nome, xPos + 10, yPos + 10, tamFonteNome, cor);

                col++;
            }
        }
    }
    
    const char *aviso = "Pressione ESC ou ENTER para voltar ao Menu";
    int tamAviso = 20;
    int posXAviso = (SCREEN_WIDTH - MeasureText(aviso, tamAviso)) / 2;
    DrawText(aviso, posXAviso, SCREEN_HEIGHT - 40, tamAviso, LIGHTGRAY);
}