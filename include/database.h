#ifndef DATABASE_H
#define DATABASE_H

#include "raylib.h"

// --- Novas Estruturas ---

// Define um único ataque
typedef struct Ataque {
    char* nome;
    char* descricao;
    int dano;
} Ataque;

// Define uma classe de personagem
typedef enum ClassePersonagem {
    CLASSE_LINHA_FRENTE,
    CLASSE_LINHA_MEIO,
    CLASSE_LINHA_TRAS
} ClassePersonagem;

// --- Estruturas de Animação (as mesmas de antes) ---

typedef struct AnimacaoDef {
    Rectangle* frames;
    int numFrames;
} AnimacaoDef;

typedef struct AnimacaoData {
    AnimacaoDef def;
    Texture2D textura;
} AnimacaoData;

// --- Estrutura Principal (Atualizada) ---

// Guarda TODOS os dados de UM personagem
typedef struct PersonagemData {
    // Info Base
    char* nome;
    char* descricao;
    ClassePersonagem classe;
    
    // Stats de Batalha
    int hpMax;
    int velocidade;

    float painelZoom;
    
    // Ataques
    Ataque ataque1;
    Ataque ataque2;
    
    // Assets
    Texture2D thumbnail; // A imagem pequena (ex: docinhothumb.png)
    AnimacaoData animIdle;
    AnimacaoData animAtaque1;
    AnimacaoData animAtaque2;
} PersonagemData;

// O Database principal com todos os personagens
typedef struct SpriteDatabase {
    PersonagemData* personagens;
    int numPersonagens;
} SpriteDatabase;

// --- Funções Públicas ---

SpriteDatabase CarregarDatabase(const char* masterJsonPath);
void LiberarDatabase(SpriteDatabase* db);
PersonagemData* GetPersonagemData(SpriteDatabase* db, const char* nome);

#endif // DATABASE_H