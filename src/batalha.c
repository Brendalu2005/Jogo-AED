#include "batalha.h"
#include "database.h"
#include <stdio.h>  // Para printf
#include <stdlib.h> // Para qsort e rand
#include <string.h> // Para sprintf e strcmp

// --- Variáveis de Posição ---
static Vector2 posJogador[3];
static Vector2 posIA[3];

// --- Funções da Máquina de Estados de Animação ---
static void PararAnimacao(EstadoAnimacao* anim) {
    anim->ativo = false;
    anim->frameAtual = 0;
    anim->timer = 0;
}

static void IniciarAnimacao(EstadoAnimacao* anim, AnimacaoData* data, Vector2 pos, float zoom) {
    anim->anim = data;
    anim->pos = pos;
    anim->zoom = zoom;
    anim->ativo = true;
    anim->frameAtual = 0;
    anim->timer = 0;
    anim->velocidade = 8;
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
    DrawTexturePro(anim->anim->textura, 
                   frameRec, 
                   (Rectangle){ anim->pos.x, anim->pos.y, frameRec.width * anim->zoom, frameRec.height * anim->zoom },
                   (Vector2){ frameRec.width * anim->zoom / 2, frameRec.height * anim->zoom / 2 }, 
                   0.0f, 
                   WHITE);
}

// --- Lógica de Turno e IA ---

// Função de comparação para qsort (ordena por velocidade, decrescente)
static int CompararVelocidade(const void* a, const void* b) {
    PersonagemData* pA = *(PersonagemData**)a;
    PersonagemData* pB = *(PersonagemData**)b;
    
    if (pA == NULL || pB == NULL) return 0; // Guarda de segurança
    
    if (pA->velocidade > pB->velocidade) return -1;
    if (pA->velocidade < pB->velocidade) return 1;
    // Se a velocidade for igual, desempate aleatoriamente
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

    if (ehJogadorAtacando) {
        alvo = estado->times.timeIA[alvoIdx];
        estado->hpIA[alvoIdx] -= dano;
        sprintf(estado->mensagemBatalha, "%s usou %s em %s e causou %d de dano!", atacante->nome, ataque->nome, alvo->nome, dano);
        if (estado->hpIA[alvoIdx] <= 0) {
            estado->hpIA[alvoIdx] = 0;
            // (Adicionar lógica de morte aqui)
        }
    } else {
        alvo = estado->times.timeJogador[alvoIdx];
        estado->hpJogador[alvoIdx] -= dano;
        sprintf(estado->mensagemBatalha, "%s usou %s em %s e causou %d de dano!", atacante->nome, ataque->nome, alvo->nome, dano);
        if (estado->hpJogador[alvoIdx] <= 0) {
            estado->hpJogador[alvoIdx] = 0;
            // (Adicionar lógica de morte aqui)
        }
    }

    // Inicia a animação de ataque
    if (strcmp(ataque->nome, atacante->ataque1.nome) == 0) {
        IniciarAnimacao(&estado->animacaoEmExecucao, &atacante->animAtaque1, (ehJogadorAtacando ? posJogador[0] : posIA[0]), 2.5f);
    } else {
        IniciarAnimacao(&estado->animacaoEmExecucao, &atacante->animAtaque2, (ehJogadorAtacando ? posJogador[0] : posIA[0]), 2.5f);
    }

    estado->estadoTurno = ESTADO_ANIMACAO_ATAQUE;
}

// A "IA"
static void ExecutarTurnoIA(EstadoBatalha *estado) {
    PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
    
    // 1. Escolher um ataque aleatório (0 ou 1)
    int escolhaAtaque = rand() % 2;
    Ataque* ataqueEscolhido = (escolhaAtaque == 0) ? &atacante->ataque1 : &atacante->ataque2;

    // 2. Escolher um alvo aleatório (0, 1, ou 2) que esteja vivo
    int alvoEscolhido;
    do {
        alvoEscolhido = rand() % 3;
    } while (estado->hpJogador[alvoEscolhido] <= 0); // Procura um alvo vivo

    ExecutarAtaque(estado, atacante, ataqueEscolhido, alvoEscolhido, false);
}

