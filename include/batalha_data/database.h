#ifndef DATABASE_H
#define DATABASE_H

#include "raylib.h"
    
typedef enum TipoAtaque {
    TIPO_DANO_UNICO, 
    TIPO_DANO_AREA,  
    TIPO_CURA_SI     
} TipoAtaque;
    
// Define um único ataque
typedef struct Ataque {
    char* nome;
    char* descricao;
    int dano;
    TipoAtaque tipo; 
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
    bool flip;
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

// deixei publica pra adicionar a lápide em batalha.c
AnimacaoData CarregarAnimacaoData(const char* pathBase);
void LiberarAnimacaoData(AnimacaoData* data);
void LiberarAnimDef(AnimacaoDef* def);

#endif 