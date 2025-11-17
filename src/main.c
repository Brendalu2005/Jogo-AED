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
#include "batalha_desenho.h"

Texture2D backgroundSelecao;

void AtualizarTelaPlaceholder(GameScreen *telaAtual);
void DesenharTelaPlaceholder(const char *titulo);

static float escala = 1.0f;
static Rectangle areaDestinoCanvas = { 0 };

static Music musicaMenu;
static Music musicaBatalhaSolo;
static Music musicaBatalhaPVP;

static Music *musicaAtiva = NULL;

Vector2 GetMouseVirtual(void) {
    Vector2 mouseNativo = GetMousePosition();
    Vector2 mouseVirtual = { 0 };
    
    mouseVirtual.x = (mouseNativo.x - areaDestinoCanvas.x) / escala;
    mouseVirtual.y = (mouseNativo.y - areaDestinoCanvas.y) / escala;
    
    return mouseVirtual;
}


int main(void) {
    
    

    int monitorAtual = GetCurrentMonitor();
    int larguraNativa = GetMonitorWidth(monitorAtual);
    int alturaNativa = GetMonitorHeight(monitorAtual);

    SetConfigFlags(FLAG_WINDOW_UNDECORATED); 
    
    InitWindow(larguraNativa, alturaNativa, "Ecos da Infância");
    
    srand(time(NULL));
    
    SetWindowPosition(0, 0);
    SetTargetFPS(60);
    

    RenderTexture2D canvas = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    Vector2 origemCanvas = { 0.0f, 0.0f };
    Rectangle retanguloFonteCanvas = { 0.0f, 0.0f, (float)canvas.texture.width, (float)-canvas.texture.height };

    // Carrega recursos
    MenuOpcao menuRes = LoadMenuResources();
    CarregarRecursosPersonagens();
    backgroundSelecao = LoadTexture("sprites/background/background3.jpg");
    SpriteDatabase database = CarregarDatabase("sprites/personagens_db.json");


    InitAudioDevice();

    musicaMenu = LoadMusicStream("musicas/The-Perfect_Pair.mp3");
    musicaBatalhaSolo = LoadMusicStream("musicas/Freaking_Out_The_Neighborhood.mp3");
    musicaBatalhaPVP = LoadMusicStream("musicas/I_Really_Want_To_Stay_In_Your_House.mp3");

    musicaMenu.looping = true;
    musicaBatalhaSolo.looping = true;
    musicaBatalhaPVP.looping = true;

    float volumeGeral = 0.5f;
    SetMusicVolume(musicaMenu, volumeGeral);
    SetMusicVolume(musicaBatalhaSolo, volumeGeral);
    SetMusicVolume(musicaBatalhaPVP, volumeGeral);

    musicaAtiva = &musicaMenu;
    PlayMusicStream(*musicaAtiva);

    TimesBatalha timesSelecionados = {0};
    EstadoBatalha estadoBatalha = {0};
    GameScreen telaAtual = SCREEN_MENU;
    int personagemSelecionado = -1;
    ModoDeJogo modoDeJogoAtual = MODO_SOLO; 

    while (!WindowShouldClose() && telaAtual != SCREEN_SAIR) {
        if (musicaAtiva != NULL) {
            if (IsMusicStreamPlaying(*musicaAtiva)) {
                UpdateMusicStream(*musicaAtiva);
            }
        }

        if (telaAtual == SCREEN_MENU || telaAtual == SCREEN_PERSONAGENS || telaAtual == SCREEN_SOBRE || telaAtual == SCREEN_MODO_JOGO || telaAtual == SCREEN_SELECAO) {
            
            if (musicaAtiva != &musicaMenu) {
                if (musicaAtiva != NULL) {
                    StopMusicStream(*musicaAtiva); 
                }
                musicaAtiva = &musicaMenu; 
                PlayMusicStream(*musicaAtiva); 
            }
        }

        if (telaAtual == SCREEN_BATALHA) {
            
            if (modoDeJogoAtual == MODO_SOLO) {
                if (musicaAtiva != &musicaBatalhaSolo) {
                    if (musicaAtiva != NULL) {
                        StopMusicStream(*musicaAtiva);
                    }
                    musicaAtiva = &musicaBatalhaSolo;
                    PlayMusicStream(*musicaAtiva);
                }
            } else {
                if (musicaAtiva != &musicaBatalhaPVP) {
                    if (musicaAtiva != NULL) {
                        StopMusicStream(*musicaAtiva);
                    }
                    musicaAtiva = &musicaBatalhaPVP;
                    PlayMusicStream(*musicaAtiva);
                }
            }
        }
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
                    DesenharTelaBatalha(&estadoBatalha, modoDeJogoAtual); 
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

    UnloadMusicStream(musicaMenu);
    UnloadMusicStream(musicaBatalhaSolo);
    UnloadMusicStream(musicaBatalhaPVP);

    
    UnloadTexture(backgroundSelecao);
    UnloadRenderTexture(canvas); 
    LiberarDatabase(&database); 
    UnloadMenuResources(menuRes);
    
    CloseAudioDevice();
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