// --- Funções Principais da Tela ---

// --- CORREÇÃO 3: Removido o parâmetro 'db' ---
void InicializarBatalha(EstadoBatalha *estado, TimesBatalha* timesSelecionados) {
    printf("INICIALIZANDO BATALHA!\n");
    
    // 1. Copia os times
    estado->times = *timesSelecionados;
    
    // 2. Define o HP inicial
    for (int i = 0; i < 3; i++) {
        if (estado->times.timeJogador[i] == NULL || estado->times.timeIA[i] == NULL) {
            printf("ERRO FATAL: Time nao selecionado corretamente!\n");
            return; // Impede o crash
        }
        estado->hpJogador[i] = estado->times.timeJogador[i]->hpMax;
        estado->hpIA[i] = estado->times.timeIA[i]->hpMax;
    }
    
    // 3. Define as posições de desenho
    posJogador[0] = (Vector2){400, 300};
    posJogador[1] = (Vector2){300, 400};
    posJogador[2] = (Vector2){200, 500};
    posIA[0] = (Vector2){1200, 300};
    posIA[1] = (Vector2){1300, 400};
    posIA[2] = (Vector2){1400, 500};
    
    // 4. Cria e ordena a fila de ataque
    for (int i = 0; i < 3; i++) {
        estado->ordemDeAtaque[i] = estado->times.timeJogador[i];
        estado->ordemDeAtaque[i+3] = estado->times.timeIA[i];
    }
    qsort(estado->ordemDeAtaque, 6, sizeof(PersonagemData*), CompararVelocidade);

    printf("Ordem de ataque:\n");
    for(int i=0; i<6; i++) {
        if (estado->ordemDeAtaque[i] != NULL) { // Guarda de segurança
            printf("  %d. %s (Vel: %d)\n", i+1, estado->ordemDeAtaque[i]->nome, estado->ordemDeAtaque[i]->velocidade);
        }
    }
    
    // 5. Configura o primeiro turno
    estado->personagemAgindoIdx = -1; // Começa em -1 para que o ProximoTurno() comece no 0
    estado->estadoTurno = ESTADO_INICIANDO;
    PararAnimacao(&estado->animacaoEmExecucao);
}

void AtualizarTelaBatalha(EstadoBatalha *estado, GameScreen *telaAtual) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        *telaAtual = SCREEN_MENU;
        return; 
    }
    
    // Guarda de segurança
    if (estado->estadoTurno != ESTADO_INICIANDO && (estado->personagemAgindoIdx < 0 || estado->personagemAgindoIdx >= 6 || estado->ordemDeAtaque[estado->personagemAgindoIdx] == NULL)) {
        if(estado->estadoTurno != ESTADO_FIM_DE_JOGO) ProximoTurno(estado);
        return;
    }

    // Atualiza a animação que estiver tocando
    AtualizarAnimacao(&estado->animacaoEmExecucao);

    // Lógica da Máquina de Estados de Batalha
    switch (estado->estadoTurno) {
        
        case ESTADO_INICIANDO:
            ProximoTurno(estado);
            break;

        case ESTADO_ESPERANDO_JOGADOR:
            {
                // Pega os dados do personagem que está agindo
                PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
                int menuY = 80 + 450 + 10;
                int textoYBase = menuY + 20;
                int colAtaquesX = 35;

                // Hitboxes dos botões de ataque
                Rectangle btnAtk1 = { colAtaquesX, textoYBase + 35, 200, 40 };
                Rectangle btnAtk2 = { colAtaquesX, textoYBase + 90, 200, 40 };
                
                // Hitboxes dos alvos (IA)
                Rectangle alvoIA_0 = {posIA[0].x - 50, posIA[0].y - 50, 100, 100};
                Rectangle alvoIA_1 = {posIA[1].x - 50, posIA[1].y - 50, 100, 100};
                Rectangle alvoIA_2 = {posIA[2].x - 50, posIA[2].y - 50, 100, 100};
                
                Vector2 mousePos = GetMousePosition();

                // 1. Jogador escolhe o ATAQUE
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mousePos, btnAtk1)) estado->ataqueSelecionado = 0;
                    if (CheckCollisionPointRec(mousePos, btnAtk2)) estado->ataqueSelecionado = 1;
                }
                
                // 2. Jogador escolhe o ALVO (se um ataque já foi escolhido)
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
            // Se a animação terminou, passa para o próximo turno
            if (!estado->animacaoEmExecucao.ativo) {
                ProximoTurno(estado);
            }
            break;

        case ESTADO_TURNO_IA:
            // A IA decide e ataca instantaneamente
            // (Poderíamos adicionar um timer aqui para a IA "pensar")
            ExecutarTurnoIA(estado);
            break;
            
        case ESTADO_FIM_DE_JOGO:
            // (Lógica de fim de jogo)
            break;
    }
}

