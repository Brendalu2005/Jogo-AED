#include "personagens.h"
#include "raylib.h"

#define ID_HELLO_KITTY  0
#define ID_SHREK        1
#define ID_MONICA       2
#define ID_BEN10        3
#define ID_DOCINHO      4
#define ID_TAZ          5
#define ID_MIRANHA      6
#define ID_ELSA         7
#define ID_MICKEY       8

static Texture2D texBen10;
static Texture2D texDocinho;
static Texture2D texMiranhathumb; 
static Texture2D texTaz;

static Rectangle rectPersonagens[9]; 

static float escalaMiniatura[9]; 
static float escalaPainel[9];

static Color corTituloLinha = { 100, 255, 100, 255 }; 
static Color corNomePersonagem = RAYWHITE;
static Color corNomeSelecionado = YELLOW;
static Color corPainel = { 50, 50, 50, 200 };
static Color corBordaPainel = { 200, 0, 0, 255 }; 

/*
A gente ainda tem que colocar os outros personagens. aí depois a gente muda a descrição deles.
só tem os que tem lá no docs
*/


void CarregarRecursosPersonagens(void) {
    //as 4 que já temos
    texBen10 = LoadTexture("sprites/telaPersonagem/ben10thumb.png");
    texDocinho = LoadTexture("sprites/telaPersonagem/docinhothumb.png");
    texMiranhathumb = LoadTexture("sprites/telaPersonagem/miranhathumb.png");
    texTaz = LoadTexture("sprites/telaPersonagem/tazthumb.png");

    int linhaFrenteY = 200;
    int linhaMeioY = 450;
    int linhaTrasY = 700;
    
    int coluna1X = 100;
    int coluna2X = 500;
    int coluna3X = 900;
    
    int hitboxWidth = 300; 
    int hitboxHeight = 180; 

    // Define as hitboxes pra cada personagem
    rectPersonagens[ID_HELLO_KITTY] = (Rectangle){ coluna1X, linhaFrenteY, hitboxWidth, hitboxHeight };
    rectPersonagens[ID_SHREK] = (Rectangle){ coluna2X, linhaFrenteY, hitboxWidth, hitboxHeight };
    rectPersonagens[ID_MONICA] = (Rectangle){ coluna3X, linhaFrenteY, hitboxWidth, hitboxHeight };
    
    rectPersonagens[ID_BEN10] = (Rectangle){ coluna1X, linhaMeioY, hitboxWidth, hitboxHeight };
    rectPersonagens[ID_DOCINHO] = (Rectangle){ coluna2X, linhaMeioY, hitboxWidth, hitboxHeight };
    rectPersonagens[ID_TAZ] = (Rectangle){ coluna3X, linhaMeioY, hitboxWidth, hitboxHeight };
    
    rectPersonagens[ID_MIRANHA] = (Rectangle){ coluna1X, linhaTrasY, hitboxWidth, hitboxHeight };
    rectPersonagens[ID_ELSA] = (Rectangle){ coluna2X, linhaTrasY, hitboxWidth, hitboxHeight };
    rectPersonagens[ID_MICKEY] = (Rectangle){ coluna3X, linhaTrasY, hitboxWidth, hitboxHeight };

    
    for (int i = 0; i < 9; i++) {
        escalaMiniatura[i] = 3.0f; // Escala padrão da lista
        escalaPainel[i] = 8.0f;    // Escala padrão do painel
    }
    
    // Escalas da Miniatura 
    escalaMiniatura[ID_BEN10] = 1.5f;
    escalaMiniatura[ID_DOCINHO] = 1.5f; // Docinho é larga
    escalaMiniatura[ID_TAZ] = -0.6f;     // Taz é muito grande
    escalaMiniatura[ID_MIRANHA] = 1.5f;
    
    // Escalas do Painel 
    escalaPainel[ID_BEN10] = 3.0f;
    escalaPainel[ID_DOCINHO] = 2.0f; // Docinho é larga
    escalaPainel[ID_TAZ] = 1.0f;     // Taz é muito grande
    escalaPainel[ID_MIRANHA] = 3.0f;
}

void DescarregarRecursosPersonagens(void) {
    UnloadTexture(texBen10);
    UnloadTexture(texDocinho);
    UnloadTexture(texMiranhathumb);
    UnloadTexture(texTaz);
}

void AtualizarTelaPersonagens(GameScreen *telaAtual, int *personagemSelecionado) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        *telaAtual = SCREEN_MENU;
    }

    // Lógica de seleção de personagem
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
        for (int i = 0; i < 9; i++) {
            if (CheckCollisionPointRec(mousePos, rectPersonagens[i])) {
                *personagemSelecionado = i;
                break; 
            }
        }
    }
}

