#include "selecao.h"
#include "database.h"
#include "telas.h"
#include <stdlib.h> 
#include <time.h>  
#include <stdio.h> 

// Define um limite máximo de personagens que o array de animacao pode suportar
#define MAX_PERSONAGENS 50

static int animFrame[MAX_PERSONAGENS] = {0};
static int animTimer[MAX_PERSONAGENS] = {0};
static int animVelocidade = 15;


static int etapaSelecao = 0;

static const char* titulosEtapa[] = {
    "Escolha seu personagem da LINHA DE FRENTE",
    "Escolha seu personagem da LINHA DO MEIO",
    "Escolha seu personagem da LINHA DE TRAS"
};

static int ContarPersonagensPorClasse(SpriteDatabase* db, ClassePersonagem classe) {
    int cont = 0;
    for (int i = 0; i < db->numPersonagens; i++) {
        if (db->personagens[i].classe == classe) {
            cont++;
        }
    }
    return cont;
}

static PersonagemData* GetPersonagemPorClasse(SpriteDatabase* db, ClassePersonagem classe, int indice) {
    int cont = 0;
    for (int i = 0; i < db->numPersonagens; i++) {
        if (db->personagens[i].classe == classe) {
            if (cont == indice) {
                return &db->personagens[i];
            }
            cont++;
        }
    }
    return NULL;
}

static void SelecionarTimeIA(SpriteDatabase* db, TimesBatalha* times) {
    printf("IA escolhendo time...\n");
    int contFrente = ContarPersonagensPorClasse(db, CLASSE_LINHA_FRENTE);
    int contMeio = ContarPersonagensPorClasse(db, CLASSE_LINHA_MEIO);
    int contTras = ContarPersonagensPorClasse(db, CLASSE_LINHA_TRAS);

    do {
        times->timeIA[0] = GetPersonagemPorClasse(db, CLASSE_LINHA_FRENTE, rand() % contFrente);
    } while (times->timeIA[0] == times->timeJogador[0] && contFrente > 1);
    
    do {
        times->timeIA[1] = GetPersonagemPorClasse(db, CLASSE_LINHA_MEIO, rand() % contMeio);
    } while (times->timeIA[1] == times->timeJogador[1] && contMeio > 1);

    do {
        times->timeIA[2] = GetPersonagemPorClasse(db, CLASSE_LINHA_TRAS, rand() % contTras);
    } while (times->timeIA[2] == times->timeJogador[2] && contTras > 1);


    printf("IA Time: %s, %s, %s\n", times->timeIA[0]->nome, times->timeIA[1]->nome, times->timeIA[2]->nome);
}

void InicializarSelecao(TimesBatalha* times) {
    etapaSelecao = 0;
    for(int i=0; i<3; i++) {
        times->timeJogador[i] = NULL;
        times->timeIA[i] = NULL;
    }
    srand(time(NULL)); 
}

void AtualizarTelaSelecao(GameScreen *telaAtual, SpriteDatabase* db, TimesBatalha* times) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        *telaAtual = SCREEN_MENU;
        return;
    }

    int cardWidth = (SCREEN_WIDTH - 200) / 3 - 20;
    int cardHeight = 120; 
    int yPosBase = 250;
    int yEspacamento = 200;

    if (etapaSelecao < 3) {
        ClassePersonagem classeAtual = (ClassePersonagem)etapaSelecao;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMouseVirtual(); // Usa a nova função
            
            int yPos = yPosBase + yEspacamento * etapaSelecao; 
            int xPos = 100;
            
            for (int i = 0; i < db->numPersonagens; i++) {
                 if (db->personagens[i].classe == classeAtual) {
                    Rectangle hitbox = { (float)xPos, (float)yPos, (float)cardWidth, (float)cardHeight };
                     if (CheckCollisionPointRec(mousePos, hitbox)) {
                        times->timeJogador[etapaSelecao] = &db->personagens[i];
                        printf("Jogador escolheu: %s\n", times->timeJogador[etapaSelecao]->nome);
                        etapaSelecao++; 
                        break; 
                    }
                    xPos += cardWidth + 20; 
                 }
            }
        }
    }

    // Loop de animacao corrigido (nao mais limitado a 9)
    for (int i = 0; i < db->numPersonagens; i++) {
        // Checa se o indice esta dentro dos limites do array de animacao
        if (i < MAX_PERSONAGENS) { 
            animTimer[i]++;
            if (animTimer[i] > animVelocidade) {
                animTimer[i] = 0;
                animFrame[i]++;
                AnimacaoData* anim = &db->personagens[i].animIdle;
                if (anim->def.numFrames > 0) { // Checa se a animação existe
                    if (animFrame[i] >= anim->def.numFrames) {
                        animFrame[i] = 0;
                    }
                } else {
                    animFrame[i] = 0;
                }
            }
        }
    }
    
    if (etapaSelecao == 3) {
        SelecionarTimeIA(db, times);
        etapaSelecao = 4;
        *telaAtual = SCREEN_BATALHA;
    }
}

