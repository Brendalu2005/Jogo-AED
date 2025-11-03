#include "raylib.h"
#include "telas.h"
#include "menu.h" 
#include "batalha.h"    
#include "personagens.h" 
#include "database.h"     
#include "selecao.h" 

void AtualizarTelaPlaceholder(GameScreen *telaAtual);
void DesenharTelaPlaceholder(const char *titulo);

int main(void) {
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ecos da Infância");
    SetTargetFPS(60);

    MenuOpcao menuRes = LoadMenuResources();
    CarregarRecursosPersonagens(); 
    
    SpriteDatabase database = CarregarDatabase("sprites/personagens_db.json"); 

    TimesBatalha timesSelecionados = {0};
    EstadoBatalha estadoBatalha = {0};
    
    GameScreen telaAtual = SCREEN_MENU;
    int personagemSelecionado = -1; // -1 significa "nenhum selecionado"

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

        BeginDrawing();
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

        EndDrawing();
    }

    LiberarDatabase(&database); 
    // DescarregarRecursosPersonagens() está vazio, não precisamos chamar
    UnloadMenuResources(menuRes);
    CloseWindow();
    
    return 0;
}


// Funções placeholder originais
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