static void DesenharPainelDetalhes(int idPersonagem) {
    Rectangle painelDireito = { 1300, 100, 580, 880 };
    
    DrawRectangleRec(painelDireito, corPainel);
    DrawRectangleLinesEx(painelDireito, 5.0f, corBordaPainel);
    
    Rectangle areaImagem = { painelDireito.x + 40, painelDireito.y + 40, painelDireito.width - 80, 300 };

    int posX = (int)painelDireito.x + 40;
    int posYBase = (int)areaImagem.y + (int)areaImagem.height + 20; 
    int tamFonteTitulo = 30;
    int tamFonteTexto = 25;

    int espacamentoLinha1 = 40;
    int espacamentoLinha2 = 70;
    int espacamentoLinha3 = 95;
    int espacamentoBloco = 150; 

    Texture2D texturaParaDesenhar;
    bool desenharTextura = false;
    
    const char *nome = "???";
    const char *intro = "Selecione um personagem...";
    const char *ataque1 = "-";
    const char *ataque2 = "-";
    const char *pv = "???";

    switch (idPersonagem) {
        case ID_BEN10:
            nome = "Ben 10";
            intro = "Garoto com relógio esquisito";
            ataque1 = "Barragem de Quatro Braços->10ATK";
            ataque2 = "Chama frenética->5ATK";
            pv = "55";
            texturaParaDesenhar = texBen10;
            desenharTextura = true;
            break;
            
        case ID_DOCINHO:
            nome = "Docinho";
            intro = "A mais doce das Meninas super poderosas";
            ataque1 = "socada doce->13ATK";
            ataque2 = "Laser carinhoso->6ATK";
            pv = "42";
            texturaParaDesenhar = texDocinho;
            desenharTextura = true;
            break;

        case ID_TAZ:
            nome = "Taz Mania";
            intro = "Demônio da Tasmânia.";
            ataque1 = "Tornado->11ATK";
            ataque2 = "Mordida->5ATK";
            pv = "50";
            texturaParaDesenhar = texTaz;
            desenharTextura = true;
            break;
            
        case ID_MIRANHA:
            nome = "Homem-Aranha";
            intro = "O amigo da vizinhança.";
            ataque1 = "Soco-teia";
            ataque2 = "Sentido Aranha (Esquiva)";
            pv = "35";
            texturaParaDesenhar = texMiranhathumb;
            desenharTextura = true;
            break;

        case ID_HELLO_KITTY:
            nome = "Hello Kitty";
            intro = "A gatinha mais famosa.";
            ataque1 = "Kityy surpresa(rolo compressor) - > 8ATK";
            ataque2 = "??? -> 4ATK";
            pv = "70";
            //adicionar a textura

            break; 

        case ID_SHREK:
            nome = "Shrek";
            intro = "O ogro mais gente fina dos pântanos";
            ataque1 = "Bofetada ->5ATK";
            ataque2 = "Arrogr5 -> 2ATK";
            pv = "65";
            //adicionar a textura
            break;
        case ID_MONICA:
            nome = "";
            intro = "";
            ataque1 = "Coelhada -> 7ATK";
            ataque2 = "Soco Super-mônico -> 3ATK";
            pv = "64";
            //adicionar a textura
            break;
        case ID_ELSA:
            nome = "Elsa";
            intro = "";
            ataque1 = "Fúria Glacial -> 15ATK";
            ataque2 = "Tempestade congelante - >7ATK";
            pv = "30";
            //adicionar a textura
            break;
        case ID_MICKEY:
            nome = "Mickey Mouse";
            intro = "";
            ataque1 = "??? ->11ATK";
            ataque2 = "??? -> 6ATK";
            pv = "40";
            //adicionar a textura
            break;
        default:
            break;
    }
    
    // Desenha a foto do personagem 
    if (desenharTextura == true) {
        
        float escala = escalaPainel[idPersonagem]; 
        
        float imgX = areaImagem.x + (areaImagem.width - (texturaParaDesenhar.width * escala)) / 2;
        float imgY = areaImagem.y + (areaImagem.height - (texturaParaDesenhar.height * escala)) / 2;
        DrawTextureEx(texturaParaDesenhar, (Vector2){ imgX, imgY }, 0.0f, escala, WHITE);
    }

    // Desenha os textos
    DrawText("Nome:", posX, posYBase, tamFonteTitulo, corTituloLinha);
    DrawText(nome, posX, posYBase + espacamentoLinha1, tamFonteTexto, LIGHTGRAY);
    DrawText(intro, posX, posYBase + espacamentoLinha2, 20, LIGHTGRAY); 
    
    int posYBlocoAtaques = posYBase + espacamentoBloco;
    DrawText("Ataques:", posX, posYBlocoAtaques, tamFonteTitulo, corTituloLinha);
    DrawText(ataque1, posX, posYBlocoAtaques + espacamentoLinha1, tamFonteTexto, LIGHTGRAY);
    DrawText(ataque2, posX, posYBlocoAtaques + espacamentoLinha3, tamFonteTexto, LIGHTGRAY);
    
    int posYBlocoPV = posYBase + (espacamentoBloco * 2);
    DrawText("PV:", posX, posYBlocoPV, tamFonteTitulo, corTituloLinha);
    DrawText(pv, posX, posYBlocoPV + espacamentoLinha1, tamFonteTexto, LIGHTGRAY);
}


