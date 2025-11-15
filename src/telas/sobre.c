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
    int posXtitulo = (SCREEN_WIDTH - MeasureText(titulo, fontSizeTitulo)) / 2;
    DrawText(titulo, posXtitulo + 3, 123, fontSizeTitulo, (Color){0,0,0,150}); // Sombra
    DrawText(titulo, posXtitulo, 120, fontSizeTitulo, RAYWHITE);


    
    int posXtexto = 250;
    int posYtexto = 210; 
    int lineSpacing = 24; 
    int sectionSpacing = 8; 
    int indent = 20; 
    
    int fontSizeTexto = 18; 
    int fontSizeTituloSecao = 20; 
    Color corTituloSecao = (Color){100, 255, 100, 255}; 
    Color corTextoComum = LIGHTGRAY;

    
    Rectangle fundoPreto = { (float)posXtexto - 30, (float)posYtexto - 20, (float)SCREEN_WIDTH - (posXtexto * 2) + 60, 680.0f };
    DrawRectangleRounded(fundoPreto, 0.1f, 8, (Color){ 0, 0, 0, 180 }); 

    

    DrawText("Ecos da Infância", posXtexto, posYtexto, fontSizeTituloSecao, corTituloSecao);
    posYtexto += lineSpacing + sectionSpacing;

    DrawText("Este jogo nasceu da ideia de misturar personagens clássicos da nossa infância", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("com a mecânica de jogos de turno, como Darkest Dungeon.", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing + sectionSpacing;

    DrawText("História:", posXtexto, posYtexto, fontSizeTituloSecao, corTituloSecao);
    posYtexto += lineSpacing;
    DrawText("A história se passa dentro da mente de Lucy, uma mulher de 25 anos que sempre", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("tinha nostalgia, relembrando como sua infância era boa e repleta de personagens", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("que ela amava.", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing; 

    DrawText("Certa noite, ela sonha com uma brincadeira que fazia com o irmão: a briga de bonecos.", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("No sonho, ela escolhe 3 personagens para lutar contra 3 personagens que o irmão", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("dela escolheu (controlados pela IA). Quem conseguir derrotar os 3 personagens", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("do outro primeiro, ganha.", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing; 

    DrawText("Nós, da equipe de desenvolvimento, esperamos que você se divirta jogando o nosso", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("jogo de turno, que foi pensado em cada detalhe para trazer memórias da sua infância.", posXtexto, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing + sectionSpacing;

    DrawText("Créditos:", posXtexto, posYtexto, fontSizeTituloSecao, corTituloSecao);
    posYtexto += lineSpacing;

    DrawText("Desenvolvimento:", posXtexto + indent, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("Augusto Malheiros, Brenda Luana e Eduardo Albuquerque", posXtexto + indent * 2, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing + sectionSpacing;

    DrawText("Projeto Acadêmico:", posXtexto + indent, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("Este jogo foi criado para a disciplina de Algoritmos e Estruturas de Dados (AED),", posXtexto + indent * 2, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("da professora Natacha Targino.", posXtexto + indent * 2, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing; 

    DrawText("O objetivo era criar um jogo em C que utilizasse estruturas de dados (como Listas", posXtexto + indent * 2, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("Duplamente Encadeadas) e algoritmos de ordenação, além de uma API de IA (Gemini)", posXtexto + indent * 2, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("para a lógica do oponente.", posXtexto + indent * 2, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing + sectionSpacing;

    DrawText("Músicas:", posXtexto, posYtexto, fontSizeTituloSecao, corTituloSecao);
    posYtexto += lineSpacing;
    DrawText("Música do Menu: The perfect pair - beabadoobee", posXtexto + indent, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("Música da parte Solo: Freaking out the neighborhood - Mac DeMarco", posXtexto + indent, posYtexto, fontSizeTexto, corTextoComum);
    posYtexto += lineSpacing;
    DrawText("Música da parte PvP: I really want to stay at your house - Rosa Walton", posXtexto + indent, posYtexto, fontSizeTexto, corTextoComum);




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