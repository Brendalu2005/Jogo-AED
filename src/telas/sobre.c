#include "raylib.h"
#include "sobre.h"
#include "menu.h"
#include "telas.h"

// A função AtualizarTelaSobre não precisa mudar
void AtualizarTelaSobre(GameScreen *telaAtual){
    if(IsKeyPressed(KEY_ESCAPE)){
        *telaAtual = SCREEN_MENU;
    }

    Vector2 mouse = GetMouseVirtual();
    Rectangle btnVoltar = { 50, 50, 150, 50 };

    
    if (CheckCollisionPointRec(mouse, btnVoltar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
        *telaAtual = SCREEN_MENU;
    }
}


void DesenharTelaSobre(MenuOpcao resoucers) {
    DrawTexture(resoucers.background, 0, 0, WHITE);

    
    char titulo[] = "Sobre o Jogo";
    int fontSizeTitulo = 60;
    float spacingTitulo = 2.0f; 
    float spacingTexto = 1.0f;
    float spacingSecao = 2.0f;

    
    Vector2 tituloSize = MeasureTextEx(resoucers.fontPressStart, titulo, (float)fontSizeTitulo, spacingTitulo);
    int posXtitulo = (SCREEN_WIDTH - tituloSize.x) / 2;

    
    DrawTextEx(resoucers.fontPressStart, titulo, (Vector2){ (float)posXtitulo + 3, 123.0f }, (float)fontSizeTitulo, spacingTitulo, (Color){0,0,0,150}); // Sombra
    DrawTextEx(resoucers.fontPressStart, titulo, (Vector2){ (float)posXtitulo, 120.0f }, (float)fontSizeTitulo, spacingTitulo, RAYWHITE);


    
    int posXtexto = 250;
    int posYtexto = 210; 


    int lineSpacing = 18;       
    int sectionSpacing = 4;      
    int indent = 20; 
    
    int fontSizeTexto = 16;       
    int fontSizeTituloSecao = 18;      
   

    Color corTituloSecao = (Color){100, 255, 100, 255}; 
    Color corTextoComum = LIGHTGRAY;

    
    
    Rectangle fundoPreto = { (float)posXtexto - 30, (float)posYtexto - 20, (float)SCREEN_WIDTH - (posXtexto * 2) + 60, 680.0f };
    DrawRectangleRounded(fundoPreto, 0.1f, 8, (Color){ 0, 0, 0, 180 }); 

    

    DrawTextEx(resoucers.fontPressStart, "Ecos da Infancia", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTituloSecao, spacingSecao, corTituloSecao);
    posYtexto += lineSpacing + sectionSpacing;

    
    DrawTextEx(resoucers.fontPressStart, "Este jogo nasceu da ideia de misturar personagens", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "classicos da nossa infancia com a mecanica de jogos", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "de turno, como Darkest Dungeon.", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing + sectionSpacing;

    DrawTextEx(resoucers.fontPressStart, "Historia:", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTituloSecao, spacingSecao, corTituloSecao);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "A historia se passa dentro da mente de Lucy, uma", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "mulher de 25 anos que sempre tinha nostalgia,", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "relembrando como sua infancia era boa e repleta", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "de personagens que ela amava.", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing; 

    DrawTextEx(resoucers.fontPressStart, "Certa noite, ela sonha com uma brincadeira que", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "fazia com o irmao: a briga de bonecos.", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "No sonho, ela escolhe 3 personagens para lutar", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "contra 3 que o irmao dela escolheu (controlados", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "pela IA). Quem conseguir derrotar os 3 personagens", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "do outro primeiro, ganha.", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing; 

    DrawTextEx(resoucers.fontPressStart, "Nos, da equipe de desenvolvimento, esperamos que", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "voce se divirta jogando o nosso jogo de turno,", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "pensado para trazer memorias da sua infancia.", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing + sectionSpacing;

    DrawTextEx(resoucers.fontPressStart, "Creditos:", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTituloSecao, spacingSecao, corTituloSecao);
    posYtexto += lineSpacing;

    DrawTextEx(resoucers.fontPressStart, "Desenvolvimento:", (Vector2){ (float)posXtexto + indent, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "Augusto Malheiros, Brenda Luana e", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "Eduardo Albuquerque", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing + sectionSpacing;

    DrawTextEx(resoucers.fontPressStart, "Projeto Academico:", (Vector2){ (float)posXtexto + indent, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "Este jogo foi criado para a disciplina de", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "Algoritmos e Estruturas de Dados (AED),", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "da professora Natacha Targino.", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing; 

    DrawTextEx(resoucers.fontPressStart, "O objetivo era criar um jogo em C que utilizasse", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "estruturas de dados (Listas Duplamente Encadeadas)", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "e algoritmos de ordenacao, alem de uma API de", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "IA (Gemini) para a logica do oponente.", (Vector2){ (float)posXtexto + indent * 2, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing + sectionSpacing;

    DrawTextEx(resoucers.fontPressStart, "Musicas:", (Vector2){ (float)posXtexto, (float)posYtexto }, (float)fontSizeTituloSecao, spacingSecao, corTituloSecao);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "Menu: The perfect pair - beabadoobee", (Vector2){ (float)posXtexto + indent, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "Solo: Freaking out the neighborhood - Mac DeMarco", (Vector2){ (float)posXtexto + indent, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawTextEx(resoucers.fontPressStart, "PvP: I really want to stay at your house - Rosa Walton", (Vector2){ (float)posXtexto + indent, (float)posYtexto }, (float)fontSizeTexto, spacingTexto, corTextoComum);
    


   Rectangle btnVoltar = { 50, 50, 150, 50 };
    Vector2 mouse = GetMouseVirtual();
    Color tintVoltar = CheckCollisionPointRec(mouse, btnVoltar) ? LIGHTGRAY : WHITE;

    DrawTexturePro(
        resoucers.btnVoltarTex,
        (Rectangle){ 0, 0, resoucers.btnVoltarTex.width, resoucers.btnVoltarTex.height },
        btnVoltar,
        (Vector2){ 0, 0 },
        0,
        tintVoltar
    );

}