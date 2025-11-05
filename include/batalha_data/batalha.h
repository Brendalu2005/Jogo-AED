#ifndef BATALHA_H
#define BATALHA_H

#include "telas.h"
#include "database.h"
#include "selecao.h" 

// --- Structs ---
typedef enum {
    TURNO_JOGADOR,
    TURNO_IA
} TurnoDe;
typedef enum {
    ESTADO_INICIANDO,      
    ESTADO_ESPERANDO_JOGADOR, 
    ESTADO_ANIMACAO_ATAQUE, 
    ESTADO_TURNO_IA,        
    ESTADO_FIM_DE_JOGO
} EstadoTurno;
typedef struct {
    AnimacaoData* anim;
    int frameAtual;
    int timer;
    int velocidade;
    bool ativo;
    Vector2 pos;
    float zoom;
    bool flip; 
} EstadoAnimacao;

typedef struct {
    TimesBatalha times;
    int hpJogador[3];
    int hpIA[3];
    PersonagemData* ordemDeAtaque[6]; 
    int personagemAgindoIdx; 
    EstadoTurno estadoTurno;
    TurnoDe turnoDe;
    int ataqueSelecionado; 
    int alvoSelecionado;   
    EstadoAnimacao animacaoEmExecucao;
    char mensagemBatalha[100];
    
    int idleFrameJogador[3];
    int idleTimerJogador[3];
    int idleFrameIA[3];
    int idleTimerIA[3];
    
} EstadoBatalha;


void InicializarBatalha(EstadoBatalha *estado, TimesBatalha* timesSelecionados);
void AtualizarTelaBatalha(EstadoBatalha *estado, GameScreen *telaAtual);
void DesenharTelaBatalha(EstadoBatalha *estado);

#endif