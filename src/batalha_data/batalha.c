#include "batalha.h"
#include "database.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "ConsumoAPI_Gemini.h" 

// Inclui a definição de 'GetMouseVirtual()'
#include "telas.h"

// NÃO precisa incluir "lista_personagem.h" aqui,
// pois "batalha.h" já o inclui.

// Declarações (protótipos) das funções estáticas
static void ExecutarAtaque(EstadoBatalha* estado, PersonagemData* atacante, Ataque* ataque, int alvoIdx, bool ehJogadorAtacando);
static void ProximoTurno(EstadoBatalha *estado);
static void ExecutarTurnoIA(EstadoBatalha *estado);


static char CHAVE_API_BUFFER[256];
static bool chaveJaFoiCarregada = false; // Impede leituras repetidas

static const char* CarregarChaveApi(const char* caminhoArquivo) {
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
        // CORREÇÃO: Verifica se o alvo na lista está vivo
        NoPersonagem* noAlvo = ObterNoNaPosicao(&estado->timeJogador, alvoAleatorio);
        while (noAlvo == NULL && tentativas < 3) {
             alvoAleatorio = (alvoAleatorio + 1) % 3;
             noAlvo = ObterNoNaPosicao(&estado->timeJogador, alvoAleatorio);
             tentativas++;
        }
        // Se ainda assim não achar um alvo vivo, ataca a posição 0
        if (noAlvo == NULL) {
            alvoAleatorio = 0;
        }
        
        ExecutarAtaque(estado, atacante, &atacante->ataque1, alvoAleatorio, false);
        return;
    }

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
static int animVelocidadeIdle = 15; 

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
    if (anim->ativo == false) {
        return;
    }
    
    anim->timer++;
    if (anim->timer >= anim->velocidade) {
        anim->timer = 0;
        anim->frameAtual++;
        if (anim->frameAtual >= anim->anim->def.numFrames) {
            PararAnimacao(anim);
        }
    }
}

static void DesenharAnimacao(EstadoAnimacao* anim) {
    if (anim->ativo == false) {
        return;
    }
    if (anim->anim == NULL || anim->anim->def.numFrames == 0) {
        return;
    }
    if (anim->frameAtual >= anim->anim->def.numFrames) {
        anim->frameAtual = 0;
    }
    
    Rectangle frameRec = anim->anim->def.frames[anim->frameAtual];
    
    if (anim->flip) {
        frameRec.width = -frameRec.width; 
    }

    DrawTexturePro(anim->anim->textura,
                   frameRec,
                   (Rectangle){ anim->pos.x, anim->pos.y, frameRec.width * anim->zoom, frameRec.height * anim->zoom },
                   (Vector2){ frameRec.width * anim->zoom / 2, frameRec.height * anim->zoom / 2 },
                   0.0f,
                   WHITE);
}

static int CompararVelocidade(const void* a, const void* b) {
    PersonagemData* pA = *(PersonagemData**)a;
    PersonagemData* pB = *(PersonagemData**)b;
    
    if (pA == NULL || pB == NULL) {
        return 0;
    }
    
    if (pA->velocidade > pB->velocidade) {
        return -1;
    }
    if (pA->velocidade < pB->velocidade) {
        return 1;
    }
    // Desempate aleatório
    return (rand() % 2 == 0) ? 1 : -1;
}

