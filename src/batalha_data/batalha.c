#include "batalha.h"
#include "database.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "ConsumoAPI_Gemini.h" 

// Inclui a definição de 'GetMouseVirtual()'
#include "telas.h"

static void ExecutarAtaque(EstadoBatalha* estado, PersonagemData* atacante, Ataque* ataque, int alvoIdx, bool ehJogadorAtacando);
static void ExecutarAtaque(EstadoBatalha* estado, PersonagemData* atacante, Ataque* ataque, int alvoIdx, bool ehJogadorAtacando);

static char CHAVE_API_BUFFER[256];
static bool chaveJaFoiCarregada = false; // Impede leituras repetidas

static const char* CarregarChaveApi(const char* caminhoArquivo) {
    // Só lê o arquivo uma vez
    if (chaveJaFoiCarregada == true) {
        return CHAVE_API_BUFFER;
    }

    FILE *arquivo = fopen(caminhoArquivo, "r");
    if (arquivo == NULL) {
        printf("ERRO FATAL: Nao foi possivel abrir: %s\n", caminhoArquivo);
        printf("Criar um arquivo 'config.txt' na raiz do projeto com a chave.\n");
        strcpy(CHAVE_API_BUFFER, "CHAVE_NAO_ENCONTRADA");
        chaveJaFoiCarregada = true;
        return CHAVE_API_BUFFER;
    }

    // Lê a primeira linha do arquivo
    if (fgets(CHAVE_API_BUFFER, sizeof(CHAVE_API_BUFFER), arquivo) != NULL) {
        CHAVE_API_BUFFER[strcspn(CHAVE_API_BUFFER, "\r\n")] = 0;
        chaveJaFoiCarregada = true;
    } else {
         printf("ERRO: Nao foi possivel ler a chave do arquivo 'config.txt'.\n");
         strcpy(CHAVE_API_BUFFER, "CHAVE_NAO_ENCONTRADA");
    }

    fclose(arquivo);
    return CHAVE_API_BUFFER;
}

//IA do GEMINI
static void ExecutarTurnoIA(EstadoBatalha *estado) {
    PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];

    const char* MINHA_CHAVE_API = CarregarChaveApi("config.txt");

    if (strcmp(MINHA_CHAVE_API, "CHAVE_NAO_ENCONTRADA") == 0) {
        printf("IA: ERRO! Chave da API nao encontrada. Pulando turno.\n");
        int alvoAleatorio = rand() % 3;
        int tentativas = 0;
        while (estado->hpJogador[alvoAleatorio] <= 0 && tentativas < 3) {
             alvoAleatorio = (alvoAleatorio + 1) % 3;
             tentativas++;
        }
        
        ExecutarAtaque(estado, atacante, &atacante->ataque1, alvoAleatorio, false);
        return;
    }

    // Obtém a decisão da API
    DecisaoIA decisao = ObterDecisaoIA(estado, MINHA_CHAVE_API);

    Ataque* ataqueEscolhido;
    if (decisao.indiceAtaque == 0) {
        ataqueEscolhido = &atacante->ataque1;
    } else {
        ataqueEscolhido = &atacante->ataque2;
    }

    ExecutarAtaque(estado, atacante, ataqueEscolhido, decisao.indiceAlvo, false);

    LiberarDecisaoIA(&decisao);
    
}

static Vector2 posJogador[3];
static Vector2 posIA[3];
static int animVelocidadeIdle = 15; // Mais lento, bom para idle

static void PararAnimacao(EstadoAnimacao* anim) {
    anim->ativo = false;
    anim->frameAtual = 0;
    anim->timer = 0;
}

static void IniciarAnimacao(EstadoAnimacao* anim, AnimacaoData* data, Vector2 pos, float zoom, bool flip) {
    anim->anim = data;
    anim->pos = pos;
    anim->zoom = zoom;
    anim->ativo = true;
    anim->frameAtual = 0;
    anim->timer = 0;
    anim->velocidade = 8;
    anim->flip = flip;
}

