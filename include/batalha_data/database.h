#ifndef DATABASE_H
#define DATABASE_H

#include "raylib.h"
    
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

//animação
typedef struct AnimacaoDef {
    Rectangle* frames;
    int numFrames;
} AnimacaoDef;

typedef struct AnimacaoData {
    AnimacaoDef def;
    Texture2D textura;
} AnimacaoData;

// Guarda os atributos do personagem
typedef struct PersonagemData {
    char* nome;
    char* descricao;
    ClassePersonagem classe;
    
    // Status de Batalha
    int hpMax;
    int velocidade;
    float painelZoom;  
    float batalhaZoom; 
    
    // Ataques
    Ataque ataque1;
    Ataque ataque2;
    
    // Assets
    Texture2D thumbnail;
    AnimacaoData animIdle;
    AnimacaoData animAtaque1;
    AnimacaoData animAtaque2;
} PersonagemData;

// O Database principal com todos os personagens
typedef struct SpriteDatabase {
    PersonagemData* personagens;
    int numPersonagens;
} SpriteDatabase;


SpriteDatabase CarregarDatabase(const char* masterJsonPath);
void LiberarDatabase(SpriteDatabase* db);
PersonagemData* GetPersonagemData(SpriteDatabase* db, const char* nome);

#endif 