static void ProximoTurno(EstadoBatalha *estado) {
    estado->personagemAgindoIdx++;
    if (estado->personagemAgindoIdx >= 6) {
        estado->personagemAgindoIdx = 0;
        printf("--- NOVO ROUND ---\n");
        estado->roundAtual++;
    }
    
    if (estado->ordemDeAtaque[estado->personagemAgindoIdx] == NULL) {
        printf("ERRO: Personagem na ordem de ataque e nulo!\n");
        return;
    }
    
    PersonagemData* personagemAtual = estado->ordemDeAtaque[estado->personagemAgindoIdx];
    
    // --- LÓGICA MODIFICADA (Usa Listas) ---
    bool estaVivo = false;
    bool ehJogador = false;

    // Procura na lista do Jogador
    NoPersonagem* noAtual = estado->timeJogador.inicio;
    while (noAtual != NULL) {
        if (noAtual->personagem == personagemAtual) {
            ehJogador = true;
            // Se o nó existe, o personagem está "vivo" na lista
            estaVivo = true; 
            break;
        }
        noAtual = noAtual->proximo;
    }

    // Se não encontrou, procura na lista da IA
    if (ehJogador == false) {
        noAtual = estado->timeIA.inicio;
        while (noAtual != NULL) {
            if (noAtual->personagem == personagemAtual) {
                 // Se o nó existe, o personagem está "vivo" na lista
                estaVivo = true;
                break;
            }
            noAtual = noAtual->proximo;
        }
    }
    // --- FIM DA LÓGICA MODIFICADA ---
    
    if (estaVivo == false) {
        // Personagem está morto (já foi removido da lista), pula o turno
        ProximoTurno(estado);
        return;
    }

    if (ehJogador) {
        estado->turnoDe = TURNO_JOGADOR;
        estado->estadoTurno = ESTADO_ESPERANDO_JOGADOR;
        estado->ataqueSelecionado = -1; 
        estado->alvoSelecionado = -1;
        sprintf(estado->mensagemBatalha, "Vez de: %s! Escolha um ataque.", personagemAtual->nome);
    } else {
        estado->turnoDe = TURNO_IA;
        estado->estadoTurno = ESTADO_TURNO_IA;
        sprintf(estado->mensagemBatalha, "Vez de: %s!", personagemAtual->nome);
    }
}

static void ExecutarAtaque(EstadoBatalha* estado, PersonagemData* atacante, Ataque* ataque, int alvoIdx, bool ehJogadorAtacando) {
    int dano = ataque->dano;
    PersonagemData* alvo = NULL;
    NoPersonagem* noAlvo = NULL; 
    
    Vector2 animPos = {0, 0}; // Posição de onde a animação vai sair

    // --- LÓGICA MODIFICADA (Usa Listas) ---
    if (ehJogadorAtacando) {
        // 1. Encontra a posição do atacante (Jogador) para a animação
        NoPersonagem* noAtacante = estado->timeJogador.inicio;
        while (noAtacante != NULL) {
            if (noAtacante->personagem == atacante) {
                animPos = posJogador[noAtacante->posicaoNoTime];
                break;
            }
            noAtacante = noAtacante->proximo;
        }

        // 2. Encontra o alvo (IA) usando a lista
        noAlvo = ObterNoNaPosicao(&estado->timeIA, alvoIdx);
        if (noAlvo == NULL) {
             printf("ATAQUE: Jogador tentou atacar posicao %d vazia.\n", alvoIdx);
             ProximoTurno(estado); // Pula o turno se o alvo não existe
             return; 
        }
        alvo = noAlvo->personagem; 
        
        // 3. Aplica dano e mensagem
        estado->hpIA[alvoIdx] -= dano; // O HP ainda é controlado pelo array
        sprintf(estado->mensagemBatalha, "%s usou %s em %s e causou %d de dano!", atacante->nome, ataque->nome, alvo->nome, dano);
        
        // 4. Verifica se morreu e remove da lista
        if (estado->hpIA[alvoIdx] <= 0) {
            estado->hpIA[alvoIdx] = 0;
            RemoverPersonagem(&estado->timeIA, noAlvo);
            // TODO: Checar condição de vitória
        }

    } else { // é a IA atacando
        // 1. Encontra a posição do atacante (IA) para a animação
        NoPersonagem* noAtacante = estado->timeIA.inicio;
        while (noAtacante != NULL) {
            if (noAtacante->personagem == atacante) {
                animPos = posIA[noAtacante->posicaoNoTime];
                break;
            }
            noAtacante = noAtacante->proximo;
        }

        // 2. Encontra o alvo (Jogador) usando a lista
        noAlvo = ObterNoNaPosicao(&estado->timeJogador, alvoIdx);
         if (noAlvo == NULL) {
             printf("ATAQUE: IA tentou atacar posicao %d vazia.\n", alvoIdx);
             ProximoTurno(estado); // Pula o turno se o alvo não existe
             return; 
        }
        alvo = noAlvo->personagem;

        // 3. Aplica dano e mensagem
        estado->hpJogador[alvoIdx] -= dano;
        sprintf(estado->mensagemBatalha, "%s usou %s em %s e causou %d de dano!", atacante->nome, ataque->nome, alvo->nome, dano);

        // 4. Verifica se morreu e remove da lista
        if (estado->hpJogador[alvoIdx] <= 0) {
            estado->hpJogador[alvoIdx] = 0;
            RemoverPersonagem(&estado->timeJogador, noAlvo);
            // TODO: Checar condição de derrota
        }
    }
    // --- FIM DA LÓGICA MODIFICADA ---

    bool deveFlipar = !ehJogadorAtacando;
    
    // Inicia a animação de ataque
    if (strcmp(ataque->nome, atacante->ataque1.nome) == 0) {
        IniciarAnimacao(&estado->animacaoEmExecucao, &atacante->animAtaque1, animPos, atacante->batalhaZoom, deveFlipar);
    } else {
        IniciarAnimacao(&estado->animacaoEmExecucao, &atacante->animAtaque2, animPos, atacante->batalhaZoom, deveFlipar);
    }

    estado->estadoTurno = ESTADO_ANIMACAO_ATAQUE;
}


