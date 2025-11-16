#include "batalha.h"
#include "database.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "ConsumoAPI_Gemini.h" 
#include "math.h"
#include <stdbool.h> 
#include "telas.h"


// --- Variáveis Globais (Definição) ---
// Elas "moram" aqui. 'batalha_desenho.c' irá acessá-las via 'extern'.
Texture2D backgroundArena;
int backgroundCarregado = 0;
// ------------------------------------


static const float DURACAO_PISCAR_DANO = 0.3f; 

static AnimacaoData g_animLapide;

#define MAX_TEXTOS_FLUTUANTES 10 
TextoFlutuante g_textosFlutuantes[MAX_TEXTOS_FLUTUANTES];
static float DURACAO_TEXTO_FLUTUANTE = 2.5f;


static Vector2 posJogador[3];
static Vector2 posIA[3];
static int animVelocidadeIdle = 15; 


// --- Funções static de Lógica ---

NoPersonagem* ObterNoPorPersonagem(ListaTime* lista, PersonagemData* p) {
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
    return NULL; 
}

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

static void IniciarTextoFlutuante(const char* texto, Vector2 pos, Color cor) {
    for (int i = 0; i < MAX_TEXTOS_FLUTUANTES; i++) {
        if (g_textosFlutuantes[i].ativo == false) {
            g_textosFlutuantes[i].ativo = true;
            strncpy(g_textosFlutuantes[i].texto, texto, 15);
            g_textosFlutuantes[i].texto[15] = '\0';
            
            // Centraliza o texto onde ele aparece
            g_textosFlutuantes[i].pos = pos;
            g_textosFlutuantes[i].pos.x -= MeasureText(texto, 20) / 2; 
            
            g_textosFlutuantes[i].timer = 0.0f;
            g_textosFlutuantes[i].duracao = DURACAO_TEXTO_FLUTUANTE;
            g_textosFlutuantes[i].cor = cor;
            g_textosFlutuantes[i].velocidadeY = -50.0f; // Sobe (pixels/seg)
            
            return;
        }
    }
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

static void AtualizarAnimacaoLapide(EstadoAnimacao* anim) {
    if (anim->ativo == false || anim->anim == NULL) {
        return;
    }
    
    int ultimoFrame = anim->anim->def.numFrames - 1;

    // Se já chegou no último frame, não faz nada (fica parado)
    if (anim->frameAtual >= ultimoFrame) {
        anim->frameAtual = ultimoFrame; 
        return; // Para a animação no último frame
    }
    
    anim->timer++;
    if (anim->timer >= anim->velocidade) {
        anim->timer = 0;
        anim->frameAtual++;
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

    if (alvo != NULL) {
        // Se for um ataque em área, o flip será tratado mais abaixo
        if (ataque->tipo != TIPO_DANO_AREA)
        {
            if (ehJogadorAtacando == false) {
                // Oponente (IA/J2) ataca -> alvo (Jogador 1) vira para a DIREITA
                alvo->animIdle.flip = false;
            } else {
                // Jogador 1 ataca -> alvo (Oponente) vira para a ESQUERDA
                alvo->animIdle.flip = true; 
            }
        }
    }

    else if (alvo != NULL && ataque->tipo == TIPO_DANO_AREA)
    {
        ListaTime* listaAlvo;
        bool flipAlvoFinal;

        if (ehJogadorAtacando == false)
        {
            // Oponente ataca -> alvos (Time Jogador) viram para a DIREITA
            listaAlvo = &estado->timeJogador;
            flipAlvoFinal = false;
        }
        else
        {
            // Jogador ataca -> alvos (Time Oponente) viram para a ESQUERDA
            listaAlvo = &estado->timeIA;
            flipAlvoFinal = true;
        }

        // Percorre todos os nós da lista alvo e aplica o flip
        NoPersonagem* noAlvo = listaAlvo->inicio;
        while (noAlvo != NULL)
        {
            if (noAlvo->personagem != NULL)
            {
                noAlvo->personagem->animIdle.flip = flipAlvoFinal;
            }
            noAlvo = noAlvo->proximo;
        }
    }


    if (strcmp(ataque->nome, atacante->ataque1.nome) == 0) {
        estado->animParaTocar = &atacante->animAtaque1;
    } else {
        estado->animParaTocar = &atacante->animAtaque2;
    }

    if (ataque->tipo == TIPO_DANO_AREA) {
        estado->isZoomAoe = true;
    } else {
        estado->isZoomAoe = false;
    }

    // Define o próximo estado como o Zoom In
    estado->estadoTurno = ESTADO_ZOOM_IN_ATAQUE;
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

    estado->numMortosPendentes = 0;

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

            char textoDano[16];
            sprintf(textoDano, "-%d", dano);
            Vector2 posAlvoDano = {0};

            if (ehJogadorAtacando) {
                posAlvoDano = posIA[alvoIdx];
            } else {
                posAlvoDano = posJogador[alvoIdx];
            }

            if (ehJogadorAtacando) {
                estado->timerDanoIA[alvoIdx] = DURACAO_PISCAR_DANO;
            } else {
                estado->timerDanoJogador[alvoIdx] = DURACAO_PISCAR_DANO;
            }

            posAlvoDano.y -= 50; // Começa acima da cabeça
            IniciarTextoFlutuante(textoDano, posAlvoDano, RED);

            sprintf(estado->mensagemBatalha, "%s usou %s em %s e causou %d de dano!", atacante->nome, ataque->nome, alvo->nome, dano);
            
            if (hpArrayAlvo[alvoIdx] <= 0) {
                hpArrayAlvo[alvoIdx] = 0;

                estado->noParaRemover[estado->numMortosPendentes] = noAlvo;
                estado->numMortosPendentes++;
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

                    char textoDanoArea[16];
                    sprintf(textoDanoArea, "-%d", dano);
                    Vector2 posAlvoArea = {0};
                    
                    if (ehJogadorAtacando) {
                        posAlvoArea = posIA[i]; // Jogador ataca IA
                    } else {
                        posAlvoArea = posJogador[i]; // IA ataca Jogador
                    }

                    if (ehJogadorAtacando) {
                        estado->timerDanoIA[i] = DURACAO_PISCAR_DANO;
                    } else {
                        estado->timerDanoJogador[i] = DURACAO_PISCAR_DANO;
                    }
                    
                    posAlvoArea.y -= 50; // Começa acima da cabeça
                    IniciarTextoFlutuante(textoDanoArea, posAlvoArea, RED);

                    if (hpArrayAlvo[i] <= 0) {
                        hpArrayAlvo[i] = 0;
                        estado->noParaRemover[estado->numMortosPendentes] = noAlvo; 
                        estado->numMortosPendentes++;
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
                
                char textoCura[16];
                sprintf(textoCura, "+%d", cura);
                Vector2 posAlvoCura;

                if (ehJogadorAtacando) {
                    posAlvoCura = posJogador[posAtacante]; // Jogador cura Jogador
                } else {
                    posAlvoCura = posIA[posAtacante]; // IA cura IA
                }

                posAlvoCura.y -= 50; // Começa acima da cabeça
                IniciarTextoFlutuante(textoCura, posAlvoCura, GREEN);

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


// --- Funções da Máquina de Estados (Refatoradas) ---

static void AtualizarEstadoEsperandoJogador(EstadoBatalha* estado) {
    bool ehJogador1 = (estado->turnoDe == TURNO_JOGADOR);

    int arenaY = 80;
    int arenaHeight = 550; 
    int menuY = arenaY + arenaHeight + 10; 
    int textoYBase = menuY + 20;           
    int colAtaquesX = 35;

    Rectangle btnAtk1 = { colAtaquesX, textoYBase + 35, 200, 40 }; 
    Rectangle btnAtk2 = { colAtaquesX, textoYBase + 90, 200, 40 }; 
    
    Rectangle alvo_0, alvo_1, alvo_2;
    ListaTime* listaAlvo;
    
    if (ehJogador1) {
        alvo_0 = (Rectangle){posIA[0].x - 50, posIA[0].y - 50, 100, 100};
        alvo_1 = (Rectangle){posIA[1].x - 50, posIA[1].y - 50, 100, 100};
        alvo_2 = (Rectangle){posIA[2].x - 50, posIA[2].y - 50, 100, 100};
        listaAlvo = &estado->timeIA;
    } else {
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
                if (ObterNoNaPosicao(listaAlvo, 0) != NULL) alvo = 0;
            }
            if (CheckCollisionPointRec(mousePos, alvo_1)) {
                    if (ObterNoNaPosicao(listaAlvo, 1) != NULL) alvo = 1;
            }
            if (CheckCollisionPointRec(mousePos, alvo_2)) {
                if (ObterNoNaPosicao(listaAlvo, 2) != NULL) alvo = 2;
            }
            
            if (alvo != -1) {
                estado->alvoSelecionado = alvo; 
                PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
                Ataque* att;
                if (estado->ataqueSelecionado == 0) att = &atacante->ataque1;
                else att = &atacante->ataque2;
                
                ExecutarAtaque(estado, atacante, att, alvo, ehJogador1);
            }
        }
    }
}

static void AtualizarEstadoZoomIn(EstadoBatalha* estado) {
    float duracaoZoom = 0.3f; 
    estado->timerFoco += GetFrameTime();
    float progresso = estado->timerFoco / duracaoZoom;
    if (progresso > 1.0f) progresso = 1.0f;

    bool ehOponenteAtacando = estado->animFlip;
    NoPersonagem* noAtacante = NULL;
    if (ehOponenteAtacando) noAtacante = ObterNoPorPersonagem(&estado->timeIA, estado->atacanteEmFoco);
    else noAtacante = ObterNoPorPersonagem(&estado->timeJogador, estado->atacanteEmFoco);
    
    Vector2 posOriginalAtacante;
    if (noAtacante != NULL) posOriginalAtacante = ehOponenteAtacando ? posIA[noAtacante->posicaoNoTime] : posJogador[noAtacante->posicaoNoTime];
    else posOriginalAtacante = (Vector2){ -200.0f, 450.0f }; // Fallback (usando 450)
    
    Vector2 posAlvoAtacante; // Declarada aqui
    float zoomOriginalAtacante = estado->atacanteEmFoco->batalhaZoom;
    float zoomAlvo = 2.5f; 

    if (estado->alvoEmFoco != NULL || estado->alvoEmFocoIdx != -1) {
        NoPersonagem* noAlvo = NULL;
        if (ehOponenteAtacando) noAlvo = ObterNoPorPersonagem(&estado->timeJogador, estado->alvoEmFoco);
        else noAlvo = ObterNoPorPersonagem(&estado->timeIA, estado->alvoEmFoco);
        
        Vector2 posOriginalAlvo;
        float zoomOriginalAlvo = 2.0f;
        
        if (noAlvo != NULL) {
                posOriginalAlvo = ehOponenteAtacando ? posJogador[noAlvo->posicaoNoTime] : posIA[noAlvo->posicaoNoTime];
                zoomOriginalAlvo = estado->alvoEmFoco->batalhaZoom;
        } else {
                posOriginalAlvo = ehOponenteAtacando ? posJogador[estado->alvoEmFocoIdx] : posIA[estado->alvoEmFocoIdx];
        }

        Vector2 posAlvoAlvo; // Declarada aqui
        
        if (ehOponenteAtacando) {
            posAlvoAtacante = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f + 200.0f, .y = posOriginalAtacante.y };
            posAlvoAlvo = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f - 200.0f, .y = posOriginalAlvo.y };
        } else {
            posAlvoAtacante = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f - 200.0f, .y = posOriginalAtacante.y };
            posAlvoAlvo = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f + 200.0f, .y = posOriginalAlvo.y };
        }

        if (estado->isZoomAoe == true) {
            posAlvoAlvo = posOriginalAlvo;
        }
        
        estado->posFocoAlvo.x = posOriginalAlvo.x + (posAlvoAlvo.x - posOriginalAlvo.x) * progresso;
        estado->posFocoAlvo.y = posOriginalAlvo.y + (posAlvoAlvo.y - posOriginalAlvo.y) * progresso;
        estado->zoomFocoAlvo = zoomOriginalAlvo + (zoomAlvo - zoomOriginalAlvo) * progresso;

    } else {
        posAlvoAtacante = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f, .y = posOriginalAtacante.y };
    }

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

