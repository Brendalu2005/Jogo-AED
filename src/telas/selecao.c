#include "selecao.h"
#include "database.h"
#include "telas.h"
#include <stdlib.h> 
#include <time.h>  
#include <stdio.h> 
#include <math.h>
#include <stdbool.h> 

#define MAX_PERSONAGENS 50
#define TEMPO_DELAY_IA 1.0 // 1 segundo de espera para a IA


// Animação dos ícones na grid
static int animFrame[MAX_PERSONAGENS] = {0};
static int animTimer[MAX_PERSONAGENS] = {0};
static int animVelocidade = 15; 

// Animação dos slots selecionados
static int animFrameJogador[3] = {0, 0, 0};
static int animTimerJogador[3] = {0, 0, 0};
static int animFrameIA[3] = {0, 0, 0};
static int animTimerIA[3] = {0, 0, 0};

static int etapaSelecao = 0; 
static bool ehTurnoJogador = true; 
static int personagemHover = -1; 
static double tempoEsperaIA = 0; // Timer para o delay da IA

// Recursos gráficos
static Texture2D background;

// Textos das etapas
static const char* titulosEtapa[] = {
    "Escolha seu personagem da LINHA DE FRENTE",
    "Escolha seu personagem da LINHA DO MEIO",
    "Escolha seu personagem da LINHA DE TRAS",
    "CARREGANDO BATALHA..." 
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

static void AtualizarLogicaAnimacao(int* frame, int* timer, AnimacaoData* anim) {
    if (anim == NULL) {
        return;
    }
    
    (*timer)++;
    if ((*timer) > animVelocidade) {
        (*timer) = 0;
        (*frame)++;
        
        if (anim->def.numFrames > 0) {
            if ((*frame) >= anim->def.numFrames) {
                (*frame) = 0;
            }
        } else {
            (*frame) = 0;
        }
    }
}

static void SelecionarIA(SpriteDatabase* db, TimesBatalha* times, int etapa) {
    ClassePersonagem classe = (ClassePersonagem)etapa;
    int cont = ContarPersonagensPorClasse(db, classe);
    
    do {
        times->timeIA[etapa] = GetPersonagemPorClasse(db, classe, rand() % cont);
    } while (times->timeIA[etapa] == times->timeJogador[etapa] && cont > 1);
    
    printf("IA escolheu (%d): %s\n", etapa, times->timeIA[etapa]->nome);
}

void InicializarSelecao(TimesBatalha* times) {
    etapaSelecao = 0;
    personagemHover = -1;
    tempoEsperaIA = 0;
    ehTurnoJogador = true;

    for (int i = 0; i < 3; i++) {
        times->timeJogador[i] = NULL;
        times->timeIA[i] = NULL;
        
        animTimerJogador[i] = 0;
        animFrameJogador[i] = 0;
        animTimerIA[i] = 0;
        animFrameIA[i] = 0;
    }
    
    for (int i = 0; i < MAX_PERSONAGENS; i++) {
        animTimer[i] = 0;
        animFrame[i] = 0;
    }
    
    srand(time(NULL)); 

    Image bgImg = LoadImage("sprites/background/background3.jpg");
    ImageResize(&bgImg, SCREEN_WIDTH, SCREEN_HEIGHT);
    background = LoadTextureFromImage(bgImg);
    UnloadImage(bgImg);
}

void AtualizarTelaSelecao(GameScreen *telaAtual, SpriteDatabase* db, TimesBatalha* times) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        *telaAtual = SCREEN_MENU;
        UnloadTexture(background); 
        return;
    }

    for (int i = 0; i < db->numPersonagens; i++) {
        if (i < MAX_PERSONAGENS) { 
            AtualizarLogicaAnimacao(&animFrame[i], &animTimer[i], &db->personagens[i].animIdle);
        }
    }
    for (int i = 0; i < 3; i++) {
        AtualizarLogicaAnimacao(&animFrameJogador[i], &animTimerJogador[i], times->timeJogador[i] ? &times->timeJogador[i]->animIdle : NULL);
        AtualizarLogicaAnimacao(&animFrameIA[i], &animTimerIA[i], times->timeIA[i] ? &times->timeIA[i]->animIdle : NULL);
    }

    if (etapaSelecao == 3) {
        printf("CARREGANDO BATALHA...\n");
        *telaAtual = SCREEN_BATALHA;
        UnloadTexture(background); 
        return;
    }
    if (ehTurnoJogador) {
        Vector2 mousePos = GetMouseVirtual();
        personagemHover = -1;
        ClassePersonagem classeAtiva = (ClassePersonagem)etapaSelecao;

        int yPosBase = 500;
        int iconSize = 64;
        int padding = 10;
        int yEspacamentoLinha = 80;

        for (int c = 0; c < 3; c++) {
            ClassePersonagem classeLinha = (ClassePersonagem)c;
            int numPersonagensLinha = ContarPersonagensPorClasse(db, classeLinha);
            int larguraLinha = numPersonagensLinha * (iconSize + padding) - padding;
            int xPos = (SCREEN_WIDTH - larguraLinha) / 2;
            int yPos = yPosBase + c * yEspacamentoLinha;

            for (int i = 0; i < db->numPersonagens; i++) {
                if (db->personagens[i].classe == classeLinha) {
                    Rectangle hitbox = { (float)xPos, (float)yPos, (float)iconSize, (float)iconSize };

                    if (CheckCollisionPointRec(mousePos, hitbox)) {
                        personagemHover = i;
                    }
                    if (classeLinha == classeAtiva) {
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            if (CheckCollisionPointRec(mousePos, hitbox)) {
                                times->timeJogador[etapaSelecao] = &db->personagens[i];
                                printf("Jogador escolheu (%d): %s\n", etapaSelecao, times->timeJogador[etapaSelecao]->nome);
                                
                                ehTurnoJogador = false;
                                tempoEsperaIA = GetTime();
                                
                                personagemHover = -1;
                                break; 
                            }
                        }
                    }
                    xPos += iconSize + padding;
                }
            } 
            
            if (ehTurnoJogador == false) {
                break; 
            }
        } 
    } else {
        if (GetTime() - tempoEsperaIA > TEMPO_DELAY_IA) {
            SelecionarIA(db, times, etapaSelecao);
            
            etapaSelecao++;
            ehTurnoJogador = true;
        }
    }
}
void DesenharTelaSelecao(SpriteDatabase* db, TimesBatalha* times) {
    ClearBackground(DARKGRAY);
    DrawTexture(background, 0, 0, WHITE);

    const char* tituloTela = "ESCOLHA SEU TIME";
    int tamTituloTela = 60;
    DrawText(tituloTela, (SCREEN_WIDTH - MeasureText(tituloTela, tamTituloTela)) / 2, 30, tamTituloTela, WHITE);

    const char* tituloInstrucao;
    if (ehTurnoJogador) {
        tituloInstrucao = titulosEtapa[etapaSelecao];
    } else {
        tituloInstrucao = "IA esta escolhendo...";
    }
    
    if (etapaSelecao < 3) {
        int tamInstrucao = 30;
        DrawText(tituloInstrucao, (SCREEN_WIDTH - MeasureText(tituloInstrucao, tamInstrucao)) / 2, 100, tamInstrucao, YELLOW);
    } else {
        int tamInstrucao = 30;
        DrawText(titulosEtapa[3], (SCREEN_WIDTH - MeasureText(titulosEtapa[3], tamInstrucao)) / 2, 100, tamInstrucao, YELLOW);
    }
    
    Rectangle slotP1[3] = {
        { 50, 180, 150, 150 }, 
        { 50, 340, 150, 150 }, 
        { 50, 500, 150, 150 }  
    };
    
    for (int i = 0; i < 3; i++) {
        DrawRectangleRec(slotP1[i], (Color){ 0, 0, 0, 150 });
        
        if (times->timeJogador[i] != NULL) {
            PersonagemData* pData = times->timeJogador[i];
            AnimacaoData* anim = &pData->animIdle;
            Texture2D thumb = pData->thumbnail;

            DrawTexturePro(thumb, 
                           (Rectangle){ 0, 0, (float)thumb.width, (float)thumb.height }, 
                           slotP1[i], 
                           (Vector2){ 0, 0 }, 0, (Color){ 255, 255, 255, 80 });

            if (anim->def.numFrames > 0) {
                int frameIdx = animFrameJogador[i];
                Rectangle frame = anim->def.frames[frameIdx];
                
                float zoom = slotP1[i].height / frame.height;
                if (frame.width * zoom > slotP1[i].width) {
                    zoom = slotP1[i].width / frame.width;
                }
                
                Vector2 animPos = { slotP1[i].x + slotP1[i].width / 2.0f, slotP1[i].y + slotP1[i].height / 2.0f };

                DrawTexturePro(anim->textura, 
                               frame, 
                               (Rectangle){ animPos.x, animPos.y, frame.width * zoom, frame.height * zoom }, 
                               (Vector2){ (frame.width * zoom) / 2, (frame.height * zoom) / 2 }, 
                               0, WHITE);
            }
        } else {
            const char* slotTexto = TextFormat("P%d", i + 1);
            DrawText(slotTexto, slotP1[i].x + (slotP1[i].width - MeasureText(slotTexto, 40)) / 2, slotP1[i].y + 55, 40, DARKGRAY);
        }
        
        if (i == etapaSelecao && ehTurnoJogador) {
            DrawRectangleLinesEx(slotP1[i], 4, YELLOW);
        } else {
            DrawRectangleLinesEx(slotP1[i], 2, GRAY);
        }
    }

    Rectangle slotIA[3] = {
        { SCREEN_WIDTH - 200, 180, 150, 150 }, 
        { SCREEN_WIDTH - 200, 340, 150, 150 }, 
        { SCREEN_WIDTH - 200, 500, 150, 150 }  
    };
    
    for (int i = 0; i < 3; i++) {
        DrawRectangleRec(slotIA[i], (Color){ 0, 0, 0, 150 });
        
        if (times->timeIA[i] != NULL) {
            PersonagemData* pData = times->timeIA[i];
            AnimacaoData* anim = &pData->animIdle;
            Texture2D thumb = pData->thumbnail;

            DrawTexturePro(thumb, 
                           (Rectangle){ 0, 0, (float)thumb.width, (float)thumb.height }, 
                           slotIA[i], 
                           (Vector2){ 0, 0 }, 0, (Color){ 255, 255, 255, 80 });

            if (anim->def.numFrames > 0) {
                int frameIdx = animFrameIA[i];
                Rectangle frame = anim->def.frames[frameIdx];
                
                float zoom = slotIA[i].height / frame.height;
                if (frame.width * zoom > slotIA[i].width) {
                    zoom = slotIA[i].width / frame.width;
                }
                
                Vector2 animPos = { slotIA[i].x + slotIA[i].width / 2.0f, slotIA[i].y + slotIA[i].height / 2.0f };
                
                frame.width = -frame.width; 
                
                Rectangle destRect = { animPos.x, animPos.y, fabsf(frame.width * zoom), fabsf(frame.height * zoom) };
                Vector2 origin = { fabsf(frame.width * zoom) / 2.0f, fabsf(frame.height * zoom) / 2.0f };

                DrawTexturePro(anim->textura, frame, destRect, origin, 0, WHITE);
            }
        } else {
            const char* slotTexto = TextFormat("IA%d", i + 1);
            DrawText(slotTexto, slotIA[i].x + (slotIA[i].width - MeasureText(slotTexto, 40)) / 2, slotIA[i].y + 55, 40, DARKGRAY);
        }
        DrawRectangleLinesEx(slotIA[i], 2, GRAY);
    }

    // Grid de Personagens 
    int yPosBase = 500;
    int iconSize = 64;
    int padding = 10;
    int yEspacamentoLinha = 80;

    for (int c = 0; c < 3; c++) {
        ClassePersonagem classeLinha = (ClassePersonagem)c;
        int numPersonagensLinha = ContarPersonagensPorClasse(db, classeLinha);
        int larguraLinha = numPersonagensLinha * (iconSize + padding) - padding;
        int xPos = (SCREEN_WIDTH - larguraLinha) / 2;
        int yPos = yPosBase + c * yEspacamentoLinha;

        Color corIcone;
        if (c == etapaSelecao && ehTurnoJogador) {
            corIcone = WHITE; 
        } else {
            corIcone = GRAY; 
        }

        for (int i = 0; i < db->numPersonagens; i++) {
            if (db->personagens[i].classe == classeLinha) {
                Rectangle box = { (float)xPos, (float)yPos, (float)iconSize, (float)iconSize };
                Texture2D thumb = db->personagens[i].thumbnail;

                if (thumb.id > 0) {
                     DrawTexturePro(thumb, 
                                   (Rectangle){ 0, 0, (float)thumb.width, (float)thumb.height }, 
                                   box, (Vector2){ 0, 0 }, 0, corIcone);
                }

                if (personagemHover == i && c == etapaSelecao) {
                    DrawRectangleLinesEx(box, 3, YELLOW);
                }
                
                xPos += iconSize + padding;
            }
        }
    }

    // Preview Grande 
    if (personagemHover != -1 && ehTurnoJogador) {
        PersonagemData* pData = &db->personagens[personagemHover];
        AnimacaoData* anim = &pData->animIdle;

        if (anim->def.numFrames > 0) {
            Vector2 previewPos = { SCREEN_WIDTH / 2.0f, 280.0f };
            
            int frameIdx = animFrame[personagemHover]; // Usa a animação da grid
            Rectangle frame = anim->def.frames[frameIdx];
            float zoom = pData->painelZoom; 

            DrawTexturePro(anim->textura, 
                           frame, 
                           (Rectangle){ previewPos.x, previewPos.y, frame.width * zoom, frame.height * zoom }, 
                           (Vector2){ (frame.width * zoom) / 2, (frame.height * zoom) / 2 }, 
                           0, WHITE);
                           
            int tamNome = 25;
            DrawText(pData->nome, (SCREEN_WIDTH - MeasureText(pData->nome, tamNome)) / 2, 400, tamNome, WHITE);
        }
    }

    DrawText("Pressione ESC para voltar ao Menu", 10, SCREEN_HEIGHT - 30, 20, WHITE);
}