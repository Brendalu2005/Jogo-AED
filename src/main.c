#include "raylib.h"
#include "telas.h"
#include "menu.h" 
#include "batalha.h"    
#include "personagens.h" 
#include "database.h"     
#include "selecao.h" 

void AtualizarTelaPlaceholder(GameScreen *telaAtual);
void DesenharTelaPlaceholder(const char *titulo);

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
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ecos da Infância");

    ToggleFullscreen();

    int monitorAtual = GetCurrentMonitor();
    int larguraNativa = GetMonitorWidth(monitorAtual);
    int alturaNativa = GetMonitorHeight(monitorAtual);

    SetWindowState(FLAG_WINDOW_UNDECORATED); 

    SetWindowSize(larguraNativa, alturaNativa);
    
    SetWindowPosition(0, 0);

    RenderTexture2D canvas = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    escala = (float)larguraNativa / SCREEN_WIDTH;
    if ((float)alturaNativa / SCREEN_HEIGHT < escala) {
        escala = (float)alturaNativa / SCREEN_HEIGHT;
    }

    areaDestinoCanvas.width = SCREEN_WIDTH * escala;
    areaDestinoCanvas.height = SCREEN_HEIGHT * escala;
    areaDestinoCanvas.x = (larguraNativa - areaDestinoCanvas.width) * 0.5f;
    areaDestinoCanvas.y = (alturaNativa - areaDestinoCanvas.height) * 0.5f;
    
    Vector2 origemCanvas = { 0.0f, 0.0f };
    Rectangle retanguloFonteCanvas = { 0.0f, 0.0f, (float)canvas.texture.width, (float)-canvas.texture.height };


    SetTargetFPS(60);

    MenuOpcao menuRes = LoadMenuResources();
    CarregarRecursosPersonagens();
    
    SpriteDatabase database = CarregarDatabase("sprites/personagens_db.json");

    TimesBatalha timesSelecionados = {0};
    EstadoBatalha estadoBatalha = {0};
    
    GameScreen telaAtual = SCREEN_MENU;
    int personagemSelecionado = -1;

    while (!WindowShouldClose() && telaAtual != SCREEN_SAIR) {
        
        switch(telaAtual) {
            case SCREEN_MENU:
                AtualizarTelaMenu(&telaAtual);
                if (telaAtual == SCREEN_SELECAO) {
                    InicializarSelecao(&timesSelecionados);
                }
                break;
            case SCREEN_SELECAO:
                AtualizarTelaSelecao(&telaAtual, &database, &timesSelecionados); 
                if (telaAtual == SCREEN_BATALHA) {
                    InicializarBatalha(&estadoBatalha, &timesSelecionados);
                }
                break;
            case SCREEN_BATALHA:
                AtualizarTelaBatalha(&estadoBatalha, &telaAtual); 
                break;
            case SCREEN_PERSONAGENS:
                AtualizarTelaPersonagens(&telaAtual, &personagemSelecionado, &database); 
                break;
            case SCREEN_SOBRE:
                AtualizarTelaPlaceholder(&telaAtual); 
                break;
            case SCREEN_SAIR:
                break;
        }
        

        BeginTextureMode(canvas); 
            ClearBackground(DARKGRAY);

            switch(telaAtual) {
                case SCREEN_MENU:
                    DesenharTelaMenu(menuRes);
                    break;
                case SCREEN_SELECAO:
                    DesenharTelaSelecao(&database, &timesSelecionados);
                    break;
                case SCREEN_BATALHA:
                    DesenharTelaBatalha(&estadoBatalha); 
                    break;
                case SCREEN_PERSONAGENS:
                    DesenharTelaPersonagens(personagemSelecionado, &database); 
                    break;
                case SCREEN_SOBRE:
                    DesenharTelaPlaceholder("SOBRE"); 
                    break;
                case SCREEN_SAIR:
                    break;
            }

        EndTextureMode(); 

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