#include "batalha_desenho.h"
#include "lista_personagem.h" // Precisa de ObterNoNaPosicao
#include <math.h>             // Precisa de fmod
#include <stdio.h>            // Precisa de TextFormat
#include <string.h>           // Precisa de strlen

// -------------------------------------------------------------------
// FUNÇÕES AUXILIARES DE DESENHO (MOVIDAS DE 'batalha.c')
// Elas agora são 'static' pois só este arquivo precisa delas.
// -------------------------------------------------------------------

static void DesenharAnimacaoLapide(EstadoAnimacao* anim) {
    if (anim->ativo == false){ 
        return; 
    }
    if (anim->anim == NULL){ 
        return; 
    }
    if (anim->anim->def.numFrames == 0){ 
        return; 
    }

    if (anim->frameAtual >= anim->anim->def.numFrames) {
        anim->frameAtual = anim->anim->def.numFrames - 1; // Garante que é o último
    }

    Rectangle frameRec = anim->anim->def.frames[anim->frameAtual];
    Rectangle destRec;
    Vector2 origem;

    destRec.x = anim->pos.x;
    destRec.y = anim->pos.y;
    destRec.width  = frameRec.width * anim->zoom;
    destRec.height = frameRec.height * anim->zoom;

    origem.x = destRec.width / 2;
    origem.y = destRec.height / 2;

    // Lápide flip
    if (anim->flip) { 
        frameRec.width = -frameRec.width; 
    }

    DrawTexturePro(
        anim->anim->textura,
        frameRec,
        destRec,
        origem,
        0.0f,
        WHITE
    );
}


static void DesenharAnimacao(EstadoAnimacao* anim) {
    if (anim->ativo == false) {
         return;
    }
    if (anim->anim == NULL) {
         return;
    }
    if (anim->anim->def.numFrames == 0) {
         return;
    }

    if (anim->frameAtual >= anim->anim->def.numFrames) {
        anim->frameAtual = 0;
    }

    Rectangle frameRec = anim->anim->def.frames[anim->frameAtual];
    Rectangle destRec;
    Vector2 origem;

    // Área destino onde o sprite será desenhado
    destRec.x = anim->pos.x;
    destRec.y = anim->pos.y;
    destRec.width  = frameRec.width * anim->zoom;
    destRec.height = frameRec.height * anim->zoom;

    // Origem: centro do sprite
    origem.x = destRec.width / 2;
    origem.y = destRec.height / 2;

    // --- Corrigido: flip real ---
    if (anim->flip) {
        frameRec.width = -frameRec.width;  // Inverte horizontalmente
    }

    DrawTexturePro(
        anim->anim->textura,
        frameRec,     // parte da textura
        destRec,      // onde desenhar
        origem,       // ponto de origem
        0.0f,         // rotação
        WHITE
    );
}


// -------------------------------------------------------------------
// FUNÇÃO PÚBLICA DE DESENHO (MOVIDA DE 'batalha.c')
// -------------------------------------------------------------------

