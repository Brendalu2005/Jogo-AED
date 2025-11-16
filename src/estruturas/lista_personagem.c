#include "batalha.h"
#include "database.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "ConsumoAPI_Gemini.h" 

ListaTime CriarLista() {
    ListaTime lista;
    
    lista.inicio = NULL;
    lista.fim = NULL;
    lista.tamanho = 0;
    
    return lista;
}

void InserirPersonagem(ListaTime* lista, PersonagemData* p, int posicao) {
    NoPersonagem* novoNo = (NoPersonagem*)malloc(sizeof(NoPersonagem));
    if (novoNo == NULL) {
        return;
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

NoPersonagem* ObterNoNaPosicao(ListaTime* lista, int pos) {
    NoPersonagem* atual = lista->inicio;
    while (atual != NULL) {
        if (atual->posicaoNoTime == pos) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL; 
}

void RemoverPersonagem(ListaTime* lista, NoPersonagem* noParaRemover) {
    if (noParaRemover == NULL || lista == NULL) {
        return;
    }

    if (noParaRemover->anterior != NULL) {
        noParaRemover->anterior->proximo = noParaRemover->proximo;
    } else {
        lista->inicio = noParaRemover->proximo;
    }

    if (noParaRemover->proximo != NULL) {
        noParaRemover->proximo->anterior = noParaRemover->anterior;
    } else {
        lista->fim = noParaRemover->anterior;
    }

    free(noParaRemover);
    lista->tamanho--;
}

void DesenharLista(ListaTime* lista) {
    NoPersonagem* atual = lista->inicio;
    
    while (atual != NULL) {
        atual = atual->proximo;
    }
}

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