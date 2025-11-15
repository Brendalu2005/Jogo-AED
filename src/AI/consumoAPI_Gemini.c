#include "ConsumoAPI_Gemini.h" 
#include "cJSON.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>  
#include <stdbool.h>  

// --- CORREÇÃO ---
#include "batalha.h" // <-- Inclui a definição COMPLETA de EstadoBatalha
// Precisamos das definições da lista e da função ObterNoNaPosicao()
#include "lista_personagem.h" 

// --- Variáveis Globais (Estáticas) para Gerenciamento da Thread ---
static pthread_mutex_t g_mutex_ia = PTHREAD_MUTEX_INITIALIZER;
static pthread_t g_thread_ia_id;
static volatile bool g_decisao_ia_pronta = false;
static volatile bool g_thread_ia_executando = false;
static DecisaoIA g_decisao_compartilhada;

typedef struct {
    char prompt[2048]; 
    char chave_api[256]; 
} DadosParaThreadIA;

static DadosParaThreadIA g_dados_para_thread; 

// --- Funções de Callback (Sem alterações) ---
typedef struct RespostaWeb {
    char *buffer;
    size_t tamanho;
} RespostaWeb;

static size_t EscreverDadosCallback(void *dados, size_t tamanho, size_t nmemb, void *ponteiroUsuario) {
    size_t tamanhoReal = tamanho * nmemb;
    RespostaWeb *mem = (RespostaWeb *)ponteiroUsuario;

    char *ptr = (char*)realloc(mem->buffer, mem->tamanho + tamanhoReal + 1);
    if (ptr == NULL) {
        printf("ERRO: Falha ao alocar memoria (realloc) no callback da curl\n");
        return 0; 
    }

    mem->buffer = ptr;
    memcpy(&(mem->buffer[mem->tamanho]), dados, tamanhoReal);
    mem->tamanho += tamanhoReal;
    mem->buffer[mem->tamanho] = '\0';

    return tamanhoReal;
}

// --- ConstruirPrompt (COM CORREÇÃO) ---
static char* ConstruirPrompt(struct EstadoBatalha *estadoIncompleto) { // <-- ALTERAÇÃO AQUI
    
    // Converte o ponteiro incompleto para o tipo completo que conhecemos
    EstadoBatalha* estado = (EstadoBatalha*)estadoIncompleto; // <-- ALTERAÇÃO AQUI

    PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
    
    NoPersonagem* noAlvo0 = ObterNoNaPosicao(&estado->timeJogador, 0);
    NoPersonagem* noAlvo1 = ObterNoNaPosicao(&estado->timeJogador, 1);
    NoPersonagem* noAlvo2 = ObterNoNaPosicao(&estado->timeJogador, 2);

    const char* nomeAlvo0;
    int hpMaxAlvo0;
    if (noAlvo0 != NULL) {
        nomeAlvo0 = noAlvo0->personagem->nome;
        hpMaxAlvo0 = noAlvo0->personagem->hpMax;
    } else {
        nomeAlvo0 = "Morto";
        hpMaxAlvo0 = 0;
    }

    const char* nomeAlvo1;
    int hpMaxAlvo1;
    if (noAlvo1 != NULL) {
        nomeAlvo1 = noAlvo1->personagem->nome;
        hpMaxAlvo1 = noAlvo1->personagem->hpMax;
    } else {
        nomeAlvo1 = "Morto";
        hpMaxAlvo1 = 0;
    }

    const char* nomeAlvo2;
    int hpMaxAlvo2;
    if (noAlvo2 != NULL) {
        nomeAlvo2 = noAlvo2->personagem->nome;
        hpMaxAlvo2 = noAlvo2->personagem->hpMax;
    } else {
        nomeAlvo2 = "Morto";
        hpMaxAlvo2 = 0;
    }

    char promptBuffer[2048];
    
    snprintf(promptBuffer, sizeof(promptBuffer),
        "Voce eh a IA de um jogo, escolha personagens diversificados. Além de usar ataques diferentes, você NÃO pode usar o mesmo ataque 2 turnos seguidos. O personagem %s (HP: ?\?/?\?) esta atacando.\n" 
        "Seus ataques sao:\n"
        "1. %s (%s, Dano: %d)\n"
        "2. %s (%s, Dano: %d)\n\n"
        "O time inimigo (Jogador) eh:\n"
        "Alvo 0: %s (HP: %d/%d)\n"
        "Alvo 1: %s (HP: %d/%d)\n"
        "Alvo 2: %s (HP: %d/%d)\n\n"
        "Qual a melhor jogada? Responda APENAS com um JSON no formato: " 
        "{\"ataque\": <0 ou 1>, \"alvo\": <0, 1, ou 2>, \"pensamento\": \"sua logica aqui\"}",
        
        atacante->nome,
        atacante->ataque1.nome, atacante->ataque1.descricao, atacante->ataque1.dano,
        atacante->ataque2.nome, atacante->ataque2.descricao, atacante->ataque2.dano,
        
        nomeAlvo0, estado->hpJogador[0], hpMaxAlvo0,
        nomeAlvo1, estado->hpJogador[1], hpMaxAlvo1,
        nomeAlvo2, estado->hpJogador[2], hpMaxAlvo2
    );

    char* promptFinal = (char*)malloc(strlen(promptBuffer) + 1);
    strcpy(promptFinal, promptBuffer);
    return promptFinal;
}