static void AtualizarEstadoZoomOut(EstadoBatalha* estado) {
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
        posOriginalAtacante = estado->posFocoAtacante; 
    }

    float zoomOriginalAtacante = estado->atacanteEmFoco->batalhaZoom;
    Vector2 posAlvoAtacante; // Declarada aqui
    float zoomAlvo = 2.5f;
    
    if (estado->alvoEmFoco != NULL || estado->alvoEmFocoIdx != -1) {
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

        Vector2 posAlvoAlvo; // Declarada aqui

        if (ehOponenteAtacando) {
            posAlvoAtacante = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f + 200.0f, .y = posOriginalAtacante.y };
            posAlvoAlvo = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f - 200.0f, .y = posOriginalAlvo.y };
        } else {
            posAlvoAtacante = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f - 200.0f, .y = posOriginalAtacante.y };
            posAlvoAlvo = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f + 200.0f, .y = posOriginalAlvo.y };
        }

        if (estado->isZoomAoe == true) {
            posAlvoAlvo = posOriginalAlvo;
        }
        
        estado->posFocoAlvo.x = posAlvoAlvo.x + (posOriginalAlvo.x - posAlvoAlvo.x) * progresso;
        estado->posFocoAlvo.y = posAlvoAlvo.y + (posOriginalAlvo.y - posAlvoAlvo.y) * progresso;
        estado->zoomFocoAlvo = zoomAlvo + (zoomOriginalAlvo - zoomAlvo) * progresso;

    } else {
            posAlvoAtacante = (Vector2){ .x = (float)SCREEN_WIDTH / 2.0f, .y = posOriginalAtacante.y };
    }

    estado->posFocoAtacante.x = posAlvoAtacante.x + (posOriginalAtacante.x - posAlvoAtacante.x) * progresso;
    estado->posFocoAtacante.y = posAlvoAtacante.y + (posOriginalAtacante.y - posAlvoAtacante.y) * progresso;
    estado->zoomFocoAtacante = zoomAlvo + (zoomOriginalAtacante - zoomAlvo) * progresso;
    
    estado->alphaOutrosPersonagens = progresso; 
    
    if (progresso >= 1.0f) {
        
        // --- CORREÇÃO: Reverte o flip do alvo ---
        if (estado->animFlip == true) // Oponente (IA/J2) atacou
        {
            // Reseta o flip do Time do Jogador (alvo) de volta para 'false'
            if (estado->isZoomAoe == true) 
            {
                NoPersonagem* noAlvo = estado->timeJogador.inicio;
                while (noAlvo != NULL) 
                {
                    if (noAlvo->personagem != NULL) {
                        noAlvo->personagem->animIdle.flip = false;
                    }
                    noAlvo = noAlvo->proximo;
                }
            } 
            else if (estado->alvoEmFoco != NULL) 
            {
                estado->alvoEmFoco->animIdle.flip = false;
            }
        }
        else // Jogador 1 atacou
        {
            // Reseta o flip do Time do Oponente (alvo) de volta para 'true'
                if (estado->isZoomAoe == true) 
            {
                NoPersonagem* noAlvo = estado->timeIA.inicio;
                while (noAlvo != NULL) 
                {
                    if (noAlvo->personagem != NULL) {
                        noAlvo->personagem->animIdle.flip = true;
                    }
                    noAlvo = noAlvo->proximo;
                }
            } 
            else if (estado->alvoEmFoco != NULL) 
            {
                estado->alvoEmFoco->animIdle.flip = true;
            }
        }
        
        // Processa mortes pendentes DEPOIS do zoom out
        if (estado->numMortosPendentes > 0) {
            for (int i = 0; i < estado->numMortosPendentes; i++) {
                NoPersonagem* noMorreu = estado->noParaRemover[i];
                
                if (noMorreu != NULL) {
                    int pos = noMorreu->posicaoNoTime;
                    
                    // Tenta remover da lista do Jogador
                    if (ObterNoPorPersonagem(&estado->timeJogador, noMorreu->personagem) != NULL) {
                        
                        IniciarAnimacao(&estado->animLapideJogador[pos], &g_animLapide, posJogador[pos], 0.8f, false);
                        RemoverPersonagem(&estado->timeJogador, noMorreu);
                    } 
                    // Tenta remover da lista da IA
                    else if (ObterNoPorPersonagem(&estado->timeIA, noMorreu->personagem) != NULL) {
                        
                        IniciarAnimacao(&estado->animLapideIA[pos], &g_animLapide, posIA[pos], 0.8f, true);
                        RemoverPersonagem(&estado->timeIA, noMorreu);
                    }
                    
                    estado->noParaRemover[i] = NULL; 
                }
            }
            estado->numMortosPendentes = 0;
        }

        // Limpa o foco e as variáveis da animação
        estado->atacanteEmFoco = NULL;
        estado->alvoEmFoco = NULL;
        estado->alvoEmFocoIdx = -1; 
        estado->alphaOutrosPersonagens = 1.0f;
        estado->isZoomAoe = false;
        
        // Verifica se o jogo acabou AGORA
        if (estado->timeJogador.tamanho == 0) {
            estado->estadoTurno = ESTADO_FIM_DE_JOGO;
            estado->resultadoBatalha = RESULTADO_DERROTA;
            sprintf(estado->mensagemBatalha, "OPONENTE VENCEU! Pressione ESC para sair.");
        } else if (estado->timeIA.tamanho == 0) {
            estado->estadoTurno = ESTADO_FIM_DE_JOGO;
            estado->resultadoBatalha = RESULTADO_VITORIA;
            sprintf(estado->mensagemBatalha, "JOGADOR 1 VENCEU! Pressione ESC para sair.");
        } else {
            ProximoTurno(estado);
        }
    }
}