static void AtualizarAnimacao(EstadoAnimacao* anim) {
    if (!anim->ativo) return;
    
    anim->timer++;
    if (anim->timer >= anim->velocidade) {
        anim->timer = 0;
        anim->frameAtual++;
        // Se a animação terminou
        if (anim->frameAtual >= anim->anim->def.numFrames) {
            PararAnimacao(anim);
        }
    }
}

static void DesenharAnimacao(EstadoAnimacao* anim) {
    if (!anim->ativo) return;
    
    // Guarda de segurança
    if (anim->anim == NULL || anim->anim->def.numFrames == 0) return;
    if (anim->frameAtual >= anim->anim->def.numFrames) anim->frameAtual = 0;
    
    Rectangle frameRec = anim->anim->def.frames[anim->frameAtual];
    
    if (anim->flip) {
        frameRec.width = -frameRec.width; // Inverte o frame
    }

    DrawTexturePro(anim->anim->textura,
                   frameRec,
                   (Rectangle){ anim->pos.x, anim->pos.y, frameRec.width * anim->zoom, frameRec.height * anim->zoom },
                   (Vector2){ frameRec.width * anim->zoom / 2, frameRec.height * anim->zoom / 2 },
                   0.0f,
                   WHITE);
}


// Função de comparação para qsort (ordena por velocidade, decrescente)
static int CompararVelocidade(const void* a, const void* b) {
    PersonagemData* pA = *(PersonagemData**)a;
    PersonagemData* pB = *(PersonagemData**)b;
    
    if (pA == NULL || pB == NULL) return 0;
    
    if (pA->velocidade > pB->velocidade) return -1;
    if (pA->velocidade < pB->velocidade) return 1;
    return (rand() % 2 == 0) ? 1 : -1;
}

// Configura o turno para o próximo personagem na fila
static void ProximoTurno(EstadoBatalha *estado) {
    estado->personagemAgindoIdx++;
    // Se todos os 6 atacaram, o round acaba
    if (estado->personagemAgindoIdx >= 6) {
        estado->personagemAgindoIdx = 0;
        printf("--- NOVO ROUND ---\n");
    }
    
    // Guarda de segurança
    if (estado->ordemDeAtaque[estado->personagemAgindoIdx] == NULL) {
        printf("ERRO: Personagem na ordem de ataque e nulo!\n");
        return;
    }
    
    PersonagemData* personagemAtual = estado->ordemDeAtaque[estado->personagemAgindoIdx];
    
    // Verifica se o personagem está vivo
    bool estaVivo = false;
    for (int i = 0; i < 3; i++) {
        if (estado->times.timeJogador[i] == personagemAtual && estado->hpJogador[i] > 0) estaVivo = true;
        if (estado->times.timeIA[i] == personagemAtual && estado->hpIA[i] > 0) estaVivo = true;
    }
    
    if (!estaVivo) {
        // Se o personagem estiver morto, pula o turno dele
        ProximoTurno(estado);
        return;
    }

    // Verifica de quem é o turno
    bool ehJogador = false;
    for (int i = 0; i < 3; i++) {
        if (estado->times.timeJogador[i] == personagemAtual) {
            ehJogador = true;
            break;
        }
    }

    if (ehJogador) {
        estado->turnoDe = TURNO_JOGADOR;
        estado->estadoTurno = ESTADO_ESPERANDO_JOGADOR;
        estado->ataqueSelecionado = -1; // Reseta seleção
        estado->alvoSelecionado = -1;
        sprintf(estado->mensagemBatalha, "Vez de: %s! Escolha um ataque.", personagemAtual->nome);
    } else {
        estado->turnoDe = TURNO_IA;
        estado->estadoTurno = ESTADO_TURNO_IA;
        sprintf(estado->mensagemBatalha, "Vez de: %s!", personagemAtual->nome);
    }
}