void InicializarBatalha(EstadoBatalha *estado, TimesBatalha* timesSelecionados) {
    printf("INICIALIZANDO BATALHA!\n");
    estado->roundAtual = 1;

    // --- LÓGICA MODIFICADA (Usa Listas) ---
    // 1. Cria as duas listas duplamente encadeadas
    estado->timeJogador = CriarLista();
    estado->timeIA = CriarLista();

    // 2. Popula as listas e define o HP inicial
    for (int i = 0; i < 3; i++) {
        if (timesSelecionados->timeJogador[i] == NULL || timesSelecionados->timeIA[i] == NULL) {
            printf("ERRO FATAL: Time nao selecionado corretamente!\n");
            return;
        }
        // Insere na lista do jogador (posicao 0, 1, 2)
        InserirPersonagem(&estado->timeJogador, timesSelecionados->timeJogador[i], i);
        estado->hpJogador[i] = timesSelecionados->timeJogador[i]->hpMax;

        // Insere na lista da IA (posicao 0, 1, 2)
        InserirPersonagem(&estado->timeIA, timesSelecionados->timeIA[i], i);
        estado->hpIA[i] = timesSelecionados->timeIA[i]->hpMax;
    }
    // --- FIM DA LÓGICA MODIFICADA ---
    
    // Posições de desenho
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

    
    // --- ***** A CORREÇÃO DO CRASH ESTÁ AQUI ***** ---
   // 4. Cria e ordena a fila de ataque
    printf("Populando ordem de ataque...\n");
    for (int i = 0; i < 3; i++) {
        // CORRIGIDO: Deve usar 'timesSelecionados' (o parâmetro da função)
        estado->ordemDeAtaque[i] = timesSelecionados->timeJogador[i];
        estado->ordemDeAtaque[i+3] = timesSelecionados->timeIA[i];
    }
    qsort(estado->ordemDeAtaque, 6, sizeof(PersonagemData*), CompararVelocidade);
    // --- ***** FIM DA CORREÇÃO ***** ---
    
    qsort(estado->ordemDeAtaque, 6, sizeof(PersonagemData*), CompararVelocidade);

    printf("Ordem de ataque:\n");
    for(int i=0; i<6; i++) {
        if (estado->ordemDeAtaque[i] != NULL) { 
            printf("  %d. %s (Vel: %d)\n", i+1, estado->ordemDeAtaque[i]->nome, estado->ordemDeAtaque[i]->velocidade);
        } else {
            printf("  %d. (NULL)\n", i+1); // Debug
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
        // Limpa a memória das listas ao sair
        LiberarLista(&estado->timeJogador);
        LiberarLista(&estado->timeIA);
        return;
    }
    
    // --- LÓGICA MODIFICADA (Usa Listas) ---
    // Atualiza animação idle de todos os personagens VIVOS na lista do Jogador
    NoPersonagem* noAtualJogador = estado->timeJogador.inicio;
    while (noAtualJogador != NULL) {
        int i = noAtualJogador->posicaoNoTime;
        if (noAtualJogador->personagem->animIdle.def.numFrames > 0) {
            estado->idleTimerJogador[i]++;
            if (estado->idleTimerJogador[i] > animVelocidadeIdle) {
                estado->idleTimerJogador[i] = 0;
                estado->idleFrameJogador[i]++;
                if (estado->idleFrameJogador[i] >= noAtualJogador->personagem->animIdle.def.numFrames) {
                    estado->idleFrameJogador[i] = 0;
                }
            }
        }
        noAtualJogador = noAtualJogador->proximo;
    }
    
    // Atualiza animação idle de todos os personagens VIVOS na lista da IA
    NoPersonagem* noAtualIA = estado->timeIA.inicio;
    while (noAtualIA != NULL) {
        int i = noAtualIA->posicaoNoTime;
        if (noAtualIA->personagem->animIdle.def.numFrames > 0) {
            estado->idleTimerIA[i]++;
            if (estado->idleTimerIA[i] > animVelocidadeIdle) {
                estado->idleTimerIA[i] = 0;
                estado->idleFrameIA[i]++;
                if (estado->idleFrameIA[i] >= noAtualIA->personagem->animIdle.def.numFrames) {
                    estado->idleFrameIA[i] = 0;
                }
            }
        }
        noAtualIA = noAtualIA->proximo;
    }
    // --- FIM DA LÓGICA MODIFICADA ---

    if (estado->estadoTurno != ESTADO_INICIANDO && (estado->personagemAgindoIdx < 0 || estado->personagemAgindoIdx >= 6 || estado->ordemDeAtaque[estado->personagemAgindoIdx] == NULL)) {
        if(estado->estadoTurno != ESTADO_FIM_DE_JOGO) {
            ProximoTurno(estado);
        }
        return;
    }

    AtualizarAnimacao(&estado->animacaoEmExecucao);

    // Checagem de Fim de Jogo
    if (estado->timeJogador.tamanho == 0 && estado->estadoTurno != ESTADO_FIM_DE_JOGO) {
        estado->estadoTurno = ESTADO_FIM_DE_JOGO;
        sprintf(estado->mensagemBatalha, "VOCE PERDEU! Pressione ESC para sair.");
    } else if (estado->timeIA.tamanho == 0 && estado->estadoTurno != ESTADO_FIM_DE_JOGO) {
        estado->estadoTurno = ESTADO_FIM_DE_JOGO;
        sprintf(estado->mensagemBatalha, "VOCE VENCEU! Pressione ESC para sair.");
    }


    switch (estado->estadoTurno) {
        
        case ESTADO_INICIANDO:
            ProximoTurno(estado);
            break;

        case ESTADO_ESPERANDO_JOGADOR:
            {
                PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
                
                int arenaY = 80;
                int arenaHeight = 550; 
                int menuY = arenaY + arenaHeight + 10; 
                int textoYBase = menuY + 20;           
                int colAtaquesX = 35;

                Rectangle btnAtk1 = { colAtaquesX, textoYBase + 35, 200, 40 }; 
                Rectangle btnAtk2 = { colAtaquesX, textoYBase + 90, 200, 40 }; 
                
                // Caixas de colisão dos alvos
                Rectangle alvoIA_0 = {posIA[0].x - 50, posIA[0].y - 50, 100, 100};
                Rectangle alvoIA_1 = {posIA[1].x - 50, posIA[1].y - 50, 100, 100};
                Rectangle alvoIA_2 = {posIA[2].x - 50, posIA[2].y - 50, 100, 100};
                
                Vector2 mousePos = GetMouseVirtual();

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousePos, btnAtk1)) {
                        estado->ataqueSelecionado = 0;
                    }
                    if (CheckCollisionPointRec(mousePos, btnAtk2)) {
                         estado->ataqueSelecionado = 1;
                    }
                }
                
                if (estado->ataqueSelecionado != -1) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        int alvo = -1;
                        // --- LÓGICA MODIFICADA (Usa Listas) ---
                        // Verifica se clicou e se o alvo EXISTE NA LISTA
                        if (CheckCollisionPointRec(mousePos, alvoIA_0)) {
                            if (ObterNoNaPosicao(&estado->timeIA, 0) != NULL) {
                                alvo = 0;
                            }
                        }
                        if (CheckCollisionPointRec(mousePos, alvoIA_1)) {
                             if (ObterNoNaPosicao(&estado->timeIA, 1) != NULL) {
                                alvo = 1;
                             }
                        }
                        if (CheckCollisionPointRec(mousePos, alvoIA_2)) {
                            if (ObterNoNaPosicao(&estado->timeIA, 2) != NULL) {
                                alvo = 2;
                            }
                        }
                        // --- FIM DA LÓGICA MODIFICADA ---
                        
                        if (alvo != -1) {
                            estado->alvoSelecionado = alvo;
                            Ataque* att;
                            if (estado->ataqueSelecionado == 0) {
                                att = &atacante->ataque1;
                            } else {
                                att = &atacante->ataque2;
                            }
                            ExecutarAtaque(estado, atacante, att, estado->alvoSelecionado, true);
                        }
                    }
                }
            }
            break;
            
        case ESTADO_ANIMACAO_ATAQUE:
            if (estado->animacaoEmExecucao.ativo == false) {
                // Não chama o próximo turno se o jogo acabou
                if (estado->timeJogador.tamanho > 0 && estado->timeIA.tamanho > 0) {
                    ProximoTurno(estado);
                }
            }
            break;

        case ESTADO_TURNO_IA:
            ExecutarTurnoIA(estado);
            break;
            
        case ESTADO_FIM_DE_JOGO:
            // Não faz nada, espera o jogador apertar ESC
            break;
    }
}