static void AtualizarEstadoIAPensando(EstadoBatalha* estado, ModoDeJogo modo) {
    // Este case agora lida com os dois estados de "turno do oponente"
    
    if (estado->estadoTurno == ESTADO_AGUARDANDO_OPONENTE) {
        if (modo == MODO_SOLO) {
            // Modo Solo: Inicia a thread da IA (ou usa ataque aleatório)
            
            // A função de carregar a chave foi movida para 'consumoAPI_Gemini.c'
            // mas a lógica de ataque aleatório depende dela, então
            // temos que chamá-la de lá.
            // Para simplificar, vamos assumir que 'IA_IniciarDecisao' 
            // lida com a falha da chave.
            
            IA_IniciarDecisao(estado, "config.txt"); // Passa o *caminho* do arquivo
            estado->estadoTurno = ESTADO_IA_PENSANDO;
            
            PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
            if (atacante != NULL) {
                    sprintf(estado->mensagemBatalha, "%s está pensando...", atacante->nome);
            }
        
        } else {
            // Modo PVP: Apenas muda o estado para o Jogador 2 agir
            estado->estadoTurno = ESTADO_ESPERANDO_JOGADOR;
            
            PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
            if (atacante != NULL) {
                sprintf(estado->mensagemBatalha, "Vez de: %s! (J2) Escolha um ataque.", atacante->nome);
            }
        }
        return; // Sai da função neste frame
    }
    
    
    if (estado->estadoTurno == ESTADO_IA_PENSANDO) {
        // Este estado não faz nada além de esperar (polir)
        DecisaoIA decisaoDaIA;
        bool estaPronta = IA_VerificarDecisaoPronta(&decisaoDaIA);
        
        if (estaPronta == true) {
            
            // Se a justificativa for "CHAVE_NAO_ENCONTRADA", usa lógica aleatória
            if (decisaoDaIA.justificativa != NULL && strcmp(decisaoDaIA.justificativa, "CHAVE_NAO_ENCONTRADA") == 0)
            {
                printf("IA: ERRO! Chave da API nao encontrada. Pulando turno com ataque aleatório.\n");
                PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
                int alvoAleatorio = rand() % 3;
                int tentativas = 0;
                NoPersonagem* noAlvo = ObterNoNaPosicao(&estado->timeJogador, alvoAleatorio);
                
                while (noAlvo == NULL && tentativas < 3) {
                        alvoAleatorio = (alvoAleatorio + 1) % 3;
                        noAlvo = ObterNoNaPosicao(&estado->timeJogador, alvoAleatorio);
                        tentativas++;
                }
                if (noAlvo == NULL) {
                    alvoAleatorio = 0; 
                }
                
                ExecutarAtaque(estado, atacante, &atacante->ataque1, alvoAleatorio, false);

            }
            else 
            {
                // Lógica normal da IA
                printf("IA (Main): Decisão recebida da thread!\n");
                
                PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
                Ataque* ataqueEscolhido;
                
                if (decisaoDaIA.indiceAtaque == 0) {
                    ataqueEscolhido = &atacante->ataque1;
                } else {
                    ataqueEscolhido = &atacante->ataque2;
                }

                int alvoFinal = decisaoDaIA.indiceAlvo;
                
                NoPersonagem* noAlvoEscolhido = ObterNoNaPosicao(&estado->timeJogador, alvoFinal);
                
                if (noAlvoEscolhido == NULL) { 
                    for (int i = 0; i < 3; i++) {
                        NoPersonagem* noAlvoVivo = ObterNoNaPosicao(&estado->timeJogador, i);
                        if (noAlvoVivo != NULL) {
                            alvoFinal = i;
                            break;
                        }
                    }
                }
                ExecutarAtaque(estado, atacante, ataqueEscolhido, alvoFinal, false);
            }
            
            LiberarDecisaoIA(&decisaoDaIA);
        }
        
        // Se 'estaPronta' for 'false', o loop simplesmente continua.
    }
}


