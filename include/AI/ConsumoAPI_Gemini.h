#ifndef IA_GPT_H
#define IA_GPT_H

// #include "batalha.h" // <-- REMOVER ESTA LINHA
#include <stdbool.h> // Adicionar para usar 'bool'

// Adicionar uma declaração "incompleta" (forward declaration)
// Isto diz ao compilador "existe uma struct com este nome",
// o que é suficiente para usar um ponteiro para ela.
struct EstadoBatalha; 

// Uma struct para guardar a decisão da IA
typedef struct DecisaoIA {
    int indiceAtaque; 
    int indiceAlvo;   
    char* justificativa; 
} DecisaoIA;


// --- FUNÇÕES ANTIGAS REMOVIDAS ---
// DecisaoIA ObterDecisaoIA(struct EstadoBatalha *estado, const char* suaChaveAPI);


// --- NOVAS FUNÇÕES ASSÍNCRONAS ---

/**
 * @brief Inicia a consulta à API em uma thread separada.
 * Esta função não bloqueia.
 */
void IA_IniciarDecisao(struct EstadoBatalha *estado, const char* suaChaveAPI);

/**
 * @brief Verifica se a thread da IA já terminou de processar.
 * Esta função não bloqueia.
 * @param saida Ponteiro para uma struct DecisaoIA que será preenchida se a decisão estiver pronta.
 * @return true se a decisão estiver pronta, false caso contrário.
 */
bool IA_VerificarDecisaoPronta(DecisaoIA *saida);


void LiberarDecisaoIA(DecisaoIA *decisao);

#endif