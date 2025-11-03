#include "database.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

static char* c99_strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char* new_s = (char*)malloc(len);
    if (new_s == NULL) return NULL;
    memcpy(new_s, s, len); 
    return new_s;
}

static void free_ataque(Ataque* att) {
    free(att->nome);
    free(att->descricao);
}


static char* LerArquivoInteiro(const char* path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("ERRO: Nao foi possivel abrir o arquivo: %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char*)malloc(tam + 1);
    fread(buffer, 1, tam, f);
    fclose(f);
    buffer[tam] = '\0';
    return buffer;
}

static AnimacaoDef ParseAnimacaoTexturePacker(const char* jsonPath) {
    AnimacaoDef def = {0};
    char* jsonString = LerArquivoInteiro(jsonPath);
    if (!jsonString) return def;

    cJSON *json = cJSON_Parse(jsonString);
    if (json == NULL) {
        fprintf(stderr, "Erro no JSON: %s\n", jsonPath);
        free(jsonString);
        return def;
    }

    cJSON* framesObj = cJSON_GetObjectItem(json, "frames");
    if (!framesObj) {
        fprintf(stderr, "Erro: JSON nao tem o objeto 'frames': %s\n", jsonPath);
        cJSON_Delete(json); free(jsonString); return def;
    }

    int numFrames = 0;
    cJSON* frameItem = framesObj->child;
    while (frameItem) { numFrames++; frameItem = frameItem->next; }
    
    if (numFrames == 0) {
        cJSON_Delete(json); free(jsonString); return def;
    }

    def.numFrames = numFrames;
    def.frames = (Rectangle*)malloc(numFrames * sizeof(Rectangle));
    int i = 0;
    frameItem = framesObj->child;
    while (frameItem) {
        cJSON* frameData = cJSON_GetObjectItem(frameItem, "frame");
        if(frameData) {
            float x = (float)cJSON_GetObjectItem(frameData, "x")->valuedouble;
            float y = (float)cJSON_GetObjectItem(frameData, "y")->valuedouble;
            float w = (float)cJSON_GetObjectItem(frameData, "w")->valuedouble;
            float h = (float)cJSON_GetObjectItem(frameData, "h")->valuedouble;
            def.frames[i] = (Rectangle){ x, y, w, h };
            i++;
        }
        frameItem = frameItem->next;
    }
    
    cJSON_Delete(json);
    free(jsonString);
    return def;
}

static void LiberarAnimDef(AnimacaoDef* def) {
    if (def && def->frames) {
        free(def->frames);
        def->frames = NULL;
        def->numFrames = 0;
    }
}

static AnimacaoData CarregarAnimacaoData(const char* pathBase) {
    AnimacaoData data = {0};
    char jsonPath[256];
    char pngPath[256];

    snprintf(jsonPath, sizeof(jsonPath), "%s.json", pathBase);
    snprintf(pngPath, sizeof(pngPath), "%s.png", pathBase);

    data.def = ParseAnimacaoTexturePacker(jsonPath);
    data.textura = LoadTexture(pngPath);
    
    if (data.def.numFrames == 0) {
        printf("AVISO: Falha ao carregar definicao anim: %s\n", jsonPath);
    }
    if (data.textura.id <= 0) {
        printf("AVISO: Falha ao carregar textura: %s\n", pngPath);
    }
    
    return data;
}

static void LiberarAnimacaoData(AnimacaoData* data) {
    LiberarAnimDef(&data->def);
    UnloadTexture(data->textura);
}

static const char* GetStringSafe(cJSON* obj, const char* key) {
    cJSON* item = cJSON_GetObjectItem(obj, key);
    if (cJSON_IsString(item) && (item->valuestring != NULL)) {
        return item->valuestring;
    }
    return "";
}

static double GetFloatSafe(cJSON* obj, const char* key, double defaultValue) {
    cJSON* item = cJSON_GetObjectItem(obj, key);
    if (cJSON_IsNumber(item)) {
        return item->valuedouble;
    }
    return defaultValue; 
}

static int GetIntSafe(cJSON* obj, const char* key) {
    cJSON* item = cJSON_GetObjectItem(obj, key);
    if (cJSON_IsNumber(item)) {
        return item->valueint;
    }
    return 0;
}

static ClassePersonagem GetClasseSafe(cJSON* obj, const char* key) {
    const char* classeStr = GetStringSafe(obj, key);
    if (strcmp(classeStr, "CLASSE_LINHA_FRENTE") == 0) return CLASSE_LINHA_FRENTE;
    if (strcmp(classeStr, "CLASSE_LINHA_MEIO") == 0) return CLASSE_LINHA_MEIO;
    if (strcmp(classeStr, "CLASSE_LINHA_TRAS") == 0) return CLASSE_LINHA_TRAS;
    return CLASSE_LINHA_MEIO; 
}

static Ataque GetAtaqueSafe(cJSON* obj, const char* key) {
    Ataque att = {0};
    cJSON* attObj = cJSON_GetObjectItem(obj, key);
    if (attObj) {
        att.nome = c99_strdup(GetStringSafe(attObj, "nome"));
        att.descricao = c99_strdup(GetStringSafe(attObj, "descricao"));
        att.dano = GetIntSafe(attObj, "dano");
    }
    return att;
}


SpriteDatabase CarregarDatabase(const char* masterJsonPath) {
    SpriteDatabase db = {0};
    char* jsonString = LerArquivoInteiro(masterJsonPath);
    if (!jsonString) {
        fprintf(stderr, "ERRO FATAL: Nao foi possivel ler o JSON Mestre: %s\n", masterJsonPath);
        return db;
    }

    cJSON *json = cJSON_Parse(jsonString);
    if (json == NULL) {
        fprintf(stderr, "ERRO FATAL: Erro de sintaxe no JSON Mestre: %s\n", masterJsonPath);
        free(jsonString);
        return db;
    }

    cJSON* personagensArray = cJSON_GetObjectItem(json, "personagens");
    if (!cJSON_IsArray(personagensArray)) {
        fprintf(stderr, "ERRO FATAL: 'personagens' nao eh um array no JSON Mestre.\n");
        cJSON_Delete(json); free(jsonString); return db;
    }

    db.numPersonagens = cJSON_GetArraySize(personagensArray);
    if (db.numPersonagens == 0) {
        cJSON_Delete(json); free(jsonString); return db;
    }
    
    db.personagens = (PersonagemData*)calloc(db.numPersonagens, sizeof(PersonagemData));

    for (int i = 0; i < db.numPersonagens; i++) {
        cJSON* pJson = cJSON_GetArrayItem(personagensArray, i);
        PersonagemData* pData = &db.personagens[i];
        
        const char* nome = GetStringSafe(pJson, "nome");
        const char* idlePath = GetStringSafe(pJson, "idle");
        const char* atq1Path = GetStringSafe(pJson, "ataque1");
        const char* atq2Path = GetStringSafe(pJson, "ataque2");
        const char* thumbPath = GetStringSafe(pJson, "thumbnailPath");


        if (nome == NULL || idlePath == NULL || atq1Path == NULL || atq2Path == NULL || thumbPath == NULL) {
            fprintf(stderr, "ERRO FATAL: Personagem %d no JSON mestre esta com dados faltando. Pulando.\n", i);
            pData->nome = c99_strdup("ERRO_CARREGAMENTO"); 
            continue; 
        }

        pData->nome = c99_strdup(nome);
        pData->descricao = c99_strdup(GetStringSafe(pJson, "descricao"));
        pData->classe = GetClasseSafe(pJson, "classe");
        
        pData->hpMax = GetIntSafe(pJson, "hpMax");
        pData->velocidade = GetIntSafe(pJson, "velocidade");

        pData->painelZoom = (float)GetFloatSafe(pJson, "painelZoom", 3.0f);
        pData->batalhaZoom = (float)GetFloatSafe(pJson, "batalhaZoom", 2.0f); 

        pData->ataque1 = GetAtaqueSafe(pJson, "ataque1");
        pData->ataque2 = GetAtaqueSafe(pJson, "ataque2");
        
        pData->thumbnail = LoadTexture(thumbPath);

        printf("--- Carregando %s (HP: %d, Vel: %d) ---\n", pData->nome, pData->hpMax, pData->velocidade);
        
        cJSON* animObj = cJSON_GetObjectItem(pJson, "anim");
        if (animObj) {
            pData->animIdle = CarregarAnimacaoData(GetStringSafe(animObj, "idle"));
            pData->animAtaque1 = CarregarAnimacaoData(GetStringSafe(animObj, "ataque1"));
            pData->animAtaque2 = CarregarAnimacaoData(GetStringSafe(animObj, "ataque2"));
        }
        printf("----------------------------------\n");
    }
    
    cJSON_Delete(json);
    free(jsonString);
    return db;
}

void LiberarDatabase(SpriteDatabase* db) {
    if (db == NULL || db->personagens == NULL) return;
    
    for (int i = 0; i < db->numPersonagens; i++) {
        PersonagemData* pData = &db->personagens[i];
        free(pData->nome);
        free(pData->descricao);
        free_ataque(&pData->ataque1);
        free_ataque(&pData->ataque2);
        
        UnloadTexture(pData->thumbnail);
        LiberarAnimacaoData(&pData->animIdle);
        LiberarAnimacaoData(&pData->animAtaque1);
        LiberarAnimacaoData(&pData->animAtaque2);
    }
    free(db->personagens);
    *db = (SpriteDatabase){0};
}

PersonagemData* GetPersonagemData(SpriteDatabase* db, const char* nome) {
    if (db == NULL || db->personagens == NULL) return NULL;
    
    for (int i = 0; i < db->numPersonagens; i++) {
        if (strcmp(db->personagens[i].nome, nome) == 0) {
            return &db->personagens[i];
        }
    }
    printf("ERRO: Personagem '%s' nao encontrado no database.\n", nome);
    return NULL;
}