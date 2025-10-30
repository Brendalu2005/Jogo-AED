#ifndef PERSONAGENS_H
#define PERSONAGENS_H

#include "telas.h" 

void CarregarRecursosPersonagens(void);

void DescarregarRecursosPersonagens(void);

void AtualizarTelaPersonagens(GameScreen *telaAtual, int *personagemSelecionado);

void DesenharTelaPersonagens(int personagemSelecionado);

#endif 