#include "raylib.h"
#include "telas.h"
#include "menu.h" 
#include "batalha.h"    
#include "personagens.h" 
#include "database.h"     
#include "selecao.h" 
#include "sobre.h"
#include "modo_jogo.h"
#include <stdlib.h> 
#include <time.h> 

Texture2D backgroundSelecao;

void AtualizarTelaPlaceholder(GameScreen *telaAtual);
void DesenharTelaPlaceholder(const char *titulo);

// Estas variáveis globais estáticas são necessárias para GetMouseVirtual
static float escala = 1.0f;
static Rectangle areaDestinoCanvas = { 0 };

// Implementação da função declarada em telas.h
Vector2 GetMouseVirtual(void) {
    Vector2 mouseNativo = GetMousePosition();
    Vector2 mouseVirtual = { 0 };
    
    // Traduz as coordenadas da tela nativa para as coordenadas do canvas
    mouseVirtual.x = (mouseNativo.x - areaDestinoCanvas.x) / escala;
    mouseVirtual.y = (mouseNativo.y - areaDestinoCanvas.y) / escala;
    
    return mouseVirtual;
}


int main(void) {
    
    // --- LÓGICA DE JANELA MODIFICADA (PARA FULLSCREEN CORRETO) ---

    // Pega as dimensões do monitor principal ANTES de criar a janela
    int monitorAtual = GetCurrentMonitor();
    int larguraNativa = GetMonitorWidth(monitorAtual);
    int alturaNativa = GetMonitorHeight(monitorAtual);

    // Configura a janela para NÃO ter borda (borderless)
    SetConfigFlags(FLAG_WINDOW_UNDECORATED); 
    
    // Inicializa a janela JÁ com o tamanho total do ecrã
    InitWindow(larguraNativa, alturaNativa, "Ecos da Infância");
    
    srand(time(NULL));
    // Garante a posição no canto superior esquerdo
    SetWindowPosition(0, 0);
    SetTargetFPS(60);
    
    // --- FIM DA MODIFICAÇÃO ---

    // Carrega o canvas na resolução-alvo (1600x900)
    RenderTexture2D canvas = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Define a origem e a fonte do canvas (não muda)
    Vector2 origemCanvas = { 0.0f, 0.0f };
    Rectangle retanguloFonteCanvas = { 0.0f, 0.0f, (float)canvas.texture.width, (float)-canvas.texture.height };

    // Carrega recursos
    MenuOpcao menuRes = LoadMenuResources();
    CarregarRecursosPersonagens();
    backgroundSelecao = LoadTexture("sprites/background/background3.jpg");
    SpriteDatabase database = CarregarDatabase("sprites/personagens_db.json");

    // Inicializa estados
    TimesBatalha timesSelecionados = {0};
    EstadoBatalha estadoBatalha = {0};
    GameScreen telaAtual = SCREEN_MENU;
    int personagemSelecionado = -1;
    ModoDeJogo modoDeJogoAtual = MODO_SOLO; 

    while (!WindowShouldClose() && telaAtual != SCREEN_SAIR) {
        
        // --- Atualização das Telas ---
        switch(telaAtual) {
            case SCREEN_MENU:
                AtualizarTelaMenu(&telaAtual);
                if (telaAtual == SCREEN_SELECAO) {
                    InicializarSelecao(&timesSelecionados);
                }
                break;
            case SCREEN_SELECAO:
                AtualizarTelaSelecao(&telaAtual, &database, &timesSelecionados, modoDeJogoAtual); 
                if (telaAtual == SCREEN_BATALHA) {
                    InicializarBatalha(&estadoBatalha, &timesSelecionados);
                }
                break;
            case SCREEN_BATALHA:
                AtualizarTelaBatalha(&estadoBatalha, &telaAtual, modoDeJogoAtual); 
                break;
            case SCREEN_PERSONAGENS:
                AtualizarTelaPersonagens(&telaAtual, &personagemSelecionado, &database); 
                break;
            case SCREEN_SOBRE:
                AtualizarTelaSobre(&telaAtual); 
                break;
            case SCREEN_MODO_JOGO:
                AtualizarTelaModoJogo(&telaAtual, &modoDeJogoAtual);
                break;
            case SCREEN_SAIR:
                break;
        }
        
        // --- Desenho no Canvas (1600x900) ---
        BeginTextureMode(canvas); 
            ClearBackground(DARKGRAY);

            switch(telaAtual) {
                case SCREEN_MENU:
                    DesenharTelaMenu(menuRes);
                    break;
                case SCREEN_SELECAO:
                    DrawTexture(backgroundSelecao, 0, 0, WHITE);
                    DesenharTelaSelecao(&database, &timesSelecionados, modoDeJogoAtual);
                    break;
                case SCREEN_BATALHA:
                    DesenharTelaBatalha(&estadoBatalha); 
                    break;
                case SCREEN_PERSONAGENS:
                    DesenharTelaPersonagens(personagemSelecionado, &database); 
                    break;
                case SCREEN_SOBRE:
                    DesenharTelaSobre(menuRes); 
                    break;
                case SCREEN_MODO_JOGO:
                    DesenharTelaModoJogo(menuRes);
                    break;
                case SCREEN_SAIR:
                    break;
            }

        EndTextureMode(); 

        // --- CÁLCULO DE ESCALA (JÁ ESTÁ CORRETO) ---
        larguraNativa = GetScreenWidth();
        alturaNativa = GetScreenHeight();
        
        escala = (float)larguraNativa / SCREEN_WIDTH;
        if ((float)alturaNativa / SCREEN_HEIGHT < escala) {
            escala = (float)alturaNativa / SCREEN_HEIGHT;
        }

        areaDestinoCanvas.width = SCREEN_WIDTH * escala;
        areaDestinoCanvas.height = SCREEN_HEIGHT * escala;
        areaDestinoCanvas.x = (larguraNativa - areaDestinoCanvas.width) * 0.5f;
        areaDestinoCanvas.y = (alturaNativa - areaDestinoCanvas.height) * 0.5f;

        // --- Desenho na Tela (Ecrã) ---
        BeginDrawing();
            ClearBackground(BLACK);
            
            DrawTexturePro(
                canvas.texture,       
                retanguloFonteCanvas, 
                areaDestinoCanvas, 
                origemCanvas,        
                0.0f,                 
                WHITE               
            );
            
        EndDrawing();   
    }

    // --- Limpeza ---
    UnloadTexture(backgroundSelecao);
    UnloadRenderTexture(canvas); 
    LiberarDatabase(&database); 
    UnloadMenuResources(menuRes);
    CloseWindow();
    
    return 0;
}


void AtualizarTelaPlaceholder(GameScreen *telaAtual) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        *telaAtual = SCREEN_MENU;
    }
}

void DesenharTelaPlaceholder(const char *titulo) {
    ClearBackground(DARKGRAY);
    int tamTitulo = 60;
    int posXtitulo = (SCREEN_WIDTH - MeasureText(titulo, tamTitulo)) / 2;
    
    DrawText(titulo, posXtitulo, 300, tamTitulo, LIGHTGRAY);
    
    const char *aviso = "Pressione ESC ou ENTER para voltar ao Menu";
    int tamAviso = 20;
    int posXAviso = (SCREEN_WIDTH - MeasureText(aviso, tamAviso)) / 2;
    
    DrawText(aviso, posXAviso, 400, tamAviso, LIGHTGRAY);
}