void DesenharTelaSelecao(SpriteDatabase* db, TimesBatalha* times) {
    ClearBackground(DARKGRAY);

    if (etapaSelecao >= 3) {
        DrawText("CARREGANDO BATALHA...", 200, 400, 40, WHITE);
        return;
    }

    const char* titulo = titulosEtapa[etapaSelecao];
    DrawText(titulo, (SCREEN_WIDTH - MeasureText(titulo, 40)) / 2, 80, 40, WHITE);

    int cardWidth = (SCREEN_WIDTH - 200) / 3 - 20;
    int cardHeight = 120;
    int yPosBase = 250;
    int yEspacamento = 200;
    
    ClassePersonagem classes[] = {CLASSE_LINHA_FRENTE, CLASSE_LINHA_MEIO, CLASSE_LINHA_TRAS};
    const char* nomesClasses[] = {"Linha de Frente", "Linha do Meio", "Linha de Trás"};
    
    for (int c = 0; c < 3; c++) {
        ClassePersonagem classeAtual = classes[c];
        
        int yPos = yPosBase + yEspacamento * c;
        
        // Desenha o título (sem operador ternario)
        Color corTitulo;
        if (c == etapaSelecao) {
            corTitulo = YELLOW;
        } else {
            corTitulo = WHITE;
        }
        DrawText(nomesClasses[c], 50, yPos - 50, 30, corTitulo);
        
        int xPos = 100;
        
        for (int i = 0; i < db->numPersonagens; i++) {
            if (db->personagens[i].classe == classeAtual) {
                
                Rectangle card = { (float)xPos, (float)yPos, (float)cardWidth, (float)cardHeight };
                
                // Define a cor (sem operador ternario)
                Color cor;
                if (c == etapaSelecao) {
                    cor = LIGHTGRAY; // Classe atual
                } else {
                    cor = GRAY;
                }
                
                if (times->timeJogador[c] == &db->personagens[i]) {
                    cor = GREEN; // Já selecionado
                }
                
                DrawRectangleRec(card, cor);
                DrawRectangleLines((int)card.x, (int)card.y, (int)card.width, (int)card.height, WHITE);
                
                AnimacaoData* anim = &db->personagens[i].animIdle;
                
                // Checa se o personagem 'i' esta dentro dos limites do array de animacao
                if (i < MAX_PERSONAGENS) {
                    if (anim->def.numFrames > 0) {
                        Rectangle frame = anim->def.frames[animFrame[i]];

                        Rectangle areaAnim = { card.x + 10, card.y + 10, (card.width / 2.0f) - 20, card.height - 20 };

                        float zoom = areaAnim.height / frame.height;
                        
                        if (frame.width * zoom > areaAnim.width) {
                            zoom = areaAnim.width / frame.width;
                        }

                        DrawTexturePro(anim->textura, frame,
                            (Rectangle){ areaAnim.x + areaAnim.width / 2, areaAnim.y + areaAnim.height / 2, frame.width * zoom, frame.height * zoom },
                            (Vector2){ (frame.width * zoom) / 2, (frame.height * zoom) / 2 }, 0, WHITE);
                    }
                }
                
                int textoX = (int)card.x + (int)card.width / 2; 
                int textoY = (int)card.y + 20;

                DrawText(db->personagens[i].nome, textoX + 10, textoY, 20, BLACK);
                DrawText(TextFormat("HP: %d", db->personagens[i].hpMax), textoX + 10, textoY + 30, 18, BLACK);
                DrawText(TextFormat("Vel: %d", db->personagens[i].velocidade), textoX + 10, textoY + 55, 18, BLACK);
                
                xPos += cardWidth + 20; 
            }
        }
    }
    
    DrawText("Pressione ESC para voltar ao Menu", 10, SCREEN_HEIGHT - 30, 20, WHITE);
}