// --- Funções Públicas (Interface) ---

void InicializarBatalha(EstadoBatalha *estado, TimesBatalha* timesSelecionados) {
    printf("INICIALIZANDO BATALHA!\n");

    if (backgroundCarregado == 0) {
        backgroundArena = LoadTexture("sprites/background/backgroundbatalha.png");
        backgroundCarregado = 1;
    }

    g_animLapide = CarregarAnimacaoData("sprites/personagens/lapide/lapide");

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
    
    NoPersonagem* noAtualIA = estado->timeIA.inicio;

    while (noAtualIA != NULL)
    {
        if (noAtualIA->personagem != NULL)
        {
            // Define o flip da animação IDLE para true
            noAtualIA->personagem->animIdle.flip = true; 
        }
        noAtualIA = noAtualIA->proximo;
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
        estado->timerDanoJogador[i] = 0.0f;
        estado->timerDanoIA[i] = 0.0f;
        PararAnimacao(&estado->animLapideJogador[i]);
        PararAnimacao(&estado->animLapideIA[i]);
    }


    // Inicializa as novas variáveis de foco
    estado->atacanteEmFoco = NULL;
    estado->alvoEmFoco = NULL;
    estado->alvoEmFocoIdx = -1; 
    estado->timerFoco = 0.0f;
    estado->animParaTocar = NULL;
    estado->animFlip = false;
    estado->alphaOutrosPersonagens = 1.0f;

    estado->numMortosPendentes = 0;
    for (int i = 0; i < 6; i++) {
        estado->noParaRemover[i] = NULL;
    }

    for (int i = 0; i < 6; i++) {
        estado->noParaRemover[i] = NULL;
    }

    for (int i = 0; i < MAX_TEXTOS_FLUTUANTES; i++) {
        g_textosFlutuantes[i].ativo = false;
    }
    estado->resultadoBatalha = RESULTADO_EM_JOGO;
}


