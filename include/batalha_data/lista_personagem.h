#ifndef LISTA_PERSONAGEM_H
#define LISTA_PERSONAGEM_H

#include "batalha_data/database.h" // Precisa saber o que é PersonagemData
#include <stdbool.h> 

// 1. DEFINIÇÃO DAS STRUCTS DA LISTA
typedef struct NoPersonagem {
    PersonagemData* personagem;
    int posicaoNoTime; // 0 (Frente), 1 (Meio), 2 (Tras)
    
    struct NoPersonagem* proximo;
    struct NoPersonagem* anterior;
} NoPersonagem;

typedef struct ListaTime {
    NoPersonagem* inicio;
    NoPersonagem* fim;
    int tamanho;
} ListaTime;

// 2. DECLARAÇÃO DAS FUNÇÕES (que estão em .c)
ListaTime CriarLista(void);
void InserirPersonagem(ListaTime* lista, PersonagemData* p, int posicao);
NoPersonagem* ObterNoNaPosicao(ListaTime* lista, int pos);
void RemoverPersonagem(ListaTime* lista, NoPersonagem* noParaRemover);
void LiberarLista(ListaTime* lista);
void DesenharLista(ListaTime* lista);

#endif