// --- ExecutarConsultaCurl (Sem alterações) ---
static DecisaoIA ExecutarConsultaCurl(const char* prompt, const char* suaChaveAPI) {
    CURL *curl;
    CURLcode res;
    DecisaoIA decisao = {0, 0, NULL}; 
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (curl) {
        RespostaWeb resposta = {0};
        resposta.buffer = (char*)malloc(1); 
        resposta.tamanho = 0;
        
        cJSON *jsonBody = cJSON_CreateObject();
        cJSON *contents = cJSON_CreateArray();
        cJSON *contentPart = cJSON_CreateObject();
        cJSON *parts = cJSON_CreateArray();
        cJSON *partText = cJSON_CreateObject();

       
        cJSON_AddItemToObject(partText, "text", cJSON_CreateString(prompt));
        cJSON_AddItemToArray(parts, partText);
        cJSON_AddItemToObject(contentPart, "parts", parts);
        cJSON_AddItemToArray(contents, contentPart);
        cJSON_AddItemToObject(jsonBody, "contents", contents);
        
        cJSON *genConfig = cJSON_CreateObject();
        cJSON_AddItemToObject(genConfig, "maxOutputTokens", cJSON_CreateNumber(1024));
        cJSON_AddItemToObject(jsonBody, "generationConfig", genConfig);
        
        char* corpoRequisicao = cJSON_PrintUnformatted(jsonBody);
        cJSON_Delete(jsonBody); 

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        char urlComChave[512];

        snprintf(urlComChave, sizeof(urlComChave), 
                 "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent?key=%s", 
                 suaChaveAPI);
                 
        curl_easy_setopt(curl, CURLOPT_URL, urlComChave); 
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, corpoRequisicao);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, EscreverDadosCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resposta);
        
        printf("IA (Thread): Consultando API do Gemini...\n"); 
        res = curl_easy_perform(curl); 
        printf("IA (Thread): Resposta recebida.\n");

        printf("IA: RESPOSTA DA API: \n%s\n", resposta.buffer);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() falhou: %s\n", curl_easy_strerror(res));
        } else {
            cJSON *jsonResposta = cJSON_Parse(resposta.buffer);
            if (jsonResposta) {
                cJSON *candidates = cJSON_GetObjectItem(jsonResposta, "candidates");
                if (candidates) {
                    cJSON *firstCandidate = cJSON_GetArrayItem(candidates, 0);
                    cJSON *content = cJSON_GetObjectItem(firstCandidate, "content");
                    cJSON *parts = cJSON_GetObjectItem(content, "parts");
                    cJSON *firstPart = cJSON_GetArrayItem(parts, 0);
                    cJSON *conteudoJsonStr = cJSON_GetObjectItem(firstPart, "text");

                    if (cJSON_IsString(conteudoJsonStr)) {
                        char* strInicio = strstr(conteudoJsonStr->valuestring, "{");
                        char* strFim = strrchr(conteudoJsonStr->valuestring, '}');
                        char* strJsonLimpo = conteudoJsonStr->valuestring; 

                        if (strInicio && strFim && strFim > strInicio) {
                            *(strFim + 1) = '\0'; 
                            strJsonLimpo = strInicio;
                        }
                        cJSON *decisaoJson = cJSON_Parse(strJsonLimpo); 
                        if (decisaoJson) {
                            cJSON *itemAtaque = cJSON_GetObjectItem(decisaoJson, "ataque");
                            cJSON *itemAlvo = cJSON_GetObjectItem(decisaoJson, "alvo");
                            cJSON *itemPensamento = cJSON_GetObjectItem(decisaoJson, "pensamento");

                            if (cJSON_IsNumber(itemAtaque)) decisao.indiceAtaque = itemAtaque->valueint;
                            if (cJSON_IsNumber(itemAlvo)) decisao.indiceAlvo = itemAlvo->valueint;
                            
                            if (cJSON_IsString(itemPensamento)) {
                                decisao.justificativa = (char*)malloc(strlen(itemPensamento->valuestring) + 1);
                                strcpy(decisao.justificativa, itemPensamento->valuestring);
                            }
                            
                            cJSON_Delete(decisaoJson);
                        }
                    }
                }
                cJSON_Delete(jsonResposta);
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(corpoRequisicao);
        free(resposta.buffer);
    }
    
    curl_global_cleanup();
    
    return decisao;
}

