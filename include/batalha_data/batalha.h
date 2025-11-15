#ifndef BATALHA_H
#define BATALHA_H

#include "telas.h"
#include "database.h"
#include "selecao.h" 

// --- MUDANÇA IMPORTANTE ---
// Inclui a definição de 'ListaTime' do arquivo raiz 'include/'
#include "lista_personagem.h" 
// --------------------------

typedef enum {
    TURNO_JOGADOR,
    TURNO_IA
} TurnoDe;

// As structs da lista NÃO estão mais aqui

typedef enum {
    ESTADO_INICIANDO,      
    ESTADO_ESPERANDO_JOGADOR, 
    ESTADO_ZOOM_IN_ATAQUE,  
    ESTADO_ANIMACAO_ATAQUE, 
    ESTADO_ZOOM_OUT_ATAQUE, 
    // --- MODIFICADO ---
    ESTADO_AGUARDANDO_OPONENTE, // Renomeado de ESTADO_TURNO_IA
    // ------------------
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
    ListaTime timeJogador;
    ListaTime timeIA;
    
    int hpJogador[3];
    int hpIA[3];

    PersonagemData* ordemDeAtaque[6]; 
    int personagemAgindoIdx; 
    TurnoDe turnoDe;
    EstadoTurno estadoTurno;
    int ataqueSelecionado;
    int alvoSelecionado;
    char mensagemBatalha[256];
    EstadoAnimacao animacaoEmExecucao;
    int roundAtual;
    
    PersonagemData* atacanteEmFoco;
    PersonagemData* alvoEmFoco;
    int alvoEmFocoIdx;
    Vector2 posFocoAtacante;
    Vector2 posFocoAlvo;
    float zoomFocoAtacante;
    float zoomFocoAlvo;
    float timerFoco;
    AnimacaoData* animParaTocar;
    bool animFlip;
    float alphaOutrosPersonagens;

    int idleFrameJogador[3];
    int idleTimerJogador[3];
    int idleFrameIA[3];
    int idleTimerIA[3];

    float timerDanoJogador[3];
    float timerDanoIA[3];

    EstadoAnimacao animLapideJogador[3];
    EstadoAnimacao animLapideIA[3];

    NoPersonagem* noParaRemover[6];
    int numMortosPendentes;

    bool isZoomAoe;
    
} EstadoBatalha;


void InicializarBatalha(EstadoBatalha *estado, TimesBatalha* timesSelecionados);

// --- MODIFICADO ---
// AtualizarTelaBatalha agora recebe o modo de jogo
void AtualizarTelaBatalha(EstadoBatalha *estado, GameScreen *telaAtual, ModoDeJogo modo);
// ------------------

void DesenharTelaBatalha(EstadoBatalha *estado);

#endif