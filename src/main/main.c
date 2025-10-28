#include "raylib.h"
#include "tela.h" 

int main(void){

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ecos da infância");
    SetTargetFPS(60);

    int ataqueSelecionado = 0;
    while (!WindowShouldClose()){
        
        AtualizarTelaBatalha(&ataqueSelecionado);

        BeginDrawing();
        ClearBackground(DARKGRAY); 
        DesenharTelaBatalha(ataqueSelecionado);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}