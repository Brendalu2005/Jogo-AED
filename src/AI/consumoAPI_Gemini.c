#include "ConsumoAPI_Gemini.h" 
#include "cJSON.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

static char* ConstruirPrompt(EstadoBatalha *estado) {
    PersonagemData* atacante = estado->ordemDeAtaque[estado->personagemAgindoIdx];
    
    char promptBuffer[2048];
    
    // prompt *(guto)
    snprintf(promptBuffer, sizeof(promptBuffer),
        "Voce eh a IA de um jogo. O personagem %s (HP: ??/??) esta atacando.\n" 
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
        
        estado->times.timeJogador[0]->nome, estado->hpJogador[0], estado->times.timeJogador[0]->hpMax,
        estado->times.timeJogador[1]->nome, estado->hpJogador[1], estado->times.timeJogador[1]->hpMax,
        estado->times.timeJogador[2]->nome, estado->hpJogador[2], estado->times.timeJogador[2]->hpMax
    );

    char* promptFinal = (char*)malloc(strlen(promptBuffer) + 1);
    strcpy(promptFinal, promptBuffer);
    return promptFinal;
}

DecisaoIA ObterDecisaoIA(EstadoBatalha *estado, const char* suaChaveAPI) {
    CURL *curl;
    CURLcode res;
    DecisaoIA decisao = {0, 0, NULL}; 
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (curl) {
        RespostaWeb resposta = {0};
        resposta.buffer = (char*)malloc(1); 
        resposta.tamanho = 0;

        // construir o prompt *(guto)
        char* prompt = ConstruirPrompt(estado);
        

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
        //config para qnt de token *(guto)
        cJSON *genConfig = cJSON_CreateObject();
        cJSON_AddItemToObject(genConfig, "maxOutputTokens", cJSON_CreateNumber(150));
        cJSON_AddItemToObject(jsonBody, "generationConfig", genConfig);
        
        char* corpoRequisicao = cJSON_PrintUnformatted(jsonBody);
        free(prompt); 
        cJSON_Delete(jsonBody); 

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        char urlComChave[512];

        snprintf(urlComChave, sizeof(urlComChave), 
                 "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=%s", 
                 suaChaveAPI);
        curl_easy_setopt(curl, CURLOPT_URL, urlComChave); 
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, corpoRequisicao);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, EscreverDadosCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resposta);
        
        printf("IA: Consultando API do Gemini...\n"); // Mudei o texto
        res = curl_easy_perform(curl);
        printf("IA: Resposta recebida.\n");

        // processa a resposta *(guto)
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
                        //Limpa algumas respostas fora do padrão *(guto)
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
    
    if (estado->hpJogador[decisao.indiceAlvo] <= 0) {
        for (int i = 0; i < 3; i++) {
            if (estado->hpJogador[i] > 0) {
                decisao.indiceAlvo = i;
                break;
            }
        }
    }
    
    return decisao;
}

void LiberarDecisaoIA(DecisaoIA *decisao) {
    if (decisao->justificativa != NULL) {
        free(decisao->justificativa);
        decisao->justificativa = NULL;
    }
}