void DesenharTelaBatalha(EstadoBatalha *estado) {
    DrawText("Jogador 1", 20, 15, 20, RAYWHITE);

    const char* textoRound = TextFormat("Round: %d", estado->roundAtual);
    int larguraTextoRound = MeasureText(textoRound, 20);
    DrawText(textoRound, (SCREEN_WIDTH - larguraTextoRound) / 2, 15, 20, RAYWHITE);

    DrawText("IA", SCREEN_WIDTH - 20 - 200 - 35, 15, 20, RAYWHITE);

    int arenaY = 80;
    int arenaHeight = 550; 
    DrawRectangleLines(10, arenaY, SCREEN_WIDTH - 20, arenaHeight, LIGHTGRAY);
    
    // --- LÓGICA MODIFICADA (Usa Listas) ---
    // Desenha a lista de personagens do Jogador
    NoPersonagem* noAtualJogador = estado->timeJogador.inicio;
    while (noAtualJogador != NULL) {
        int i = noAtualJogador->posicaoNoTime;
        PersonagemData* pData = noAtualJogador->personagem;
        
        AnimacaoData* anim = &pData->animIdle;
        if (anim->def.numFrames > 0) {
            int frameIndex = estado->idleFrameJogador[i];
            if (frameIndex >= anim->def.numFrames) {
                frameIndex = 0;
            }
            Rectangle frame = anim->def.frames[frameIndex];
            
            float zoom = pData->batalhaZoom;

            DrawTexturePro(anim->textura, frame,
                (Rectangle){posJogador[i].x, posJogador[i].y, frame.width * zoom, frame.height * zoom},
                (Vector2){frame.width * zoom / 2, frame.height * zoom / 2}, 0, WHITE);
            
            // Barra de HP
            float hpBarY = posJogador[i].y + (frame.height * zoom / 2) + 5;
            int posXBarra = posJogador[i].x - 50;
            int posYBarra = (int)hpBarY;
            int larguraBarra = 100;
            int alturaBarra = 10;
            
            DrawRectangle(posXBarra, posYBarra, larguraBarra, alturaBarra, DARKGRAY);
            
            int larguraPreenchimento = (int)((float)larguraBarra * (float)estado->hpJogador[i] / (float)pData->hpMax);
            DrawRectangle(posXBarra, posYBarra, larguraPreenchimento, alturaBarra, GREEN);
            
            DrawRectangleLines(posXBarra, posYBarra, larguraBarra, alturaBarra, LIGHTGRAY);
        }
        
        noAtualJogador = noAtualJogador->proximo;
    }
    
    // Desenha a lista de personagens da IA
    NoPersonagem* noAtualIA = estado->timeIA.inicio;
    while (noAtualIA != NULL) {
        int i = noAtualIA->posicaoNoTime;
        PersonagemData* pDataIA = noAtualIA->personagem;

        AnimacaoData* anim = &pDataIA->animIdle;
        if (anim->def.numFrames > 0) {

            int frameIndexIA = estado->idleFrameIA[i];
            if (frameIndexIA >= anim->def.numFrames) {
                frameIndexIA = 0;
            }
            Rectangle frame = anim->def.frames[frameIndexIA];

            float zoomIA = pDataIA->batalhaZoom;
            Rectangle frameIA = frame;
            frameIA.width = -frame.width; // Flip

            DrawTexturePro(anim->textura, frameIA,
                (Rectangle){posIA[i].x, posIA[i].y, frame.width * zoomIA, frame.height * zoomIA},
                (Vector2){frame.width * zoomIA / 2, frame.height * zoomIA / 2}, 0, WHITE);
            
            // Barra de HP
            float hpBarY = posIA[i].y + (frame.height * zoomIA / 2) + 5;
            int posXBarraIA = posIA[i].x - 50;
            int posYBarraIA = (int)hpBarY;
            int larguraBarraIA = 100;
            int alturaBarraIA = 10;

            DrawRectangle(posXBarraIA, posYBarraIA, larguraBarraIA, alturaBarraIA, DARKGRAY);
            
            int larguraPreenchimentoIA = (int)((float)larguraBarraIA * (float)estado->hpIA[i] / (float)pDataIA->hpMax);
            DrawRectangle(posXBarraIA, posYBarraIA, larguraPreenchimentoIA, alturaBarraIA, RED);
            
            DrawRectangleLines(posXBarraIA, posYBarraIA, larguraBarraIA, alturaBarraIA, LIGHTGRAY);
        }

        noAtualIA = noAtualIA->proximo;
    }
    // --- FIM DA LÓGICA MODIFICADA ---

    
    DesenharAnimacao(&estado->animacaoEmExecucao);
    
    // Desenha as caixas de seleção de alvo
    if (estado->estadoTurno == ESTADO_ESPERANDO_JOGADOR && estado->ataqueSelecionado != -1) {
        // Itera na lista da IA para desenhar a caixa apenas nos vivos
        noAtualIA = estado->timeIA.inicio;
        while (noAtualIA != NULL) {
            int i = noAtualIA->posicaoNoTime;
            DrawRectangleLines(posIA[i].x - 50, posIA[i].y - 50, 100, 100, YELLOW);
            noAtualIA = noAtualIA->proximo;
        }
    }

    // --- Menu Inferior ---
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
            
            Color corAtk1 = corBotaoNormal;
            Color corAtk2 = corBotaoNormal;
            if (estado->ataqueSelecionado == 0) {
                corAtk1 = corBotaoSelecionado;
            }
            if (estado->ataqueSelecionado == 1) {
                corAtk2 = corBotaoSelecionado;
            }

            DrawRectangleRounded(btnAtk1, 0.2f, 4, corAtk1);
            DrawRectangleRoundedLinesEx(btnAtk1, 0.2f, 4, espessuraBorda, BLACK);
            DrawText(atacante->ataque1.nome, btnAtk1.x + (btnAtk1.width - MeasureText(atacante->ataque1.nome, 20)) / 2, btnAtk1.y + 10, 20, corTexto);

            DrawRectangleRounded(btnAtk2, 0.2f, 4, corAtk2);
            DrawRectangleRoundedLinesEx(btnAtk2, 0.2f, 4, espessuraBorda, BLACK);
            DrawText(atacante->ataque2.nome, btnAtk2.x + (btnAtk2.width - MeasureText(atacante->ataque2.nome, 20)) / 2, btnAtk2.y + 10, 20, corTexto);
        
            Vector2 mousePos = GetMouseVirtual();
            if (CheckCollisionPointRec(mousePos, btnAtk1)) {
                DrawText(atacante->ataque1.descricao, colSpecsX, textoYBase + 40, 20, RAYWHITE);
                DrawText(TextFormat("Causa %d de Dano.", atacante->ataque1.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
            } else {
                if (CheckCollisionPointRec(mousePos, btnAtk2)) {
                    DrawText(atacante->ataque2.descricao, colSpecsX, textoYBase + 40, 20, RAYWHITE);
                    DrawText(TextFormat("Causa %d de Dano.", atacante->ataque2.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
                }
            }
        }
        
    } else {
        // Modo "Log" ou Fim de Jogo
        int larguraLog = MeasureText(estado->mensagemBatalha, 20);
        int logX = (SCREEN_WIDTH - larguraLog) / 2;
        if (logX < 15) { 
            logX = 15;
        }
        DrawText(estado->mensagemBatalha, logX, textoYBase + 60, 20, WHITE);
    }
}