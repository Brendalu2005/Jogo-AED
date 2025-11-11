#include "batalha.h"
#include "database.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "ConsumoAPI_Gemini.h" 
#include "math.h"

#include "telas.h"


static void ExecutarAtaque(EstadoBatalha* estado, PersonagemData* atacante, Ataque* ataque, int alvoIdx, bool ehJogadorAtacando);
static void ProximoTurno(EstadoBatalha *estado);
static void ExecutarTurnoIA(EstadoBatalha *estado);

Texture2D backgroundArena;
int backgroundCarregado = 0;


// --- NOVA FUNÇÃO AUXILIAR ---
/**
 * @brief Encontra um NoPersonagem numa lista com base no ponteiro PersonagemData.
 * * @param lista A lista (Time do Jogador ou Time da IA).
 * @param p O ponteiro para o PersonagemData que procuramos.
 * @return O NoPersonagem* correspondente ou NULL se não for encontrado.
 */

static NoPersonagem* ObterNoPorPersonagem(ListaTime* lista, PersonagemData* p) {
    if (lista == NULL) {
        return NULL;
    }
    if (p == NULL) {
        return NULL;
    }

    NoPersonagem* atual = lista->inicio;
    while (atual != NULL) {
        if (atual->personagem == p) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL; // Não encontrou
}
// --- FIM DA NOVA FUNÇÃO ---


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

static void IniciarZoomEAnimacao(EstadoBatalha* estado, PersonagemData* atacante, PersonagemData* alvo, int alvoIdx, bool ehJogadorAtacando, Ataque* ataque) {
    
    bool deveFlipar;
    if (ehJogadorAtacando) {
        deveFlipar = false;
    } else {
        deveFlipar = true;
    }

    // Prepara para o Zoom In
    estado->atacanteEmFoco = atacante;
    estado->alvoEmFoco = alvo; // Pode ser NULL (no caso de cura)
    estado->alvoEmFocoIdx = alvoIdx; // Pode ser -1 (no caso de cura)
    estado->timerFoco = 0.0f;
    estado->alphaOutrosPersonagens = 1.0f;
    estado->animFlip = deveFlipar;

    // --- NOVO: faz o personagem atacado (se houver) virar ---
    if (ehJogadorAtacando == false && alvo != NULL) {
        // A IA está atacando ? o alvo (jogador) vira para a esquerda
        alvo->animIdle.flip = true;
    }
    
    // Guarda qual animação deve tocar depois do zoom
    if (strcmp(ataque->nome, atacante->ataque1.nome) == 0) {
        estado->animParaTocar = &atacante->animAtaque1;
    } else {
        estado->animParaTocar = &atacante->animAtaque2;
    }

    // Define o próximo estado como o Zoom In
    estado->estadoTurno = ESTADO_ZOOM_IN_ATAQUE;
}


void DesenharAnimacao(EstadoAnimacao* anim) {
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

    if (anim->flip){
        // "IA" aqui significa Oponente (Time 2)
        DrawText("Oponente", anim->pos.x - 40, anim->pos.y - 100, 20, RED);
    }
    else{
        DrawText("Jogador 1", anim->pos.x - 40, anim->pos.y - 100, 20, BLUE);
        
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

static int CompararVelocidade(const void* a, const void* b) {
    PersonagemData* pA = *(PersonagemData**)a;
    PersonagemData* pB = *(PersonagemData**)b;
    
    if (pA == NULL) {
        return 1; // Coloca nulos no fim
    }
    if (pB == NULL) {
        return -1; // Coloca nulos no fim
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
        printf("ERRO: Personagem na ordem de ataque e nulo! Pulando turno.\n");
        // Chama o próximo turno recursivamente para pular este
        ProximoTurno(estado);
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

    // Se não encontrou, procura na lista da IA/Oponente
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

    // Define o estado baseado em quem joga
    if (ehJogador) {
        estado->turnoDe = TURNO_JOGADOR;
        estado->estadoTurno = ESTADO_ESPERANDO_JOGADOR;
        estado->ataqueSelecionado = -1; 
        estado->alvoSelecionado = -1;
        sprintf(estado->mensagemBatalha, "Vez de: %s! (J1) Escolha um ataque.", personagemAtual->nome);
    } else {
        estado->turnoDe = TURNO_IA; // Enum "IA" significa Time 2 / Oponente
        estado->estadoTurno = ESTADO_AGUARDANDO_OPONENTE; // Novo estado
        sprintf(estado->mensagemBatalha, "Vez de: %s! (Oponente)", personagemAtual->nome);
    }
}

static void ExecutarAtaque(EstadoBatalha* estado, PersonagemData* atacante, Ataque* ataque, int alvoIdx, bool ehJogadorAtacando) {
    int dano = ataque->dano;
    PersonagemData* alvo = NULL;
    NoPersonagem* noAlvo = NULL; 

    // Variáveis para o time alvo (depende de quem ataca)
    ListaTime* listaAlvo;
    int* hpArrayAlvo;
    
    // Variáveis para o time atacante (usado na CURA)
    ListaTime* listaAtacante;
    int* hpArrayAtacante;

    if (ehJogadorAtacando) {
        // Jogador 1 ataca
        listaAlvo = &estado->timeIA;
        hpArrayAlvo = estado->hpIA;
        listaAtacante = &estado->timeJogador;
        hpArrayAtacante = estado->hpJogador;
    } else {
        // Oponente (IA ou J2) ataca
        listaAlvo = &estado->timeJogador;
        hpArrayAlvo = estado->hpJogador;
        listaAtacante = &estado->timeIA;
        hpArrayAtacante = estado->hpIA;
    }

    // --- LÓGICA PRINCIPAL DOS TIPOS DE ATAQUE ---
    switch (ataque->tipo) {
        
        // --- CASO 1: DANO EM ALVO ÚNICO (Lógica antiga) ---
        case TIPO_DANO_UNICO:
        {
            noAlvo = ObterNoNaPosicao(listaAlvo, alvoIdx);
            if (noAlvo == NULL) {
                 if (ehJogadorAtacando) {
                    printf("ATAQUE: Jogador 1 tentou atacar posicao %d vazia.\n", alvoIdx);
                 } else {
                    printf("ATAQUE: Oponente tentou atacar posicao %d vazia.\n", alvoIdx);
                 }
                 ProximoTurno(estado); // Pula o turno se o alvo não existe
                 return; 
            }
            alvo = noAlvo->personagem; 
            
            hpArrayAlvo[alvoIdx] -= dano;
            sprintf(estado->mensagemBatalha, "%s usou %s em %s e causou %d de dano!", atacante->nome, ataque->nome, alvo->nome, dano);
            
            if (hpArrayAlvo[alvoIdx] <= 0) {
                hpArrayAlvo[alvoIdx] = 0;
                RemoverPersonagem(listaAlvo, noAlvo);
            }
            
            // Inicia o zoom e a animação
            IniciarZoomEAnimacao(estado, atacante, alvo, alvoIdx, ehJogadorAtacando, ataque);
            break;
        }

        // --- CASO 2: DANO EM ÁREA ---
        case TIPO_DANO_AREA:
        {
            sprintf(estado->mensagemBatalha, "%s usou %s e atingiu TODOS por %d de dano!", atacante->nome, ataque->nome, dano);

            // Loop pelos 3 alvos
            for (int i = 0; i < 3; i++) {
                noAlvo = ObterNoNaPosicao(listaAlvo, i);
                
                // Se o alvo existe (está vivo)
                if (noAlvo != NULL) {
                    hpArrayAlvo[i] -= dano; // Aplica dano
                    
                    if (hpArrayAlvo[i] <= 0) {
                        hpArrayAlvo[i] = 0;
                        RemoverPersonagem(listaAlvo, noAlvo);
                    }
                }
            }
            
            // Para o zoom, vamos focar na POSIÇÃO 1 (meio) do time inimigo
            alvoIdx = 1; 
            noAlvo = ObterNoNaPosicao(listaAlvo, alvoIdx);
            if (noAlvo != NULL) {
                alvo = noAlvo->personagem;
            } else {
                alvo = NULL; // Posição 1 pode estar morta, o zoom foca no vazio
            }

            IniciarZoomEAnimacao(estado, atacante, alvo, alvoIdx, ehJogadorAtacando, ataque);
            break;
        }

        // --- CASO 3: CURA EM SI MESMO ---
        case TIPO_CURA_SI:
        {
            int cura = ataque->dano; // Reinterpretamos 'dano' como 'cura'
            NoPersonagem* noAtacante = ObterNoPorPersonagem(listaAtacante, atacante);

            if (noAtacante != NULL) {
                int posAtacante = noAtacante->posicaoNoTime;
                int hpMax = atacante->hpMax;

                hpArrayAtacante[posAtacante] += cura;
                
                // Impede sobrecura
                if (hpArrayAtacante[posAtacante] > hpMax) {
                    hpArrayAtacante[posAtacante] = hpMax;
                }
                
                sprintf(estado->mensagemBatalha, "%s usou %s e curou %d de vida!", atacante->nome, ataque->nome, cura);

            } else {
                // Isso não deve acontecer se a lógica estiver correta
                sprintf(estado->mensagemBatalha, "%s tentou se curar, mas falhou.", atacante->nome);
            }

            IniciarZoomEAnimacao(estado, atacante, NULL, -1, ehJogadorAtacando, ataque);
            break;
        }
    }
}


void InicializarBatalha(EstadoBatalha *estado, TimesBatalha* timesSelecionados) {
    printf("INICIALIZANDO BATALHA!\n");

    if (backgroundCarregado == 0) {
        backgroundArena = LoadTexture("sprites/background/backgroundbatalha.png");
        backgroundCarregado = 1;
    }

    estado->roundAtual = 1;

    // --- LÓGICA MODIFICADA (Usa Listas) ---
    estado->timeJogador = CriarLista();
    estado->timeIA = CriarLista();

    for (int i = 0; i < 3; i++) {
        if (timesSelecionados->timeJogador[i] == NULL) {
            printf("ERRO FATAL: Time do Jogador 1 nao selecionado corretamente!\n");
            return;
        }
         if (timesSelecionados->timeIA[i] == NULL) {
            printf("ERRO FATAL: Time do Oponente nao selecionado corretamente!\n");
            return;
        }
        
        InserirPersonagem(&estado->timeJogador, timesSelecionados->timeJogador[i], i);
        estado->hpJogador[i] = timesSelecionados->timeJogador[i]->hpMax;

        InserirPersonagem(&estado->timeIA, timesSelecionados->timeIA[i], i);
        estado->hpIA[i] = timesSelecionados->timeIA[i]->hpMax;
    }
    
    // Posições de desenho
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

    
   // 4. Cria e ordena a fila de ataque
    printf("Populando ordem de ataque...\n");
    for (int i = 0; i < 3; i++) {
        estado->ordemDeAtaque[i] = timesSelecionados->timeJogador[i];
        estado->ordemDeAtaque[i+3] = timesSelecionados->timeIA[i];
    }
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

    // Inicializa as novas variáveis de foco
    estado->atacanteEmFoco = NULL;
    estado->alvoEmFoco = NULL;
    estado->alvoEmFocoIdx = -1; 
    estado->timerFoco = 0.0f;
    estado->animParaTocar = NULL;
    estado->animFlip = false;
    estado->alphaOutrosPersonagens = 1.0f;
}

// Assinatura modificada para aceitar ModoDeJogo
void AtualizarTelaBatalha(EstadoBatalha *estado, GameScreen *telaAtual, ModoDeJogo modo) {
    if (IsKeyPressed(KEY_ESCAPE)) {

        if (backgroundCarregado == 1) {
            UnloadTexture(backgroundArena);
            backgroundCarregado = 0;
        }

        *telaAtual = SCREEN_MENU;
        LiberarLista(&estado->timeJogador);
        LiberarLista(&estado->timeIA);
        return;
    }
    
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

    if (estado->estadoTurno != ESTADO_INICIANDO) {
        if (estado->personagemAgindoIdx < 0 || estado->personagemAgindoIdx >= 6 || estado->ordemDeAtaque[estado->personagemAgindoIdx] == NULL) {
            if(estado->estadoTurno != ESTADO_FIM_DE_JOGO) {
                ProximoTurno(estado);
            }
            return;
        }
    }


    AtualizarAnimacao(&estado->animacaoEmExecucao);

    // Checagem de Fim de Jogo
    if (estado->timeJogador.tamanho == 0 && estado->estadoTurno != ESTADO_FIM_DE_JOGO) {
        estado->estadoTurno = ESTADO_FIM_DE_JOGO;
        sprintf(estado->mensagemBatalha, "OPONENTE VENCEU! Pressione ESC para sair.");
    } else if (estado->timeIA.tamanho == 0 && estado->estadoTurno != ESTADO_FIM_DE_JOGO) {
        estado->estadoTurno = ESTADO_FIM_DE_JOGO;
        sprintf(estado->mensagemBatalha, "JOGADOR 1 VENCEU! Pressione ESC para sair.");
    }


    switch (estado->estadoTurno) {
        
        case ESTADO_INICIANDO:
            ProximoTurno(estado);
            break;

        // Este estado agora serve para J1 e J2
        case ESTADO_ESPERANDO_JOGADOR:
            {
                // Verifica de quem é o turno
                bool ehJogador1 = (estado->turnoDe == TURNO_JOGADOR);

                int arenaY = 80;
                int arenaHeight = 550; 
                int menuY = arenaY + arenaHeight + 10; 
                int textoYBase = menuY + 20;           
                int colAtaquesX = 35;

                Rectangle btnAtk1 = { colAtaquesX, textoYBase + 35, 200, 40 }; 
                Rectangle btnAtk2 = { colAtaquesX, textoYBase + 90, 200, 40 }; 
                
                // Define os alvos baseado em quem está jogando
                Rectangle alvo_0, alvo_1, alvo_2;
                ListaTime* listaAlvo;
                
                if (ehJogador1) {
                    // P1 ataca P2/IA
                    alvo_0 = (Rectangle){posIA[0].x - 50, posIA[0].y - 50, 100, 100};
                    alvo_1 = (Rectangle){posIA[1].x - 50, posIA[1].y - 50, 100, 100};
                    alvo_2 = (Rectangle){posIA[2].x - 50, posIA[2].y - 50, 100, 100};
                    listaAlvo = &estado->timeIA;
                } else {
                    // P2 ataca P1
                    alvo_0 = (Rectangle){posJogador[0].x - 50, posJogador[0].y - 50, 100, 100};
                    alvo_1 = (Rectangle){posJogador[1].x - 50, posJogador[1].y - 50, 100, 100};
                    alvo_2 = (Rectangle){posJogador[2].x - 50, posJogador[2].y - 50, 100, 100};
                    listaAlvo = &estado->timeJogador;
                }
                
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
                        
                        if (CheckCollisionPointRec(mousePos, alvo_0)) {
                            if (ObterNoNaPosicao(listaAlvo, 0) != NULL) {
                                alvo = 0;
                            }
                        }
                        if (CheckCollisionPointRec(mousePos, alvo_1)) {
                             if (ObterNoNaPosicao(listaAlvo, 1) != NULL) {
                                alvo = 1;
                             }
                        }
                        if (CheckCollisionPointRec(mousePos, alvo_2)) {
                            if (ObterNoNaPosicao(listaAlvo, 2) != NULL) {
                                alvo = 2;
                            }
                        }
                        
                        if (alvo != -1) {
                            estado->alvoSelecionado = alvo; 
                            PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
                            Ataque* att;
                            if (estado->ataqueSelecionado == 0) {
                                att = &atacante->ataque1;
                            } else {
                                att = &atacante->ataque2;
                            }
                            // Passa 'ehJogador1' para ExecutarAtaque
                            ExecutarAtaque(estado, atacante, att, alvo, ehJogador1);
                        }
                    }
                }
            }
            break;
            
        case ESTADO_ANIMACAO_ATAQUE:
            if (estado->animacaoEmExecucao.ativo == false) {
                estado->estadoTurno = ESTADO_ZOOM_OUT_ATAQUE;
                estado->timerFoco = 0.0f; 
            }
            break;

        case ESTADO_ZOOM_IN_ATAQUE:
            {
                float duracaoZoom = 0.3f; 
                estado->timerFoco += GetFrameTime();
                float progresso = estado->timerFoco / duracaoZoom;

                if (progresso > 1.0f) {
                    progresso = 1.0f;
                }

                bool ehOponenteAtacando = estado->animFlip; // 'flip' é true se o Oponente (J2/IA) ataca

                NoPersonagem* noAtacante = NULL;
                if (ehOponenteAtacando) {
                    noAtacante = ObterNoPorPersonagem(&estado->timeIA, estado->atacanteEmFoco);
                } else {
                    noAtacante = ObterNoPorPersonagem(&estado->timeJogador, estado->atacanteEmFoco);
                }
                
                Vector2 posOriginalAtacante;
                if (noAtacante != NULL) {
                     posOriginalAtacante = ehOponenteAtacando ? posIA[noAtacante->posicaoNoTime] : posJogador[noAtacante->posicaoNoTime];
                } else {
                    posOriginalAtacante = (Vector2){ -200.0f, 350.0f }; 
                }
               
                Vector2 posAlvoAtacante;
                float zoomOriginalAtacante = estado->atacanteEmFoco->batalhaZoom;
                float zoomAlvo = 2.5f; 

                // Verifica se há um alvo para o zoom
                if (estado->alvoEmFoco != NULL || estado->alvoEmFocoIdx != -1) {
                    // --- Lógica de zoom com alvo (Dano Único ou Área) ---
                    NoPersonagem* noAlvo = NULL;
                    if (ehOponenteAtacando) {
                        noAlvo = ObterNoPorPersonagem(&estado->timeJogador, estado->alvoEmFoco);
                    } else {
                        noAlvo = ObterNoPorPersonagem(&estado->timeIA, estado->alvoEmFoco);
                    }
                    
                    Vector2 posOriginalAlvo;
                    float zoomOriginalAlvo = 2.0f; // Valor padrão se alvo estiver morto
                    
                    if (noAlvo != NULL) {
                         posOriginalAlvo = ehOponenteAtacando ? posJogador[noAlvo->posicaoNoTime] : posIA[noAlvo->posicaoNoTime];
                         zoomOriginalAlvo = estado->alvoEmFoco->batalhaZoom;
                    } else {
                         // Se o nó do alvo é nulo (morto), usa o índice guardado para a posição
                         posOriginalAlvo = ehOponenteAtacando ? posJogador[estado->alvoEmFocoIdx] : posIA[estado->alvoEmFocoIdx];
                    }

                    Vector2 posAlvoAlvo;

                    if (ehOponenteAtacando) {
                        posAlvoAtacante = (Vector2){ (float)SCREEN_WIDTH / 2.0f + 200.0f, 350.0f };
                        posAlvoAlvo = (Vector2){ (float)SCREEN_WIDTH / 2.0f - 200.0f, 350.0f };
                    } else {
                        posAlvoAtacante = (Vector2){ (float)SCREEN_WIDTH / 2.0f - 200.0f, 350.0f };
                        posAlvoAlvo = (Vector2){ (float)SCREEN_WIDTH / 2.0f + 200.0f, 350.0f };
                    }
                    
                    estado->posFocoAlvo.x = posOriginalAlvo.x + (posAlvoAlvo.x - posOriginalAlvo.x) * progresso;
                    estado->posFocoAlvo.y = posOriginalAlvo.y + (posAlvoAlvo.y - posOriginalAlvo.y) * progresso;
                    estado->zoomFocoAlvo = zoomOriginalAlvo + (zoomAlvo - zoomOriginalAlvo) * progresso;

                } else {
                    // --- Lógica de zoom sem alvo (Cura) ---
                    posAlvoAtacante = (Vector2){ (float)SCREEN_WIDTH / 2.0f, 350.0f }; // Foca no centro
                }

                // Interpola o atacante (sempre)
                estado->posFocoAtacante.x = posOriginalAtacante.x + (posAlvoAtacante.x - posOriginalAtacante.x) * progresso;
                estado->posFocoAtacante.y = posOriginalAtacante.y + (posAlvoAtacante.y - posOriginalAtacante.y) * progresso;
                estado->zoomFocoAtacante = zoomOriginalAtacante + (zoomAlvo - zoomOriginalAtacante) * progresso;
                
                estado->alphaOutrosPersonagens = 1.0f - progresso; 

                if (progresso >= 1.0f) {
                    estado->estadoTurno = ESTADO_ANIMACAO_ATAQUE;
                    IniciarAnimacao(&estado->animacaoEmExecucao, 
                                    estado->animParaTocar, 
                                    estado->posFocoAtacante, 
                                    estado->zoomFocoAtacante, 
                                    estado->animFlip);
                }
            }
            break;
        
        case ESTADO_ZOOM_OUT_ATAQUE:
            {
                float duracaoZoomOut = 0.3f; 
                estado->timerFoco += GetFrameTime();
                float progresso = estado->timerFoco / duracaoZoomOut;

                if (progresso > 1.0f) {
                    progresso = 1.0f;
                }

                bool ehOponenteAtacando = estado->animFlip;
                
                NoPersonagem* noAtacante = NULL;
                if (ehOponenteAtacando) {
                    noAtacante = ObterNoPorPersonagem(&estado->timeIA, estado->atacanteEmFoco);
                } else {
                    noAtacante = ObterNoPorPersonagem(&estado->timeJogador, estado->atacanteEmFoco);
                }
                
                Vector2 posOriginalAtacante;
                if (noAtacante != NULL) {
                     posOriginalAtacante = ehOponenteAtacando ? posIA[noAtacante->posicaoNoTime] : posJogador[noAtacante->posicaoNoTime];
                } else {
                    // Se o atacante morreu (ex: recuo) e foi removido, volta da posição de foco
                    posOriginalAtacante = estado->posFocoAtacante; 
                }

                float zoomOriginalAtacante = estado->atacanteEmFoco->batalhaZoom;
                Vector2 posAlvoAtacante;
                float zoomAlvo = 2.5f;
                
                // Verifica se há um alvo para o zoom
                if (estado->alvoEmFoco != NULL || estado->alvoEmFocoIdx != -1) {
                    // --- Lógica de zoom com alvo (Dano Único ou Área) ---
                    NoPersonagem* noAlvo = NULL;
                    if (ehOponenteAtacando) {
                        noAlvo = ObterNoPorPersonagem(&estado->timeJogador, estado->alvoEmFoco);
                    } else {
                        noAlvo = ObterNoPorPersonagem(&estado->timeIA, estado->alvoEmFoco);
                    }
                    
                    Vector2 posOriginalAlvo;
                    float zoomOriginalAlvo = 2.0f;
                    
                    if (noAlvo != NULL) {
                        posOriginalAlvo = ehOponenteAtacando ? posJogador[noAlvo->posicaoNoTime] : posIA[noAlvo->posicaoNoTime];
                        zoomOriginalAlvo = estado->alvoEmFoco->batalhaZoom;
                    } else {
                        posOriginalAlvo = ehOponenteAtacando ? posJogador[estado->alvoEmFocoIdx] : posIA[estado->alvoEmFocoIdx];
                    }

                    Vector2 posAlvoAlvo;

                    if (ehOponenteAtacando) {
                        posAlvoAtacante = (Vector2){ (float)SCREEN_WIDTH / 2.0f + 200.0f, 350.0f };
                        posAlvoAlvo = (Vector2){ (float)SCREEN_WIDTH / 2.0f - 200.0f, 350.0f };
                    } else {
                        posAlvoAtacante = (Vector2){ (float)SCREEN_WIDTH / 2.0f - 200.0f, 350.0f };
                        posAlvoAlvo = (Vector2){ (float)SCREEN_WIDTH / 2.0f + 200.0f, 350.0f };
                    }
                    
                    estado->posFocoAlvo.x = posAlvoAlvo.x + (posOriginalAlvo.x - posAlvoAlvo.x) * progresso;
                    estado->posFocoAlvo.y = posAlvoAlvo.y + (posOriginalAlvo.y - posAlvoAlvo.y) * progresso;
                    estado->zoomFocoAlvo = zoomAlvo + (zoomOriginalAlvo - zoomAlvo) * progresso;

                } else {
                    // --- Lógica de zoom sem alvo (Cura) ---
                     posAlvoAtacante = (Vector2){ (float)SCREEN_WIDTH / 2.0f, 350.0f }; // Foca no centro
                }

                // Interpola o atacante (sempre)
                estado->posFocoAtacante.x = posAlvoAtacante.x + (posOriginalAtacante.x - posAlvoAtacante.x) * progresso;
                estado->posFocoAtacante.y = posAlvoAtacante.y + (posOriginalAtacante.y - posAlvoAtacante.y) * progresso;
                estado->zoomFocoAtacante = zoomAlvo + (zoomOriginalAtacante - zoomAlvo) * progresso;
                
                estado->alphaOutrosPersonagens = progresso; 

                if (progresso >= 1.0f) {
                    estado->atacanteEmFoco = NULL;
                    estado->alvoEmFoco = NULL;
                    estado->alvoEmFocoIdx = -1; 
                    estado->alphaOutrosPersonagens = 1.0f;
                    
                    // Reseta o flip do oponente (caso J1 tenha atacado P2)
                    if (ehOponenteAtacando == false && estado->alvoEmFoco != NULL) {
                         // estado->alvoEmFoco->animIdle.flip = false; // (O alvo já é NULL aqui, esta lógica precisa ser revista se quiser que o P2 vire de volta)
                    }

                    
                    if (estado->timeJogador.tamanho == 0 || estado->timeIA.tamanho == 0) {
                        // Não faz nada, o loop principal vai pegar o FIM_DE_JOGO
                    } else {
                        ProximoTurno(estado);
                    }
                }
            }
            break;

        // Renomeado
        case ESTADO_AGUARDANDO_OPONENTE:
            if (modo == MODO_SOLO) {
                ExecutarTurnoIA(estado); // Chama a IA (Gemini)
            } else {
                // Modo PVP: É a vez do Jogador 2
                // Apenas muda o estado para esperar o input do P2
                estado->estadoTurno = ESTADO_ESPERANDO_JOGADOR;
                // Atualiza a mensagem para o P2
                sprintf(estado->mensagemBatalha, "Vez de: %s! (J2) Escolha um ataque.", estado->ordemDeAtaque[estado->personagemAgindoIdx]->nome);
            }
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

    // Texto do Oponente (IA ou J2)
    DrawText("Oponente", SCREEN_WIDTH - MeasureText("Oponente", 20) - 20, 15, 20, RAYWHITE);

    int arenaY = 80;
    int arenaHeight = 550; 
    DrawRectangleLines(10, arenaY, SCREEN_WIDTH - 20, arenaHeight, LIGHTGRAY);

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

    // --- Desenha personagens do JOGADOR 1 (que NÃO estão em foco) ---
    NoPersonagem* noAtualJogador = estado->timeJogador.inicio;
    while (noAtualJogador != NULL) {
        
        // Pula o desenho se este personagem for o atacante ou o alvo em foco
        if (estado->atacanteEmFoco != noAtualJogador->personagem && estado->alvoEmFoco != noAtualJogador->personagem) {
            int i = noAtualJogador->posicaoNoTime;
            PersonagemData* pData = noAtualJogador->personagem;
            AnimacaoData* anim = &pData->animIdle;

            if (anim->def.numFrames > 0) {
                // Desenha o Jogador (sem flip)
                int frameIndex = estado->idleFrameJogador[i];
                if (frameIndex >= anim->def.numFrames) {
                    frameIndex = 0;
                }
                Rectangle frame = anim->def.frames[frameIndex];
                float zoom = pData->batalhaZoom;
                
                DrawTexturePro(anim->textura, frame,
                (Rectangle){posJogador[i].x, posJogador[i].y, frame.width * zoom, frame.height * zoom},
                (Vector2){frame.width * zoom / 2, frame.height * zoom / 2}, 0, corFade);

                // Barra de HP
                int posXBarra = (int)posJogador[i].x - 50; 
                int posYBarra = (int)hpBarYFixo; 
                int larguraBarra = 100;
                int alturaBarra = 10;

                DrawRectangle(posXBarra, posYBarra, larguraBarra, alturaBarra, corBgFade);

                int larguraPreenchimento = (int)((float)larguraBarra * (float)estado->hpJogador[i] / (float)pData->hpMax);
                DrawRectangle(posXBarra, posYBarra, larguraPreenchimento, alturaBarra, corHpFadeJ);

                DrawRectangleLines(posXBarra, posYBarra, larguraBarra, alturaBarra, corBordaFade);
            }
        }
        noAtualJogador = noAtualJogador->proximo;
    }

    
    // --- Desenha personagens do OPONENTE (IA/J2) (que NÃO estão em foco) ---
    NoPersonagem* noAtualIA = estado->timeIA.inicio;
    while (noAtualIA != NULL) {
        
        // Pula o desenho se este personagem for o atacante ou o alvo em foco
         if (estado->atacanteEmFoco != noAtualIA->personagem && estado->alvoEmFoco != noAtualIA->personagem) {
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
                (Vector2){frame.width * zoomIA / 2, frame.height * zoomIA / 2}, 0, corFade);
                
                // Barra de HP
                int posXBarra = (int)posIA[i].x - 50; 
                int posYBarra = (int)hpBarYFixo; 
                int larguraBarraIA = 100;
                int alturaBarraIA = 10;

                DrawRectangle(posXBarra, posYBarra, larguraBarraIA, alturaBarraIA, corBgFade);

                int larguraPreenchimentoIA = (int)((float)larguraBarraIA * (float)estado->hpIA[i] / (float)pDataIA->hpMax);
                DrawRectangle(posXBarra, posYBarra, larguraPreenchimentoIA, alturaBarraIA, corHpFadeIA);

                DrawRectangleLines(posXBarra, posYBarra, larguraBarraIA, alturaBarraIA, corBordaFade);
            }
        }
        noAtualIA = noAtualIA->proximo;
    }


    // --- Desenha Personagens em Foco ---
    if (estado->atacanteEmFoco != NULL) {
        PersonagemData* pData = estado->atacanteEmFoco;
        bool ehOponente = estado->animFlip;
        int frameIdx = 0;
        int hpAtual = 0;
        int posAtacante = -1;

        NoPersonagem* noAtacante = ObterNoPorPersonagem(ehOponente ? &estado->timeIA : &estado->timeJogador, pData);
        if (noAtacante != NULL) {
            posAtacante = noAtacante->posicaoNoTime;
        }

        if (posAtacante != -1) { // Só desenha se o atacante for encontrado
            if (ehOponente) {
                frameIdx = estado->idleFrameIA[posAtacante];
                hpAtual = estado->hpIA[posAtacante];
            } else {
                frameIdx = estado->idleFrameJogador[posAtacante];
                hpAtual = estado->hpJogador[posAtacante];
            }

            if (pData->animIdle.def.numFrames > 0) {
                 if (frameIdx >= pData->animIdle.def.numFrames) {
                     frameIdx = 0;
                 }
            } else {
                frameIdx = 0; // Evita erro se não houver frames
            }
           
            Rectangle frame = pData->animIdle.def.frames[frameIdx];
            if (estado->animFlip == true) { 
                frame.width = -frame.width; // Flip atacante
            }
            
            // Desenha o IDLE do atacante, APENAS se a animação NÃO estiver tocando
            if (estado->animacaoEmExecucao.ativo == false) {
                 DrawTexturePro(pData->animIdle.textura, frame,
                    (Rectangle){ estado->posFocoAtacante.x, estado->posFocoAtacante.y, fabsf(frame.width) * estado->zoomFocoAtacante, frame.height * estado->zoomFocoAtacante },
                    (Vector2){ fabsf(frame.width) * estado->zoomFocoAtacante / 2, frame.height * estado->zoomFocoAtacante / 2 }, 0, WHITE);
            }
           
            // Barra de HP do Atacante
            float hpBarY = estado->posFocoAtacante.y + (fabsf(frame.height) * estado->zoomFocoAtacante / 2) + 5;
            int posXBarra = (int)estado->posFocoAtacante.x - 50;
            int larguraBarra = 100;
            int alturaBarra = 10;
            
            DrawRectangle(posXBarra, (int)hpBarY, larguraBarra, alturaBarra, DARKGRAY);
            int larguraPreenchimento = (int)((float)larguraBarra * (float)hpAtual / (float)pData->hpMax);
            DrawRectangle(posXBarra, (int)hpBarY, larguraPreenchimento, alturaBarra, ehOponente ? RED : GREEN);
            DrawRectangleLines(posXBarra, (int)hpBarY, larguraBarra, alturaBarra, LIGHTGRAY);
        }
    }
    
    if (estado->alvoEmFoco != NULL) {
        PersonagemData* pData = estado->alvoEmFoco;
        bool ehOponente = !estado->animFlip; // Alvo é o oposto do atacante
        int frameIdx = 0;
        int hpAtual = 0;
        int posAlvo = -1;

        NoPersonagem* noAlvo = ObterNoPorPersonagem(ehOponente ? &estado->timeIA : &estado->timeJogador, pData);
        
        if (noAlvo != NULL) {
            // Alvo está vivo, pega a posição do nó
            posAlvo = noAlvo->posicaoNoTime;
        } else {
            // Alvo está morto, usa o índice guardado
            posAlvo = estado->alvoEmFocoIdx;
        }

        if (posAlvo != -1) { // Só desenha se tivermos uma posição válida
            if (ehOponente) {
                frameIdx = estado->idleFrameIA[posAlvo];
                hpAtual = estado->hpIA[posAlvo];
            } else {
                frameIdx = estado->idleFrameJogador[posAlvo];
                hpAtual = estado->hpJogador[posAlvo];
            }

            // Só desenha o sprite do alvo se ele ainda estiver vivo (na lista)
            if (noAlvo != NULL) {
                if (pData->animIdle.def.numFrames > 0) {
                    if (frameIdx >= pData->animIdle.def.numFrames) {
                        frameIdx = 0;
                    }
                } else {
                    frameIdx = 0;
                }
                
                Rectangle frame = pData->animIdle.def.frames[frameIdx];
                if (ehOponente) { // Flip alvo se for Oponente
                    frame.width = -frame.width; 
                } 
                
                 DrawTexturePro(pData->animIdle.textura, frame,
                    (Rectangle){ estado->posFocoAlvo.x, estado->posFocoAlvo.y, fabsf(frame.width) * estado->zoomFocoAlvo, frame.height * estado->zoomFocoAlvo },
                    (Vector2){ fabsf(frame.width) * estado->zoomFocoAlvo / 2, frame.height * estado->zoomFocoAlvo / 2 }, 0, WHITE);
            }

            // Barra de HP do Alvo (desenha sempre, mesmo se morto, para mostrar o 0)
            float alturaFrame = 100.0f; // Altura de fallback
            if (pData->animIdle.def.numFrames > 0) {
                 alturaFrame = pData->animIdle.def.frames[0].height;
            }
           
            float hpBarY = estado->posFocoAlvo.y + (alturaFrame * estado->zoomFocoAlvo / 2) + 5;
            int posXBarra = (int)estado->posFocoAlvo.x - 50;
            int larguraBarra = 100;
            int alturaBarra = 10;
            
            DrawRectangle(posXBarra, (int)hpBarY, larguraBarra, alturaBarra, DARKGRAY);
            int larguraPreenchimento = (int)((float)larguraBarra * (float)hpAtual / (float)pData->hpMax);
            DrawRectangle(posXBarra, (int)hpBarY, larguraPreenchimento, alturaBarra, ehOponente ? RED : GREEN);
            DrawRectangleLines(posXBarra, (int)hpBarY, larguraBarra, alturaBarra, LIGHTGRAY);
        }
    }
    // --- FIM DO DESENHO EM FOCO ---

    // Desenha a animação de ataque por cima de tudo
    DesenharAnimacao(&estado->animacaoEmExecucao);
    
    // Desenha as caixas de seleção de alvo
    if (estado->estadoTurno == ESTADO_ESPERANDO_JOGADOR) {
        if (estado->ataqueSelecionado != -1) {
            
            bool ehJogador1 = (estado->turnoDe == TURNO_JOGADOR);

            if (ehJogador1) {
                // P1 está atacando: desenha caixas no Time IA/P2
                noAtualIA = estado->timeIA.inicio;
                while (noAtualIA != NULL) {
                    int i = noAtualIA->posicaoNoTime;
                    DrawRectangleLines((int)posIA[i].x - 50, (int)posIA[i].y - 50, 100, 100, YELLOW);
                    noAtualIA = noAtualIA->proximo;
                }
            } else {
                // P2 está atacando: desenha caixas no Time P1
                noAtualJogador = estado->timeJogador.inicio;
                while (noAtualJogador != NULL) {
                    int i = noAtualJogador->posicaoNoTime;
                    DrawRectangleLines((int)posJogador[i].x - 50, (int)posJogador[i].y - 50, 100, 100, YELLOW);
                    noAtualJogador = noAtualJogador->proximo;
                }
            }
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

    // Só mostra o menu de ataque se for o turno do jogador E ele não estiver no meio de um zoom
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
            DrawText(atacante->ataque1.nome, (int)btnAtk1.x + ((int)btnAtk1.width - MeasureText(atacante->ataque1.nome, 20)) / 2, (int)btnAtk1.y + 10, 20, corTexto);

            DrawRectangleRounded(btnAtk2, 0.2f, 4, corAtk2);
            DrawRectangleRoundedLinesEx(btnAtk2, 0.2f, 4, espessuraBorda, BLACK);
            DrawText(atacante->ataque2.nome, (int)btnAtk2.x + ((int)btnAtk2.width - MeasureText(atacante->ataque2.nome, 20)) / 2, (int)btnAtk2.y + 10, 20, corTexto);
        
            Vector2 mousePos = GetMouseVirtual();
            if (CheckCollisionPointRec(mousePos, btnAtk1)) {
                DrawText(atacante->ataque1.descricao, colSpecsX, textoYBase + 40, 20, RAYWHITE);
                
                if (atacante->ataque1.tipo == TIPO_CURA_SI) {
                    DrawText(TextFormat("Cura %d de PV.", atacante->ataque1.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
                } else if (atacante->ataque1.tipo == TIPO_DANO_AREA) {
                    DrawText(TextFormat("Causa %d de Dano em Área.", atacante->ataque1.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
                } else {
                    DrawText(TextFormat("Causa %d de Dano.", atacante->ataque1.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
                }
                
            } else {
                // MODIFICADO: Atualiza texto de descrição do Ataque 2
                if (CheckCollisionPointRec(mousePos, btnAtk2)) {
                    DrawText(atacante->ataque2.descricao, colSpecsX, textoYBase + 40, 20, RAYWHITE);
                    
                    if (atacante->ataque2.tipo == TIPO_CURA_SI) {
                        DrawText(TextFormat("Cura %d de PV.", atacante->ataque2.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
                    } else if (atacante->ataque2.tipo == TIPO_DANO_AREA) {
                        DrawText(TextFormat("Causa %d de Dano em Área.", atacante->ataque2.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
                    } else {
                        DrawText(TextFormat("Causa %d de Dano.", atacante->ataque2.dano), colSpecsX, textoYBase + 70, 20, RAYWHITE);
                    }
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