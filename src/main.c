#include "raylib.h"
#include "telas.h"
#include "menu.h" 
#include "batalha.h"    

void AtualizarTelaPlaceholder(GameScreen *telaAtual);
void DesenharTelaPlaceholder(const char *titulo);

int main(void) {
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ecos da Infância");
    SetTargetFPS(60);

    MenuOpcao menuRes = LoadMenuResources();

    GameScreen telaAtual = SCREEN_MENU;
    int ataqueSelecionado = 0;

    while (!WindowShouldClose() && telaAtual != SCREEN_SAIR) {
        

        switch(telaAtual) {
            case SCREEN_MENU:
                AtualizarTelaMenu(&telaAtual);
                break;
            case SCREEN_BATALHA:
                AtualizarTelaBatalha(&ataqueSelecionado, &telaAtual);
                break;
            case SCREEN_PERSONAGENS:
            case SCREEN_SOBRE:
                AtualizarTelaPlaceholder(&telaAtual); 
                break;
            default: break;
        }

        BeginDrawing();
        ClearBackground(DARKGRAY); 

        switch(telaAtual) {
            case SCREEN_MENU:
                DesenharTelaMenu(menuRes);
                break;
            case SCREEN_BATALHA:
                DesenharTelaBatalha(ataqueSelecionado);
                break;
            case SCREEN_PERSONAGENS:
                DesenharTelaPlaceholder("PERSONAGENS"); //nada ainda
                break;
            case SCREEN_SOBRE:
                DesenharTelaPlaceholder("SOBRE"); //nada ainda
                
                break;
            default: break;
        }

        EndDrawing();
    }

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