#ifndef PERSONAGENS_H
#define PERSONAGENS_H

#include "telas.h" 
#include "database.h" 

void CarregarRecursosPersonagens(void);
void DescarregarRecursosPersonagens(void);

void AtualizarTelaPersonagens(GameScreen *telaAtual, int *personagemSelecionado, SpriteDatabase* db);
void DesenharTelaPersonagens(int personagemSelecionado, SpriteDatabase* db);

#endif