// Lógica de ataque (aplica dano)
static void ExecutarAtaque(EstadoBatalha* estado, PersonagemData* atacante, Ataque* ataque, int alvoIdx, bool ehJogadorAtacando) {
    int dano = ataque->dano;
    PersonagemData* alvo;
    
    Vector2 animPos;
    if (ehJogadorAtacando) {
        for (int i = 0; i < 3; i++) {
            if (estado->times.timeJogador[i] == atacante) {
                animPos = posJogador[i];
                break;
            }
        }
        alvo = estado->times.timeIA[alvoIdx];
        estado->hpIA[alvoIdx] -= dano;
        sprintf(estado->mensagemBatalha, "%s usou %s em %s e causou %d de dano!", atacante->nome, ataque->nome, alvo->nome, dano);
        if (estado->hpIA[alvoIdx] <= 0) {
            estado->hpIA[alvoIdx] = 0;
            // (Adicionar lógica de morte aqui)
        }
    } else {
        for (int i = 0; i < 3; i++) {
            if (estado->times.timeIA[i] == atacante) {
                animPos = posIA[i];
                break;
            }
        }
        alvo = estado->times.timeJogador[alvoIdx];
        estado->hpJogador[alvoIdx] -= dano;
        sprintf(estado->mensagemBatalha, "%s usou %s em %s e causou %d de dano!", atacante->nome, ataque->nome, alvo->nome, dano);
        if (estado->hpJogador[alvoIdx] <= 0) {
            estado->hpJogador[alvoIdx] = 0;
            // (Adicionar lógica de morte aqui)
        }
    }


    bool deveFlipar = !ehJogadorAtacando;
    
    // Inicia a animação de ataque usando a 'animPos' correta
    if (strcmp(ataque->nome, atacante->ataque1.nome) == 0) {
        IniciarAnimacao(&estado->animacaoEmExecucao, &atacante->animAtaque1, animPos, atacante->batalhaZoom, deveFlipar);
    } else {
        IniciarAnimacao(&estado->animacaoEmExecucao, &atacante->animAtaque2, animPos, atacante->batalhaZoom, deveFlipar);
    }

    estado->estadoTurno = ESTADO_ANIMACAO_ATAQUE;
}


void InicializarBatalha(EstadoBatalha *estado, TimesBatalha* timesSelecionados) {
    printf("INICIALIZANDO BATALHA!\n");
    
    // 1. Copia os times
    estado->times = *timesSelecionados;
    
    // 2. Define o HP inicial
    for (int i = 0; i < 3; i++) {
        if (estado->times.timeJogador[i] == NULL || estado->times.timeIA[i] == NULL) {
            printf("ERRO FATAL: Time nao selecionado corretamente!\n");
            return;
        }
        estado->hpJogador[i] = estado->times.timeJogador[i]->hpMax;
        estado->hpIA[i] = estado->times.timeIA[i]->hpMax;
    }
    
    float posY = 350.0f;
    
    float espacamentoX = 200.0f;
    
    float offsetInicialJogador = 100.0f;
    float offsetInicialIA = SCREEN_WIDTH - 100.0f;
    
    posJogador[2] = (Vector2){offsetInicialJogador, posY};
    posJogador[1] = (Vector2){offsetInicialJogador + espacamentoX, posY};
    posJogador[0] = (Vector2){offsetInicialJogador + espacamentoX * 2, posY};
    
    posIA[0] = (Vector2){offsetInicialIA - espacamentoX * 2, posY};
    posIA[1] = (Vector2){offsetInicialIA - espacamentoX, posY};
    posIA[2] = (Vector2){offsetInicialIA, posY};

    
    // 4. Cria e ordena a fila de ataque
    for (int i = 0; i < 3; i++) {
        estado->ordemDeAtaque[i] = estado->times.timeJogador[i];
        estado->ordemDeAtaque[i+3] = estado->times.timeIA[i];
    }
    qsort(estado->ordemDeAtaque, 6, sizeof(PersonagemData*), CompararVelocidade);

    printf("Ordem de ataque:\n");
    for(int i=0; i<6; i++) {
        if (estado->ordemDeAtaque[i] != NULL) { 
            printf("  %d. %s (Vel: %d)\n", i+1, estado->ordemDeAtaque[i]->nome, estado->ordemDeAtaque[i]->velocidade);
        }
    }
    
    estado->personagemAgindoIdx = -1;
    estado->estadoTurno = ESTADO_INICIANDO;
    PararAnimacao(&estado->animacaoEmExecucao);
    
    
    for (int i=0; i<3; i++) {
        estado->idleFrameJogador[i] = 0;
        estado->idleTimerJogador[i] = 0;
        estado->idleFrameIA[i] = 0;
        estado->idleTimerIA[i] = 0;
    }
}

