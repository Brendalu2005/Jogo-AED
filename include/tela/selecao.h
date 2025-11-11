#ifndef SELECAO_H
#define SELECAO_H

#include "telas.h"
#include "database.h"

// Define os times
typedef struct {
    PersonagemData* timeJogador[3];
    PersonagemData* timeIA[3];
} TimesBatalha;

// Funções da tela
void InicializarSelecao(TimesBatalha* times);
void AtualizarTelaSelecao(GameScreen *telaAtual, SpriteDatabase* db, TimesBatalha* times, ModoDeJogo modo);
void DesenharTelaSelecao(SpriteDatabase* db, TimesBatalha* times, ModoDeJogo modo);

#endif