#ifndef IA_GPT_H
#define IA_GPT_H

#include "batalha.h" 

// Uma struct para guardar a decisão da IA
typedef struct DecisaoIA {
    int indiceAtaque; 
    int indiceAlvo;   
    char* justificativa; 
} DecisaoIA;


DecisaoIA ObterDecisaoIA(EstadoBatalha *estado, const char* suaChaveAPI);

void LiberarDecisaoIA(DecisaoIA *decisao);

#endif