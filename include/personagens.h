#ifndef PERSONAGENS_H
#define PERSONAGENS_H

#include "telas.h" 
#include "database.h" // <--- ADICIONE ISSO

// Remove Carregar/Descarregar, pois o database faz isso
void AtualizarTelaPersonagens(GameScreen *telaAtual, int *personagemSelecionado, SpriteDatabase* db);
void DesenharTelaPersonagens(int personagemSelecionado, SpriteDatabase* db);

#endif