void DesenharTelaBatalha(EstadoBatalha *estado) {
    DrawText("Jogador 1", 20, 15, 20, RAYWHITE);

    const char* textoRound = TextFormat("Round: %d", estado->roundAtual);
    int larguraTextoRound = MeasureText(textoRound, 20);
    DrawText(textoRound, (SCREEN_WIDTH - larguraTextoRound) / 2, 15, 20, RAYWHITE);

    // Texto do Oponente (IA ou J2)
    DrawText("Oponente", SCREEN_WIDTH - MeasureText("Oponente", 20) - 20, 15, 20, RAYWHITE);

    int arenaY = 80;
    int arenaHeight = 550; 
    DrawRectangleLines(10, arenaY, SCREEN_WIDTH - 20, arenaHeight, LIGHTGRAY);

    // Agora isso funciona, pois 'backgroundArena' e 'backgroundCarregado'
    // são declaradas como 'extern' em 'batalha.h'
    if (backgroundCarregado == 1) {
        DrawTexturePro(
            backgroundArena,
            (Rectangle){0, 0, backgroundArena.width, backgroundArena.height},
            (Rectangle){10, arenaY, SCREEN_WIDTH - 20, arenaHeight},
            (Vector2){0, 0},
            0.0f,
            WHITE
        );
    }

    const float hpBarYFixo = 530.0f;
    
    // Cores para o fade
    Color corFade = ColorAlpha(WHITE, estado->alphaOutrosPersonagens);
    Color corBgFade = ColorAlpha(DARKGRAY, estado->alphaOutrosPersonagens);
    Color corHpFadeJ = ColorAlpha(GREEN, estado->alphaOutrosPersonagens);
    Color corHpFadeIA = ColorAlpha(RED, estado->alphaOutrosPersonagens);
    Color corBordaFade = ColorAlpha(LIGHTGRAY, estado->alphaOutrosPersonagens);

    // --- 1. IDENTIFICAR QUEM ESTÁ EM FOCO (PARA NÃO DESENHAR NO FUNDO) ---
    int idxAtacanteFoco = -1;
    bool atacanteEhIA = false;
    int idxAlvoFoco = -1;
    bool alvoEhIA = false;

    // Posições de desenho (precisamos delas aqui para desenhar)
    Vector2 posJogador[3];
    Vector2 posIA[3];
    float posY = 450.0f;
    float espacamentoX = 200.0f;
    float offsetInicialJogador = 100.0f;
    float offsetInicialIA = SCREEN_WIDTH - 100.0f;
    
    posJogador[2] = (Vector2){offsetInicialJogador, posY};
    posJogador[1] = (Vector2){offsetInicialJogador + espacamentoX, posY};
    posJogador[0] = (Vector2){offsetInicialJogador + espacamentoX * 2, posY};
    
    posIA[0] = (Vector2){offsetInicialIA - espacamentoX * 2, posY};
    posIA[1] = (Vector2){offsetInicialIA - espacamentoX, posY};
    posIA[2] = (Vector2){offsetInicialIA, posY};


    if (estado->atacanteEmFoco != NULL) {
        // Procura atacante no Jogador
        NoPersonagem* no = ObterNoPorPersonagem(&estado->timeJogador, estado->atacanteEmFoco);
        if (no != NULL) {
            idxAtacanteFoco = no->posicaoNoTime;
            atacanteEhIA = false;
        } else {
            // Procura atacante na IA
            no = ObterNoPorPersonagem(&estado->timeIA, estado->atacanteEmFoco);
            if (no != NULL) {
                idxAtacanteFoco = no->posicaoNoTime;
                atacanteEhIA = true;
            }
        }
    }

    if (estado->alvoEmFoco != NULL || estado->alvoEmFocoIdx != -1) {
        if (estado->animFlip) { // Se flip é true, Oponente está atacando -> Alvo é Jogador
             alvoEhIA = false;
        } else { // Jogador ataca -> Alvo é IA
             alvoEhIA = true;
        }
        
        NoPersonagem* no = ObterNoPorPersonagem(alvoEhIA ? &estado->timeIA : &estado->timeJogador, estado->alvoEmFoco);
        if (no != NULL) {
            idxAlvoFoco = no->posicaoNoTime;
        } else {
            idxAlvoFoco = estado->alvoEmFocoIdx;
        }
    }

    // --- 2. DESENHAR O FUNDO (PERSONAGENS QUE NÃO ESTÃO EM FOCO) ---
    // A) Time JOGADOR (Background)
    for (int i = 0; i < 3; i++) {
        bool ehFoco = false;
        if (!atacanteEhIA && idxAtacanteFoco == i) ehFoco = true;
        if (!alvoEhIA && idxAlvoFoco == i) ehFoco = true;


        if (ehFoco == false) {
            // 1. Desenha Lápide se estiver ativa
            if (estado->animLapideJogador[i].ativo) {
                
                estado->animLapideJogador[i].pos = posJogador[i];
                
                if (estado->animLapideJogador[i].anim != NULL) {
                     Rectangle frame = estado->animLapideJogador[i].anim->def.frames[estado->animLapideJogador[i].frameAtual];
                     
                     Rectangle dest = { 
                         posJogador[i].x, 
                         posJogador[i].y, 
                         frame.width * estado->animLapideJogador[i].zoom,
                         frame.height * estado->animLapideJogador[i].zoom
                     };
                     Vector2 orig = { dest.width/2, dest.height/2 };

                    if (estado->animLapideJogador[i].flip) {
                         frame.width = -frame.width;
                     }

                     DrawTexturePro(estado->animLapideJogador[i].anim->textura, frame, dest, orig, 0.0f, corFade);
                }
            }
            // 2. Se não tem lápide, verifica se tem personagem vivo na lista
            else {
                 NoPersonagem* no = ObterNoNaPosicao(&estado->timeJogador, i);
                 if (no != NULL) {
                    PersonagemData* pData = no->personagem;
                    AnimacaoData* anim = &pData->animIdle;
                    
                    Color corPersonagem = corFade; 
                    if (estado->timerDanoJogador[i] > 0.0f && fmod(estado->timerDanoJogador[i], 0.2f) < 0.1f) {
                        corPersonagem = ColorAlpha(RED, estado->alphaOutrosPersonagens);
                    }

                    if (anim->def.numFrames > 0) {
                        int frameIndex = estado->idleFrameJogador[i];
                        if (frameIndex >= anim->def.numFrames) {
                            frameIndex = 0;
                        }
                        
                        Rectangle frame = anim->def.frames[frameIndex];
                        float zoom = pData->batalhaZoom;
                        if (pData->animIdle.flip == true) {
                            frame.width = -frame.width;
                        }

                        DrawTexturePro(anim->textura, frame,
                        (Rectangle){posJogador[i].x, posJogador[i].y, fabsf(frame.width) * zoom, frame.height * zoom},
                        (Vector2){fabsf(frame.width) * zoom / 2, frame.height * zoom / 2}, 0, corPersonagem);

                        // Barra HP
                        int posXBarra = (int)posJogador[i].x - 50; 
                        int posYBarra = (int)hpBarYFixo; 
                        int larguraPreenchimento = (int)(100.0f * (float)estado->hpJogador[i] / (float)pData->hpMax);
                        
                        DrawRectangle(posXBarra, posYBarra, 100, 10, corBgFade);
                        DrawRectangle(posXBarra, posYBarra, larguraPreenchimento, 10, corHpFadeJ);
                        DrawRectangleLines(posXBarra, posYBarra, 100, 10, corBordaFade);
                    }
                 }
            }
        }
    }

    // B) Time IA/OPONENTE (Background)
    for (int i = 0; i < 3; i++) {
        bool ehFoco = false;
        if (atacanteEhIA && idxAtacanteFoco == i) ehFoco = true;
        if (alvoEhIA && idxAlvoFoco == i) ehFoco = true;


        if (ehFoco == false) {
            // 1. Lápide
            if (estado->animLapideIA[i].ativo) {
                
                estado->animLapideIA[i].pos = posIA[i];
                
                if (estado->animLapideIA[i].anim != NULL) {
                     Rectangle frame = estado->animLapideIA[i].anim->def.frames[estado->animLapideIA[i].frameAtual];
                     if (estado->animLapideIA[i].flip) {
                         frame.width = -frame.width;
                     }
                     
                     Rectangle dest = { 
                         posIA[i].x, 
                         posIA[i].y, 
                         fabsf(frame.width) * estado->animLapideIA[i].zoom, // Zoom da struct (1.0f)
                         frame.height * estado->animLapideIA[i].zoom       // Zoom da struct (1.0f)
                     };
                     Vector2 orig = { dest.width/2, dest.height/2 };
                     DrawTexturePro(estado->animLapideIA[i].anim->textura, frame, dest, orig, 0.0f, corFade);
                }
            }
            // 2. Vivo
            else {
                 NoPersonagem* no = ObterNoNaPosicao(&estado->timeIA, i);
                 if (no != NULL) {
                    PersonagemData* pData = no->personagem;
                    AnimacaoData* anim = &pData->animIdle;
                    
                    Color corPersonagem = corFade;
                    if (estado->timerDanoIA[i] > 0.0f && fmod(estado->timerDanoIA[i], 0.2f) < 0.1f) {
                        corPersonagem = ColorAlpha(RED, estado->alphaOutrosPersonagens);
                    }

                    if (anim->def.numFrames > 0) {
                        int frameIndex = estado->idleFrameIA[i];
                        if (frameIndex >= anim->def.numFrames) {
                            frameIndex = 0;
                        }

                        Rectangle frame = anim->def.frames[frameIndex];
                        float zoom = pData->batalhaZoom;

                        if (pData->animIdle.flip == true) {
                        frame.width = -frame.width;
                        }

                        DrawTexturePro(anim->textura, frame,
                        (Rectangle){posIA[i].x, posIA[i].y, fabsf(frame.width) * zoom, frame.height * zoom},
                        (Vector2){fabsf(frame.width) * zoom / 2, frame.height * zoom / 2}, 0, corPersonagem);
                        // Barra HP
                        int posXBarra = (int)posIA[i].x - 50; 
                        int posYBarra = (int)hpBarYFixo; 
                        int larguraPreenchimento = (int)(100.0f * (float)estado->hpIA[i] / (float)pData->hpMax);

                        DrawRectangle(posXBarra, posYBarra, 100, 10, corBgFade);
                        DrawRectangle(posXBarra, posYBarra, larguraPreenchimento, 10, corHpFadeIA);
                        DrawRectangleLines(posXBarra, posYBarra, 100, 10, corBordaFade);
                    }
                 }
            }
        }
    }


    // --- 3. DESENHAR O FOCO (ATACANTE E ALVO) ---
    if (estado->atacanteEmFoco != NULL) {
        
        // A) DESENHA O ATACANTE (que pode ser animado ou idle)
        PersonagemData* pDataAtacante = estado->atacanteEmFoco;
        
        if (estado->estadoTurno == ESTADO_ANIMACAO_ATAQUE) {
            // A animação de ataque é desenhada em separado mais abaixo
        } else {
            // Desenha o IDLE do atacante durante o zoom
            int frameIdx = atacanteEhIA ? estado->idleFrameIA[idxAtacanteFoco] : estado->idleFrameJogador[idxAtacanteFoco];
            if (pDataAtacante->animIdle.def.numFrames > 0 && frameIdx >= pDataAtacante->animIdle.def.numFrames) frameIdx = 0;
            
            Rectangle frame = pDataAtacante->animIdle.def.frames[frameIdx];
            if (estado->animFlip) frame.width = -frame.width;

            DrawTexturePro(pDataAtacante->animIdle.textura, frame,
                (Rectangle){ estado->posFocoAtacante.x, estado->posFocoAtacante.y, fabsf(frame.width) * estado->zoomFocoAtacante, frame.height * estado->zoomFocoAtacante },
                (Vector2){ fabsf(frame.width) * estado->zoomFocoAtacante / 2, frame.height * estado->zoomFocoAtacante / 2 }, 0, WHITE);
        }
        
        // Barra de HP do Atacante
        int hpAtacante = atacanteEhIA ? estado->hpIA[idxAtacanteFoco] : estado->hpJogador[idxAtacanteFoco];
        float alturaFrame = (pDataAtacante->animIdle.def.numFrames > 0) ? pDataAtacante->animIdle.def.frames[0].height : 100.0f;
        float hpBarY = estado->posFocoAtacante.y + (alturaFrame * estado->zoomFocoAtacante / 2) + 5;
        DrawRectangle((int)estado->posFocoAtacante.x - 50, (int)hpBarY, 100, 10, DARKGRAY);
        DrawRectangle((int)estado->posFocoAtacante.x - 50, (int)hpBarY, (int)(100.0f * hpAtacante / pDataAtacante->hpMax), 10, atacanteEhIA ? RED : GREEN);
        DrawRectangleLines((int)estado->posFocoAtacante.x - 50, (int)hpBarY, 100, 10, LIGHTGRAY);
        
        
        // B) DESENHA O ALVO (se houver)
        if (estado->isZoomAoe == true) 
        {
            // --- MODO AOE: Desenha os 3 alvos (1 focado, 2 parados) ---
            ListaTime* listaAlvo = alvoEhIA ? &estado->timeIA : &estado->timeJogador;
            Vector2* posAlvoArray = alvoEhIA ? posIA : posJogador; // Posições originais
            
            for (int i = 0; i < 3; i++) {
                NoPersonagem* noAlvo = ObterNoNaPosicao(listaAlvo, i);
                PersonagemData* pDataAlvo = (noAlvo != NULL) ? noAlvo->personagem : NULL;
                EstadoAnimacao* animLapideAlvo = alvoEhIA ? &estado->animLapideIA[i] : &estado->animLapideJogador[i];
                
                Vector2 posAtualAlvo;
                float zoomAtualAlvo;
                bool flipAlvo = alvoEhIA; // Flip padrão

                if (i == estado->alvoEmFocoIdx) {
                    // Este é o alvo focado (o que foi clicado)
                    posAtualAlvo = estado->posFocoAlvo;
                    zoomAtualAlvo = estado->zoomFocoAlvo;
                    if (estado->alvoEmFoco != NULL) {
                        pDataAlvo = estado->alvoEmFoco; // Garante pData para lápide
                    }
                    if (pDataAlvo != NULL) { 
                        flipAlvo = pDataAlvo->animIdle.flip; // Usa o flip dele (que foi setado no ZoomIn)
                    }

                } else {
                    // Este é um dos outros 2 alvos (não focados)
                    posAtualAlvo = posAlvoArray[i];
                    if (pDataAlvo != NULL) {
                        zoomAtualAlvo = estado->zoomFocoAlvo;
                        flipAlvo = pDataAlvo->animIdle.flip; // Usa o flip padrão dele
                    } else {
                        zoomAtualAlvo = 0.8f; // Zoom da lápide
                        flipAlvo = animLapideAlvo->flip; 
                    }
                }
                
                // --- Desenha Lápide ou Personagem (sempre com ALPHA 1.0) ---
                if (animLapideAlvo->ativo) {
                    animLapideAlvo->pos = posAtualAlvo;
                    animLapideAlvo->zoom = 0.8f; // Zoom padrão da lápide

                    // Se for o alvo focado, calcula o zoom da lápide baseado no zoom interpolado
                    if (i == estado->alvoEmFocoIdx && pDataAlvo != NULL) {
                        animLapideAlvo->zoom = (zoomAtualAlvo / pDataAlvo->batalhaZoom) * 0.8f;
                    }
                    
                    DesenharAnimacaoLapide(animLapideAlvo);
                } 
                else if (pDataAlvo != NULL) {
                     int frameIdx = alvoEhIA ? estado->idleFrameIA[i] : estado->idleFrameJogador[i];
                     if (pDataAlvo->animIdle.def.numFrames > 0 && frameIdx >= pDataAlvo->animIdle.def.numFrames) frameIdx = 0;

                     Color corAlvo = WHITE;
                     if (alvoEhIA) {
                        if (estado->timerDanoIA[i] > 0.0f && fmod(estado->timerDanoIA[i], 0.2f) < 0.1f) corAlvo = RED;
                     } else {
                        if (estado->timerDanoJogador[i] > 0.0f && fmod(estado->timerDanoJogador[i], 0.2f) < 0.1f) corAlvo = RED;
                     }
                    
                     Rectangle frame = pDataAlvo->animIdle.def.frames[frameIdx];
                     if (flipAlvo) frame.width = -frame.width; // Usa o flip calculado

                     DrawTexturePro(pDataAlvo->animIdle.textura, frame,
                        (Rectangle){ posAtualAlvo.x, posAtualAlvo.y, fabsf(frame.width) * zoomAtualAlvo, frame.height * zoomAtualAlvo },
                        (Vector2){ fabsf(frame.width) * zoomAtualAlvo / 2, frame.height * zoomAtualAlvo / 2 }, 0, corAlvo);

                    // Barra de HP
                    int hpAlvo = alvoEhIA ? estado->hpIA[i] : estado->hpJogador[i];
                    float alturaFrameAlvo = (pDataAlvo->animIdle.def.numFrames > 0) ? pDataAlvo->animIdle.def.frames[0].height : 100.0f;
                    float hpBarYAlvo = posAtualAlvo.y + (alturaFrameAlvo * zoomAtualAlvo / 2) + 5;
                    DrawRectangle((int)posAtualAlvo.x - 50, (int)hpBarYAlvo, 100, 10, DARKGRAY);
                    DrawRectangle((int)posAtualAlvo.x - 50, (int)hpBarYAlvo, (int)(100.0f * hpAlvo / pDataAlvo->hpMax), 10, alvoEhIA ? RED : GREEN);
                    DrawRectangleLines((int)posAtualAlvo.x - 50, (int)hpBarYAlvo, 100, 10, LIGHTGRAY);
                }
            }
        }
        else if (estado->alvoEmFoco != NULL) 
        {
            // --- MODO ALVO ÚNICO (Lógica original) ---
            PersonagemData* pDataAlvo = estado->alvoEmFoco;
            EstadoAnimacao* animLapideAlvo = alvoEhIA ? &estado->animLapideIA[idxAlvoFoco] : &estado->animLapideJogador[idxAlvoFoco];

            if (animLapideAlvo->ativo) {
                animLapideAlvo->pos = estado->posFocoAlvo;
                DesenharAnimacaoLapide(animLapideAlvo);
            } else {
                int frameIdx = alvoEhIA ? estado->idleFrameIA[idxAlvoFoco] : estado->idleFrameJogador[idxAlvoFoco];
                if (pDataAlvo->animIdle.def.numFrames > 0 && frameIdx >= pDataAlvo->animIdle.def.numFrames) frameIdx = 0;

                Color corAlvo = WHITE;
                if (alvoEhIA) {
                    if (estado->timerDanoIA[idxAlvoFoco] > 0.0f && fmod(estado->timerDanoIA[idxAlvoFoco], 0.2f) < 0.1f) corAlvo = RED;
                } else {
                    if (estado->timerDanoJogador[idxAlvoFoco] > 0.0f && fmod(estado->timerDanoJogador[idxAlvoFoco], 0.2f) < 0.1f) corAlvo = RED;
                }
                
                Rectangle frame = pDataAlvo->animIdle.def.frames[frameIdx];
                
                if (pDataAlvo->animIdle.flip == true) {
                    frame.width = -frame.width;
                }

                DrawTexturePro(pDataAlvo->animIdle.textura, frame,
                    (Rectangle){ estado->posFocoAlvo.x, estado->posFocoAlvo.y, fabsf(frame.width) * estado->zoomFocoAlvo, frame.height * estado->zoomFocoAlvo },
                    (Vector2){ fabsf(frame.width) * estado->zoomFocoAlvo / 2, frame.height * estado->zoomFocoAlvo / 2 }, 0, corAlvo);
            }

            // Barra de HP do Alvo
            int hpAlvo = alvoEhIA ? estado->hpIA[idxAlvoFoco] : estado->hpJogador[idxAlvoFoco];
            float alturaFrameAlvo = (pDataAlvo->animIdle.def.numFrames > 0) ? pDataAlvo->animIdle.def.frames[0].height : 100.0f;
            float hpBarYAlvo = estado->posFocoAlvo.y + (alturaFrameAlvo * estado->zoomFocoAlvo / 2) + 5;
            DrawRectangle((int)estado->posFocoAlvo.x - 50, (int)hpBarYAlvo, 100, 10, DARKGRAY);
            DrawRectangle((int)estado->posFocoAlvo.x - 50, (int)hpBarYAlvo, (int)(100.0f * hpAlvo / pDataAlvo->hpMax), 10, alvoEhIA ? RED : GREEN);
            DrawRectangleLines((int)estado->posFocoAlvo.x - 50, (int)hpBarYAlvo, 100, 10, LIGHTGRAY);
        }
    }
    
    // Desenha a animação de ataque por cima de tudo (explosão, soco, etc)
    DesenharAnimacao(&estado->animacaoEmExecucao);
    
    // --- 4. DESENHAR TEXTOS FLUTUANTES ---
    // (A variável g_textosFlutuantes é static de 'batalha.c', então 
    // precisamos movê-la para cá ou expô-la também)
    // *** CORREÇÃO: Vamos mover g_textosFlutuantes para cá ***
    // (Vou assumir que você moveu as definições de 
    // TextoFlutuante, MAX_TEXTOS_FLUTUANTES, g_textosFlutuantes, 
    // e DURACAO_TEXTO_FLUTUANTE para este arquivo 'batalha_desenho.c' 
    // e as removeu de 'batalha.c')
    
    /* NOTA: Na verdade, é melhor manter g_textosFlutuantes em 'batalha.c'
    junto com a lógica 'IniciarTextoFlutuante' e 'AtualizarTelaBatalha'.
    Para desenhá-los aqui, teríamos que:
    1. Mover a definição de g_textosFlutuantes para 'batalha_desenho.c'
    2. Expor 'IniciarTextoFlutuante' em 'batalha_desenho.h' para 'batalha.c' usar.
    3. Mover a lógica de update dos timers de g_textosFlutuantes para 'AtualizarTelaBatalha'.

    É mais simples deixar como estava. Vou assumir que você **NÃO** moveu
    as variáveis 'g_textosFlutuantes' e elas continuam 'static' em 'batalha.c'.
    Isso significa que a lógica de desenho delas TAMBÉM deve ficar em 'batalha.c'.

    VOU REVERTER A DECISÃO: O Desenho do texto flutuante DEVE ficar aqui.
    Vamos expor a 'g_textosFlutuantes' da mesma forma que 'backgroundArena'.
    */

    /*
    RE-REVERSÃO: É muito complexo expor um array de structs.
    A forma mais simples é criar uma função `DesenharTextosFlutuantes()`
    em `batalha.c`, e chamá-la a partir de `DesenharTelaBatalha`.
    
    ...mas isso quebra a separação.
    
    SOLUÇÃO FINAL (A melhor):
    1. Mova a definição do array g_textosFlutuantes para `batalha.c` (onde já está).
    2. Mova a lógica de ATUALIZAÇÃO do timer (o loop `for (int i = 0; i < MAX_TEXTOS_FLUTUANTES...` 
       de `AtualizarTelaBatalha`) para `batalha_desenho.c`.
    3. Mova a lógica de DESENHO (o loop `for (int i = 0;... DrawText(...)`) 
       para `batalha_desenho.c`.
    4. Exponha `g_textosFlutuantes` com `extern` em `batalha.h`.
    */

    // --- (ASSUMINDO QUE 'batalha.h' AGORA TEM: extern TextoFlutuante g_textosFlutuantes[MAX_TEXTOS_FLUTUANTES];) ---
    // --- (E 'batalha.c' tem: TextoFlutuante g_textosFlutuantes[MAX_TEXTOS_FLUTUANTES];) ---
    // --- (E 'batalha.c' em 'AtualizarTelaBatalha' removeu o loop de update dos timers de texto) ---

    // 4. ATUALIZAR E DESENHAR TEXTOS FLUTUANTES
    float dt = GetFrameTime();
    for (int i = 0; i < MAX_TEXTOS_FLUTUANTES; i++) {
        if (g_textosFlutuantes[i].ativo) {
            
            // Lógica de Update (movida de AtualizarTelaBatalha)
            g_textosFlutuantes[i].timer += dt;
            if (g_textosFlutuantes[i].timer >= g_textosFlutuantes[i].duracao) {
                g_textosFlutuantes[i].ativo = false; // Desativa
            } else {
                g_textosFlutuantes[i].pos.y += g_textosFlutuantes[i].velocidadeY * dt;
            }

            // Lógica de Desenho
            if (g_textosFlutuantes[i].ativo) { // Verifica de novo caso tenha acabado de ser desativado
                float progresso = g_textosFlutuantes[i].timer / g_textosFlutuantes[i].duracao;
                float alpha = 1.0f;
                
                float inicioFade = 0.6f;
                if (progresso > inicioFade) {
                    alpha = (1.0f - progresso) / (1.0f - inicioFade);
                }

                Color corComAlpha = ColorAlpha(g_textosFlutuantes[i].cor, alpha);
                
                DrawText(g_textosFlutuantes[i].texto, (int)g_textosFlutuantes[i].pos.x + 1, (int)g_textosFlutuantes[i].pos.y + 1, 30, ColorAlpha(BLACK, alpha));
                DrawText(g_textosFlutuantes[i].texto, (int)g_textosFlutuantes[i].pos.x, (int)g_textosFlutuantes[i].pos.y, 30, corComAlpha);
            }
        }
    }


    // --- 5. DESENHAR INDICADORES DE ALVO ---
    if (estado->estadoTurno == ESTADO_ESPERANDO_JOGADOR && estado->ataqueSelecionado != -1) {
        bool ehJogador1 = (estado->turnoDe == TURNO_JOGADOR);
        if (ehJogador1) {
            NoPersonagem* noAtual = estado->timeIA.inicio;
            while (noAtual != NULL) {
                int i = noAtual->posicaoNoTime;
                DrawRectangleLines((int)posIA[i].x - 50, (int)posIA[i].y - 50, 100, 100, YELLOW);
                noAtual = noAtual->proximo;
            }
        } else {
            NoPersonagem* noAtual = estado->timeJogador.inicio;
            while (noAtual != NULL) {
                int i = noAtual->posicaoNoTime;
                DrawRectangleLines((int)posJogador[i].x - 50, (int)posJogador[i].y - 50, 100, 100, YELLOW);
                noAtual = noAtual->proximo;
            }
        }
    }

    // --- 6. DESENHAR ESTADO DE FIM DE JOGO ---
    if (estado->estadoTurno == ESTADO_FIM_DE_JOGO)
    {
        const char* textoResultado = "";
        Color corResultado = BLACK;
        int fontSize = 80;

        if (estado->resultadoBatalha == RESULTADO_VITORIA) {
            textoResultado = "VITÓRIA";
            corResultado = (Color){100, 255, 100, 255}; 
        } else if (estado->resultadoBatalha == RESULTADO_DERROTA) {
            textoResultado = "DERROTA";
            corResultado = (Color){255, 100, 100, 255}; 
        }

        if (strlen(textoResultado) > 0)
        {
            int larguraTexto = MeasureText(textoResultado, fontSize);
            int posX = (SCREEN_WIDTH - larguraTexto) / 2;
            int posY = arenaY + (arenaHeight / 2) - (fontSize / 2); //

            DrawText(textoResultado, posX + 4, posY + 4, fontSize, ColorAlpha(BLACK, 0.7f));
            DrawText(textoResultado, posX, posY, fontSize, corResultado);
        }
    }

    // --- 7. DESENHAR O MENU INFERIOR ---
    int menuY = arenaY + arenaHeight + 10; 
    int menuHeight = 240; 
    Color menuBG = (Color){ 40, 40, 40, 255 };
    DrawRectangle(10, menuY, SCREEN_WIDTH - 20, menuHeight, menuBG);
    DrawRectangleLines(10, menuY, SCREEN_WIDTH - 20, menuHeight, RAYWHITE);
    
    int colAtaquesX = 35;
    int colLogX = 300;
    int colSpecsX = 850;
    int textoYBase = menuY + 20; 

    Color corBotaoNormal = LIGHTGRAY;
    Color corBotaoSelecionado = YELLOW;
    Color corTexto = BLACK;
    float espessuraBorda = 2.0f;
    
    Rectangle btnAtk1 = { colAtaquesX, textoYBase + 35, 200, 40 };
    Rectangle btnAtk2 = { colAtaquesX, textoYBase + 90, 200, 40 };

    if (estado->estadoTurno == ESTADO_ESPERANDO_JOGADOR) {
        DrawText("Ataque:", colAtaquesX, textoYBase, 20, GREEN);
        DrawText("Especificações:", colSpecsX, textoYBase, 20, GREEN);
        DrawText(estado->mensagemBatalha, colLogX, textoYBase, 20, WHITE); 

        PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
        if (atacante != NULL) {
            Color corAtk1 = (estado->ataqueSelecionado == 0) ? corBotaoSelecionado : corBotaoNormal;
            Color corAtk2 = (estado->ataqueSelecionado == 1) ? corBotaoSelecionado : corBotaoNormal;

            DrawRectangleRounded(btnAtk1, 0.2f, 4, corAtk1);
            DrawRectangleRoundedLinesEx(btnAtk1, 0.2f, 4, espessuraBorda, BLACK);
            DrawText(atacante->ataque1.nome, (int)btnAtk1.x + ((int)btnAtk1.width - MeasureText(atacante->ataque1.nome, 20)) / 2, (int)btnAtk1.y + 10, 20, corTexto);

            DrawRectangleRounded(btnAtk2, 0.2f, 4, corAtk2);
            DrawRectangleRoundedLinesEx(btnAtk2, 0.2f, 4, espessuraBorda, BLACK);
            DrawText(atacante->ataque2.nome, (int)btnAtk2.x + ((int)btnAtk2.width - MeasureText(atacante->ataque2.nome, 20)) / 2, (int)btnAtk2.y + 10, 20, corTexto);
        
            Vector2 mousePos = GetMouseVirtual();
            Ataque* attHover = NULL;
            if (CheckCollisionPointRec(mousePos, btnAtk1)) attHover = &atacante->ataque1;
            else if (CheckCollisionPointRec(mousePos, btnAtk2)) attHover = &atacante->ataque2;

            if (attHover != NULL) {
                DrawText(attHover->descricao, colSpecsX, textoYBase + 40, 20, RAYWHITE);
                const char* tipoTxt = "Causa %d de Dano.";
                if (attHover->tipo == TIPO_CURA_SI) tipoTxt = "Cura %d de PV.";
                else if (attHover->tipo == TIPO_DANO_AREA) tipoTxt = "Causa %d de Dano em Área.";
                DrawText(TextFormat(tipoTxt, attHover->dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
            }
        }
        
    } else {
        int larguraLog = MeasureText(estado->mensagemBatalha, 20);
        int logX = (SCREEN_WIDTH - larguraLog) / 2;
        if (logX < 15) logX = 15;
        DrawText(estado->mensagemBatalha, logX, textoYBase + 60, 20, WHITE);
    }
}