void DesenharTelaBatalha(EstadoBatalha *estado) {
    // --- Desenha HUD Superior ---
    DrawText("Jogador 1", 20, 15, 20, RAYWHITE);
    // (Barras de HP do Jogador)
    DrawText("Round", (SCREEN_WIDTH / 2) - 40, 15, 20, RAYWHITE);
    // (Barras de HP da IA)
    DrawText("IA", SCREEN_WIDTH - 20 - 200 - 35, 15, 20, RAYWHITE);

    // --- Arena ---
    int arenaY = 80;
    int arenaHeight = 450;
    DrawRectangleLines(10, arenaY, SCREEN_WIDTH - 20, arenaHeight, LIGHTGRAY);
    
    // --- Desenha Personagens ---
    float zoomIdle = 2.0f;
    for (int i = 0; i < 3; i++) {
        // --- Desenha Jogador ---
        if (estado->times.timeJogador[i] != NULL && estado->hpJogador[i] > 0) {
            AnimacaoData* anim = &estado->times.timeJogador[i]->animIdle;
            if (anim->def.numFrames > 0) { 
                Rectangle frame = anim->def.frames[0]; 
                DrawTexturePro(anim->textura, frame, 
                    (Rectangle){posJogador[i].x, posJogador[i].y, frame.width * zoomIdle, frame.height * zoomIdle},
                    (Vector2){frame.width * zoomIdle / 2, frame.height * zoomIdle / 2}, 0, WHITE);
                DrawRectangle(posJogador[i].x - 50, posJogador[i].y + 50, 100, 10, DARKGRAY);
                DrawRectangle(posJogador[i].x - 50, posJogador[i].y + 50, (int)(100.0f * estado->hpJogador[i] / estado->times.timeJogador[i]->hpMax), 10, GREEN);
            }
        }
        
        // --- Desenha IA ---
        if (estado->times.timeIA[i] != NULL && estado->hpIA[i] > 0) {
            AnimacaoData* anim = &estado->times.timeIA[i]->animIdle;
            if (anim->def.numFrames > 0) { 
                Rectangle frame = anim->def.frames[0];
                DrawTexturePro(anim->textura, frame, 
                    (Rectangle){posIA[i].x, posIA[i].y, frame.width * zoomIdle, frame.height * zoomIdle},
                    (Vector2){frame.width * zoomIdle / 2, frame.height * zoomIdle / 2}, 0, WHITE);
                DrawRectangle(posIA[i].x - 50, posIA[i].y + 50, 100, 10, DARKGRAY);
                DrawRectangle(posIA[i].x - 50, posIA[i].y + 50, (int)(100.0f * estado->hpIA[i] / estado->times.timeIA[i]->hpMax), 10, RED);
            }
        }
    }
    
    // --- Desenha Animação de Ataque (se estiver ativa) ---
    DesenharAnimacao(&estado->animacaoEmExecucao);
    
    // --- Desenha Alvos (se for a vez do jogador) ---
    if (estado->estadoTurno == ESTADO_ESPERANDO_JOGADOR && estado->ataqueSelecionado != -1) {
        for (int i=0; i<3; i++) {
            if (estado->hpIA[i] > 0) {
                DrawRectangleLines(posIA[i].x - 50, posIA[i].y - 50, 100, 100, YELLOW);
            }
        }
    }

    // --- Menu de Ataques (inferior) ---
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
        
            Vector2 mousePos = GetMousePosition();
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