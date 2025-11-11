#include "batalha.h"
#include "database.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "ConsumoAPI_Gemini.h" 
// 1. Função para criar uma lista vazia
ListaTime CriarLista() {
    ListaTime lista; // Cria a struct na stack (sem malloc)
    
    lista.inicio = NULL;
    lista.fim = NULL;
    lista.tamanho = 0;
    
    return lista; // Retorna a struct inicializada
}

// 2. Função para inserir um personagem (no fim da lista, durante a seleção)
void InserirPersonagem(ListaTime* lista, PersonagemData* p, int posicao) {
    NoPersonagem* novoNo = (NoPersonagem*)malloc(sizeof(NoPersonagem));
    if (novoNo == NULL) {
        return; // Falha ao alocar
    }
    novoNo->personagem = p;
    novoNo->posicaoNoTime = posicao;
    novoNo->proximo = NULL;
    novoNo->anterior = lista->fim;

    if (lista->fim != NULL) {
        lista->fim->proximo = novoNo;
    }
    lista->fim = novoNo;

    if (lista->inicio == NULL) {
        lista->inicio = novoNo;
    }
    lista->tamanho++;
}

// 3. Função para obter o Nó da posição (0, 1, ou 2)
NoPersonagem* ObterNoNaPosicao(ListaTime* lista, int pos) {
    NoPersonagem* atual = lista->inicio;
    while (atual != NULL) {
        if (atual->posicaoNoTime == pos) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL; // Posição vazia (personagem derrotado)
}

// 4. Função para remover um personagem (quando o HP <= 0)
void RemoverPersonagem(ListaTime* lista, NoPersonagem* noParaRemover) {
    if (noParaRemover == NULL || lista == NULL) {
        return;
    }

    // Se é o primeiro item da lista
    if (noParaRemover->anterior != NULL) {
        noParaRemover->anterior->proximo = noParaRemover->proximo;
    } else {
        lista->inicio = noParaRemover->proximo;
    }

    // Se é o último item da lista
    if (noParaRemover->proximo != NULL) {
        noParaRemover->proximo->anterior = noParaRemover->anterior;
    } else {
        lista->fim = noParaRemover->anterior;
    }

    free(noParaRemover);
    lista->tamanho--;
}

// 5. Função para desenhar os personagens da lista
void DesenharLista(ListaTime* lista) {
    NoPersonagem* atual = lista->inicio;
    
    while (atual != NULL) {
        // ... (seu código de desenho aqui) ...
        // (Você não estava usando 'flip' aqui, por isso o aviso)
        
        atual = atual->proximo;
    }
}

// 6. (Bônus) Função para liberar a memória da lista no fim do jogo
void LiberarLista(ListaTime* lista) {
    NoPersonagem* atual = lista->inicio;
    while (atual != NULL) {
        NoPersonagem* proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    lista->inicio = NULL;
    lista->fim = NULL;
    lista->tamanho = 0;
}