void AtualizarTelaBatalha(EstadoBatalha *estado, GameScreen *telaAtual) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        *telaAtual = SCREEN_MENU;
        return;
    }
    
    for (int i = 0; i < 3; i++) {
        // Atualiza Jogador
        if (estado->times.timeJogador[i] != NULL && estado->times.timeJogador[i]->animIdle.def.numFrames > 0) {
            estado->idleTimerJogador[i]++;
            if (estado->idleTimerJogador[i] > animVelocidadeIdle) {
                estado->idleTimerJogador[i] = 0;
                estado->idleFrameJogador[i]++;
                if (estado->idleFrameJogador[i] >= estado->times.timeJogador[i]->animIdle.def.numFrames) {
                    estado->idleFrameJogador[i] = 0;
                }
            }
        }
        // Atualiza IA
        if (estado->times.timeIA[i] != NULL && estado->times.timeIA[i]->animIdle.def.numFrames > 0) {
            estado->idleTimerIA[i]++;
            if (estado->idleTimerIA[i] > animVelocidadeIdle) {
                estado->idleTimerIA[i] = 0;
                estado->idleFrameIA[i]++;
                if (estado->idleFrameIA[i] >= estado->times.timeIA[i]->animIdle.def.numFrames) {
                    estado->idleFrameIA[i] = 0;
                }
            }
        }
    }
    if (estado->estadoTurno != ESTADO_INICIANDO && (estado->personagemAgindoIdx < 0 || estado->personagemAgindoIdx >= 6 || estado->ordemDeAtaque[estado->personagemAgindoIdx] == NULL)) {
        if(estado->estadoTurno != ESTADO_FIM_DE_JOGO) ProximoTurno(estado);
        return;
    }

    AtualizarAnimacao(&estado->animacaoEmExecucao);

    switch (estado->estadoTurno) {
        
        case ESTADO_INICIANDO:
            ProximoTurno(estado);
            break;

        case ESTADO_ESPERANDO_JOGADOR:
            {
                PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
                int menuY = 80 + 450 + 10;
                int textoYBase = menuY + 20;
                int colAtaquesX = 35;

                Rectangle btnAtk1 = { colAtaquesX, textoYBase + 35, 200, 40 };
                Rectangle btnAtk2 = { colAtaquesX, textoYBase + 90, 200, 40 };
                
                Rectangle alvoIA_0 = {posIA[0].x - 50, posIA[0].y - 50, 100, 100};
                Rectangle alvoIA_1 = {posIA[1].x - 50, posIA[1].y - 50, 100, 100};
                Rectangle alvoIA_2 = {posIA[2].x - 50, posIA[2].y - 50, 100, 100};
                

                Vector2 mousePos = GetMouseVirtual();
                // -------------------

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousePos, btnAtk1)) estado->ataqueSelecionado = 0;
                    if (CheckCollisionPointRec(mousePos, btnAtk2)) estado->ataqueSelecionado = 1;
                }
                
                if (estado->ataqueSelecionado != -1) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        int alvo = -1;
                        if (CheckCollisionPointRec(mousePos, alvoIA_0) && estado->hpIA[0] > 0) alvo = 0;
                        if (CheckCollisionPointRec(mousePos, alvoIA_1) && estado->hpIA[1] > 0) alvo = 1;
                        if (CheckCollisionPointRec(mousePos, alvoIA_2) && estado->hpIA[2] > 0) alvo = 2;
                        
                        if (alvo != -1) {
                            estado->alvoSelecionado = alvo;
                            Ataque* att = (estado->ataqueSelecionado == 0) ? &atacante->ataque1 : &atacante->ataque2;
                            ExecutarAtaque(estado, atacante, att, estado->alvoSelecionado, true);
                        }
                    }
                }
            }
            break;
            
        case ESTADO_ANIMACAO_ATAQUE:
            if (!estado->animacaoEmExecucao.ativo) {
                ProximoTurno(estado);
            }
            break;

        case ESTADO_TURNO_IA:
            ExecutarTurnoIA(estado);
            break;
            
        case ESTADO_FIM_DE_JOGO:
            break;
    }
}