void AtualizarTelaBatalha(EstadoBatalha *estado, GameScreen *telaAtual, ModoDeJogo modo) {
    if (IsKeyPressed(KEY_ESCAPE)) {

        if (backgroundCarregado == 1) {
            UnloadTexture(backgroundArena);
            backgroundCarregado = 0;
        }

        LiberarAnimacaoData(&g_animLapide);

        *telaAtual = SCREEN_MENU;
        LiberarLista(&estado->timeJogador);
        LiberarLista(&estado->timeIA);
        return;
    }

    float dt = GetFrameTime();
    for (int i = 0; i < 3; i++) {
        if (estado->timerDanoJogador[i] > 0.0f) {
            estado->timerDanoJogador[i] -= dt;
        }
        if (estado->timerDanoIA[i] > 0.0f) {
            estado->timerDanoIA[i] -= dt;
        }
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

    for (int i = 0; i < 3; i++) {
        AtualizarAnimacaoLapide(&estado->animLapideJogador[i]);
        AtualizarAnimacaoLapide(&estado->animLapideIA[i]);
    }

    for (int i = 0; i < MAX_TEXTOS_FLUTUANTES; i++) {
        if (g_textosFlutuantes[i].ativo) {
            g_textosFlutuantes[i].timer += dt;
            if (g_textosFlutuantes[i].timer >= g_textosFlutuantes[i].duracao) {
                g_textosFlutuantes[i].ativo = false; // Desativa
            } else {
                g_textosFlutuantes[i].pos.y += g_textosFlutuantes[i].velocidadeY * dt;
            }
        }
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

    // --- CORREÇÃO AQUI: Só checa Fim de Jogo se NÃO estiver no meio de uma animação de ataque ---
    bool emAnimacaoAtaque = false;
    if (estado->estadoTurno == ESTADO_ZOOM_IN_ATAQUE) emAnimacaoAtaque = true;
    if (estado->estadoTurno == ESTADO_ANIMACAO_ATAQUE) emAnimacaoAtaque = true;
    if (estado->estadoTurno == ESTADO_ZOOM_OUT_ATAQUE) emAnimacaoAtaque = true;


    if (emAnimacaoAtaque == false) {
        if (estado->timeJogador.tamanho == 0 && estado->estadoTurno != ESTADO_FIM_DE_JOGO) {
            estado->estadoTurno = ESTADO_FIM_DE_JOGO;
            estado->resultadoBatalha = RESULTADO_DERROTA;
            sprintf(estado->mensagemBatalha, "OPONENTE VENCEU! Pressione ESC para sair.");
        } else if (estado->timeIA.tamanho == 0 && estado->estadoTurno != ESTADO_FIM_DE_JOGO) {
            estado->estadoTurno = ESTADO_FIM_DE_JOGO;
            estado->resultadoBatalha = RESULTADO_VITORIA;
            sprintf(estado->mensagemBatalha, "JOGADOR 1 VENCEU! Pressione ESC para sair.");
        }
    }


    // --- MÁQUINA DE ESTADOS (Refatorada) ---
    switch (estado->estadoTurno) {
        
        case ESTADO_INICIANDO:
            ProximoTurno(estado);
            break;

        case ESTADO_ESPERANDO_JOGADOR:
            AtualizarEstadoEsperandoJogador(estado);
            break;
            
        case ESTADO_ANIMACAO_ATAQUE:
            if (estado->animacaoEmExecucao.ativo == false) {
                estado->estadoTurno = ESTADO_ZOOM_OUT_ATAQUE;
                estado->timerFoco = 0.0f; 
            }
            break;

        case ESTADO_ZOOM_IN_ATAQUE:
            AtualizarEstadoZoomIn(estado);
            break;
        
        case ESTADO_ZOOM_OUT_ATAQUE:
            AtualizarEstadoZoomOut(estado);
            break;

        case ESTADO_AGUARDANDO_OPONENTE:
        case ESTADO_IA_PENSANDO:
            AtualizarEstadoIAPensando(estado, modo);
            break;
            
        case ESTADO_FIM_DE_JOGO:
            break;
    }
}