void DesenharTelaPersonagens(int personagemSelecionado) {
    ClearBackground(DARKGRAY);
    
    DesenharPainelDetalhes(personagemSelecionado);

    DrawText("Personagens:", 50, 50, 60, corTituloLinha);

    int tamFonteTituloLinha = 40;
    int tamFonteNome = 30;
    
    
    int linhaFrenteY = (int)rectPersonagens[ID_HELLO_KITTY].y;
    int linhaMeioY = (int)rectPersonagens[ID_BEN10].y;
    int linhaTrasY = (int)rectPersonagens[ID_MIRANHA].y;
    
    int coluna1X = (int)rectPersonagens[ID_HELLO_KITTY].x;
    int coluna2X = (int)rectPersonagens[ID_SHREK].x;
    int coluna3X = (int)rectPersonagens[ID_MONICA].x;
    
    Color cor; 

    // --- Linha da Frente ---
    DrawText("Linha da Frente:", coluna1X, linhaFrenteY - 60, tamFonteTituloLinha, corTituloLinha);
    
    if (personagemSelecionado == ID_HELLO_KITTY) { 
        cor = corNomeSelecionado;
    }else { 
        cor = corNomePersonagem;
    }
    DrawText("-Hello Kitty", coluna1X, linhaFrenteY, tamFonteNome, cor);
    
    if (personagemSelecionado == ID_SHREK) { 
        cor = corNomeSelecionado; 
    }else { 
        cor = corNomePersonagem; 
    }
    DrawText("-Shrek", coluna2X, linhaFrenteY, tamFonteNome, cor);
    
    if (personagemSelecionado == ID_MONICA){ 
        cor = corNomeSelecionado; 
    }else{ 
        cor = corNomePersonagem; 
    }
    DrawText("-Mônica", coluna3X, linhaFrenteY, tamFonteNome, cor);

    // --- Linha do Meio ---
    DrawText("Linha do Meio:", coluna1X, linhaMeioY - 60, tamFonteTituloLinha, corTituloLinha);
    
    if (personagemSelecionado == ID_BEN10) {
        cor = corNomeSelecionado; 
    }else { 
        cor = corNomePersonagem; 
    }
    DrawText("-Ben 10", coluna1X, linhaMeioY, tamFonteNome, cor);
    DrawTextureEx(texBen10, (Vector2){ (float)coluna1X + 40, (float)linhaMeioY + 40 }, 0.0f, escalaMiniatura[ID_BEN10], WHITE);
    
    if (personagemSelecionado == ID_DOCINHO) { 
        cor = corNomeSelecionado; 
    }else { 
        cor = corNomePersonagem; 
    }
    DrawText("-Docinho", coluna2X, linhaMeioY, tamFonteNome, cor);
    DrawTextureEx(texDocinho, (Vector2){ (float)coluna2X + 40, (float)linhaMeioY + 40 }, 0.0f, escalaMiniatura[ID_DOCINHO], WHITE);

    if (personagemSelecionado == ID_TAZ) { cor = corNomeSelecionado; } 
    else { cor = corNomePersonagem; }
    DrawText("-Taz mania", coluna3X, linhaMeioY, tamFonteNome, cor);
    DrawTextureEx(texTaz, (Vector2){ (float)coluna3X + 40, (float)linhaMeioY + 40 }, 0.0f, escalaMiniatura[ID_TAZ], WHITE);

    // --- Linha de Trás ---
    DrawText("Linha de Trás:", coluna1X, linhaTrasY - 60, tamFonteTituloLinha, corTituloLinha);

    if (personagemSelecionado == ID_MIRANHA) { 
        cor = corNomeSelecionado; 
    }else{ 
        cor = corNomePersonagem; 
    }
    DrawText("-Homem-Aranha", coluna1X, linhaTrasY, tamFonteNome, cor);
    DrawTextureEx(texMiranhathumb, (Vector2){ (float)coluna1X + 40, (float)linhaTrasY + 40 }, 0.0f, escalaMiniatura[ID_MIRANHA], WHITE);

    if (personagemSelecionado == ID_ELSA) { 
        cor = corNomeSelecionado; 
    }else {
        cor = corNomePersonagem; 
    }
    DrawText("-Elsa", coluna2X, linhaTrasY, tamFonteNome, cor);
    
    if (personagemSelecionado == ID_MICKEY) { 
        cor = corNomeSelecionado; 
    }else {
         cor = corNomePersonagem; 
    }
    DrawText("-Mickey mouse", coluna3X, linhaTrasY, tamFonteNome, cor);
    
    const char *aviso = "Pressione ESC ou ENTER para voltar ao Menu";
    int tamAviso = 20;
    int posXAviso = (SCREEN_WIDTH - MeasureText(aviso, tamAviso)) / 2;
    DrawText(aviso, posXAviso, SCREEN_HEIGHT - 40, tamAviso, LIGHTGRAY);
}