void DesenharTelaBatalha(EstadoBatalha *estado) {
    DrawText("Jogador 1", 20, 15, 20, RAYWHITE);
    DrawText("Round", (SCREEN_WIDTH / 2) - 40, 15, 20, RAYWHITE);
    DrawText("IA", SCREEN_WIDTH - 20 - 200 - 35, 15, 20, RAYWHITE);

    int arenaY = 80;
    int arenaHeight = 450;
    DrawRectangleLines(10, arenaY, SCREEN_WIDTH - 20, arenaHeight, LIGHTGRAY);
    
    for (int i = 0; i < 3; i++) {
        PersonagemData* pData = estado->times.timeJogador[i];
        if (pData != NULL && estado->hpJogador[i] > 0) {
            AnimacaoData* anim = &pData->animIdle;
            if (anim->def.numFrames > 0) {
                int frameIndex = estado->idleFrameJogador[i];
                if (frameIndex >= anim->def.numFrames) frameIndex = 0;
                Rectangle frame = anim->def.frames[frameIndex];
                
                float zoom = pData->batalhaZoom;

                DrawTexturePro(anim->textura, frame,
                    (Rectangle){posJogador[i].x, posJogador[i].y, frame.width * zoom, frame.height * zoom},
                    (Vector2){frame.width * zoom / 2, frame.height * zoom / 2}, 0, WHITE);
                
                float hpBarY = posJogador[i].y + (frame.height * zoom / 2) + 5;
                
                DrawRectangle(posJogador[i].x - 50, (int)hpBarY, 100, 10, DARKGRAY);
                DrawRectangle(posJogador[i].x - 50, (int)hpBarY, (int)(100.0f * estado->hpJogador[i] / pData->hpMax), 10, GREEN);
            }
        }
        
        PersonagemData* pDataIA = estado->times.timeIA[i];
        if (pDataIA != NULL && estado->hpIA[i] > 0) {
            AnimacaoData* anim = &pDataIA->animIdle;
            if (anim->def.numFrames > 0) {

                int frameIndexIA = estado->idleFrameIA[i];
                if (frameIndexIA >= anim->def.numFrames) frameIndexIA = 0;
                Rectangle frame = anim->def.frames[frameIndexIA];

                float zoomIA = pDataIA->batalhaZoom;

                Rectangle frameIA = frame;
                frameIA.width = -frame.width;

                DrawTexturePro(anim->textura, frameIA,
                    (Rectangle){posIA[i].x, posIA[i].y, frame.width * zoomIA, frame.height * zoomIA},
                    (Vector2){frame.width * zoomIA / 2, frame.height * zoomIA / 2}, 0, WHITE);
                
                float hpBarY = posIA[i].y + (frame.height * zoomIA / 2) + 5;

                DrawRectangle(posIA[i].x - 50, (int)hpBarY, 100, 10, DARKGRAY);
                DrawRectangle(posIA[i].x - 50, (int)hpBarY, (int)(100.0f * estado->hpIA[i] / pDataIA->hpMax), 10, RED);
            }
        }
    }
    
    DesenharAnimacao(&estado->animacaoEmExecucao);
    
    if (estado->estadoTurno == ESTADO_ESPERANDO_JOGADOR && estado->ataqueSelecionado != -1) {
        for (int i=0; i<3; i++) {
            if (estado->hpIA[i] > 0) {
                DrawRectangleLines(posIA[i].x - 50, posIA[i].y - 50, 100, 100, YELLOW);
            }
        }
    }

    int menuY = arenaY + arenaHeight + 10;
    int menuHeight = SCREEN_HEIGHT - menuY - 10;
    Color menuBG = (Color){ 40, 40, 40, 255 };
    DrawRectangle(10, menuY, SCREEN_WIDTH - 20, menuHeight, menuBG);
    DrawRectangleLines(10, menuY, SCREEN_WIDTH - 20, menuHeight, RAYWHITE);
    
    int colAtaquesX = 35;
    int colSpecsX = (SCREEN_WIDTH / 2) - 100;
    int textoYBase = menuY + 20;

    DrawText("Ataque:", colAtaquesX, textoYBase, 20, GREEN);
    DrawText("Especificações:", colSpecsX, textoYBase, 20, GREEN);

    DrawText(estado->mensagemBatalha, colAtaquesX + 250, textoYBase, 20, WHITE);

    Color corBotaoNormal = LIGHTGRAY;
    Color corBotaoSelecionado = YELLOW;
    Color corTexto = BLACK;
    float espessuraBorda = 2.0f;
    
    Rectangle btnAtk1 = { colAtaquesX, textoYBase + 35, 200, 40 };
    Rectangle btnAtk2 = { colAtaquesX, textoYBase + 90, 200, 40 };
    
    Color corAtk1 = (estado->ataqueSelecionado == 0) ? corBotaoSelecionado : corBotaoNormal;
    Color corAtk2 = (estado->ataqueSelecionado == 1) ? corBotaoSelecionado : corBotaoNormal;

    if (estado->turnoDe == TURNO_JOGADOR && estado->personagemAgindoIdx >= 0)  {
        PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
        
        if (atacante != NULL) {
            DrawRectangleRounded(btnAtk1, 0.2f, 4, corAtk1);
            DrawRectangleRoundedLinesEx(btnAtk1, 0.2f, 4, espessuraBorda, BLACK);
            DrawText(atacante->ataque1.nome, btnAtk1.x + (btnAtk1.width - MeasureText(atacante->ataque1.nome, 20)) / 2, btnAtk1.y + 10, 20, corTexto);

            DrawRectangleRounded(btnAtk2, 0.2f, 4, corAtk2);
            DrawRectangleRoundedLinesEx(btnAtk2, 0.2f, 4, espessuraBorda, BLACK);
            DrawText(atacante->ataque2.nome, btnAtk2.x + (btnAtk2.width - MeasureText(atacante->ataque2.nome, 20)) / 2, btnAtk2.y + 10, 20, corTexto);
        

            Vector2 mousePos = GetMouseVirtual();
            // -------------------
            
            if (CheckCollisionPointRec(mousePos, btnAtk1)) {
                DrawText(atacante->ataque1.descricao, colSpecsX, textoYBase + 40, 20, RAYWHITE);
                DrawText(TextFormat("Causa %d de Dano.", atacante->ataque1.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
            } else if (CheckCollisionPointRec(mousePos, btnAtk2)) {
                DrawText(atacante->ataque2.descricao, colSpecsX, textoYBase + 40, 20, RAYWHITE);
                DrawText(TextFormat("Causa %d de Dano.", atacante->ataque2.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
            }
        }
    }
}