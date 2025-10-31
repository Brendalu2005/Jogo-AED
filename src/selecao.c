#include "selecao.h"
#include "database.h"
#include "telas.h"
#include <stdlib.h> // Para rand()
#include <time.h>   // Para srand()
#include <stdio.h> 

// --- NOVO: Variáveis de Animação (copiadas de personagens.c) ---
// Precisamos de timers e frames para todas as 9 animações
static int animFrame[9] = {0}; // Assumindo 9 personagens
static int animTimer[9] = {0};
static int animVelocidade = 15; // Velocidade mais lenta para idle
// -----------------------------------------------------------------


// Variável estática para rastrear o estado da seleção
// 0 = Linha Frente, 1 = Linha Meio, 2 = Linha Trás, 3 = Pronto
static int etapaSelecao = 0;

static const char* titulosEtapa[] = {
    "Escolha seu personagem da LINHA DE FRENTE",
    "Escolha seu personagem da LINHA DO MEIO",
    "Escolha seu personagem da LINHA DE TRAS"
};

// --- Funções Auxiliares (Sem alteração) ---

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

// --- Funções Públicas (Sem alteração na InicializarSelecao) ---

void InicializarSelecao(TimesBatalha* times) {
    etapaSelecao = 0;
    for(int i=0; i<3; i++) {
        times->timeJogador[i] = NULL;
        times->timeIA[i] = NULL;
    }
    srand(time(NULL)); 
}

// --- ATUALIZADO: AtualizarTelaSelecao ---
void AtualizarTelaSelecao(GameScreen *telaAtual, SpriteDatabase* db, TimesBatalha* times) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        *telaAtual = SCREEN_MENU;
        return;
    }

    // --- Definindo o novo layout ---
    int cardWidth = (SCREEN_WIDTH - 200) / 3 - 20; // 446px
    int cardHeight = 120; // Card mais alto para a animação
    int yPosBase = 250; // Posição Y da primeira linha (mais baixa)
    int yEspacamento = 200; // Espaço vertical entre as linhas

    // Se a seleção não terminou (etapa 0, 1 ou 2)
    if (etapaSelecao < 3) {
        ClassePersonagem classeAtual = (ClassePersonagem)etapaSelecao;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            
            // Posição Y da linha atual
            int yPos = yPosBase + yEspacamento * etapaSelecao; 
            int xPos = 100;
            
            for (int i = 0; i < db->numPersonagens; i++) {
                 if (db->personagens[i].classe == classeAtual) {
                    // Usa a nova altura do card para a hitbox
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

    // --- NOVO: Atualiza todas as 9 animações da grade ---
    for (int i = 0; i < db->numPersonagens && i < 9; i++) { // i < 9 é uma segurança
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
                animFrame[i] = 0; // Garante que o frame é 0 se não houver animação
            }
        }
    }
    
    // Se a seleção terminou (etapa == 3)
    if (etapaSelecao == 3) {
        SelecionarTimeIA(db, times);
        etapaSelecao = 4; // Impede de rodar de novo
        *telaAtual = SCREEN_BATALHA; // VAI PARA A BATALHA!
    }
}

// --- ATUALIZADO: DesenharTelaSelecao ---
void DesenharTelaSelecao(SpriteDatabase* db, TimesBatalha* times) {
    ClearBackground(DARKGRAY);

    if (etapaSelecao >= 3) {
        DrawText("CARREGANDO BATALHA...", 200, 400, 40, WHITE);
        return;
    }

    // Desenha o título da etapa atual
    const char* titulo = titulosEtapa[etapaSelecao];
    DrawText(titulo, (SCREEN_WIDTH - MeasureText(titulo, 40)) / 2, 80, 40, WHITE);

    // --- Definindo o novo layout (deve ser igual ao da Atualizar) ---
    int cardWidth = (SCREEN_WIDTH - 200) / 3 - 20;
    int cardHeight = 120;
    int yPosBase = 250;
    int yEspacamento = 200;
    
    ClassePersonagem classes[] = {CLASSE_LINHA_FRENTE, CLASSE_LINHA_MEIO, CLASSE_LINHA_TRAS};
    const char* nomesClasses[] = {"Linha de Frente", "Linha do Meio", "Linha de Trás"};
    
    for (int c = 0; c < 3; c++) {
        ClassePersonagem classeAtual = classes[c];
        
        // --- Posição Y da linha atual ---
        int yPos = yPosBase + yEspacamento * c;
        
        // --- Posição do Título da Linha (CORRIGIDO) ---
        // Desenha o título 50 pixels ACIMA do card
        DrawText(nomesClasses[c], 50, yPos - 50, 30, (c == etapaSelecao) ? YELLOW : WHITE);
        
        int xPos = 100;
        
        for (int i = 0; i < db->numPersonagens; i++) {
            if (db->personagens[i].classe == classeAtual) {
                
                // --- Card com nova altura ---
                Rectangle card = { (float)xPos, (float)yPos, (float)cardWidth, (float)cardHeight };
                
                // Define a cor
                Color cor = GRAY;
                if (c == etapaSelecao) cor = LIGHTGRAY; // Classe atual
                if (times->timeJogador[c] == &db->personagens[i]) cor = GREEN; // Já selecionado
                
                DrawRectangleRec(card, cor);
                DrawRectangleLines((int)card.x, (int)card.y, (int)card.width, (int)card.height, WHITE);
                
                // --- NOVO: Desenha a Animação Idle (Lado Esquerdo) ---
                AnimacaoData* anim = &db->personagens[i].animIdle;
                if (anim->def.numFrames > 0) {
                    Rectangle frame = anim->def.frames[animFrame[i]];
                    
                    // Define a área de desenho da animação (metade esquerda do card)
                    Rectangle areaAnim = { card.x + 10, card.y + 10, (card.width / 2.0f) - 20, card.height - 20 };

                    // Calcula o zoom para caber na ALTURA da área
                    float zoom = areaAnim.height / frame.height;
                    
                    // Se ficar muito largo, calcula o zoom pela LARGURA
                    if (frame.width * zoom > areaAnim.width) {
                        zoom = areaAnim.width / frame.width;
                    }

                    // Desenha a animação centralizada na 'areaAnim'
                    DrawTexturePro(anim->textura, frame,
                        (Rectangle){ areaAnim.x + areaAnim.width / 2, areaAnim.y + areaAnim.height / 2, frame.width * zoom, frame.height * zoom },
                        (Vector2){ (frame.width * zoom) / 2, (frame.height * zoom) / 2 }, 0, WHITE);
                }
                
                // --- NOVO: Desenha os Textos (Lado Direito) ---
                int textoX = (int)card.x + (int)card.width / 2; // Começa da metade do card
                int textoY = (int)card.y + 20;

                DrawText(db->personagens[i].nome, textoX + 10, textoY, 20, BLACK);
                DrawText(TextFormat("HP: %d", db->personagens[i].hpMax), textoX + 10, textoY + 30, 18, BLACK);
                DrawText(TextFormat("Vel: %d", db->personagens[i].velocidade), textoX + 10, textoY + 55, 18, BLACK);
                
                xPos += cardWidth + 20; // Próximo card
            }
        }
    }
    
    DrawText("Pressione ESC para voltar ao Menu", 10, SCREEN_HEIGHT - 30, 20, WHITE);
}