// --- ThreadTrabalhadoraIA (Sem alterações) ---
static void* ThreadTrabalhadoraIA(void* arg) {
    (void)arg; 

    printf("IA (Thread): Thread iniciada.\n");
    
    DecisaoIA decisao = ExecutarConsultaCurl(g_dados_para_thread.prompt, g_dados_para_thread.chave_api);

    pthread_mutex_lock(&g_mutex_ia);
    
    printf("IA (Thread): Salvando decisão.\n");
    g_decisao_compartilhada = decisao;
    g_decisao_ia_pronta = true;
    g_thread_ia_executando = false;
    
    pthread_mutex_unlock(&g_mutex_ia);

    return NULL;
}


// --- IA_IniciarDecisao (COM CORREÇÃO) ---
void IA_IniciarDecisao(struct EstadoBatalha *estado, const char* suaChaveAPI) { // <-- Assinatura já estava correta
    pthread_mutex_lock(&g_mutex_ia);

    if (g_thread_ia_executando == true) {
        pthread_mutex_unlock(&g_mutex_ia);
        return;
    }
    
    g_thread_ia_executando = true;
    g_decisao_ia_pronta = false;

    LiberarDecisaoIA(&g_decisao_compartilhada);
    
    // A função ConstruirPrompt agora espera 'struct EstadoBatalha *', que é o que temos
    char* prompt = ConstruirPrompt(estado); // <-- ALTERAÇÃO AQUI (agora compila)
    strncpy(g_dados_para_thread.prompt, prompt, 2047);
    g_dados_para_thread.prompt[2047] = '\0';
    
    strncpy(g_dados_para_thread.chave_api, suaChaveAPI, 255);
    g_dados_para_thread.chave_api[255] = '\0';
    
    free(prompt);

    pthread_create(&g_thread_ia_id, NULL, ThreadTrabalhadoraIA, NULL);
    pthread_detach(g_thread_ia_id); 

    pthread_mutex_unlock(&g_mutex_ia);
}

// --- Funções Finais (Sem alterações) ---
bool IA_VerificarDecisaoPronta(DecisaoIA *saida) {
    bool decisao_esta_pronta = false;
    
    pthread_mutex_lock(&g_mutex_ia);
    
    if (g_decisao_ia_pronta == true) {
        *saida = g_decisao_compartilhada; 
        
        g_decisao_compartilhada.justificativa = NULL; 
        
        g_decisao_ia_pronta = false; 
        decisao_esta_pronta = true;
    }
    
    pthread_mutex_unlock(&g_mutex_ia);
    
    return decisao_esta_pronta;
}

void LiberarDecisaoIA(DecisaoIA *decisao) {
    if (decisao->justificativa != NULL) {
        free(decisao->justificativa);
        decisao->justificativa = NULL;
    }
}