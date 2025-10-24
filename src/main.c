#include "raylib.h"
#include "tela.h" 

const int screenWidth = 1280;
const int screenHeight = 720;

int main(void){
    InitWindow(screenWidth, screenHeight, "Meu Jogo AED - Batalha");
    SetTargetFPS(60);

    while (!WindowShouldClose()){
        
        BeginDrawing();

        ClearBackground(DARKGRAY); 

        DesenharTelaBatalha();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}