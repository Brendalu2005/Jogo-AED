#include "raylib.h"
#include "tela.h" // Inclui as constantes (screenWidth, screenHeight)

// Esta é a implementação da função de desenho
void DesenharTelaBatalha(void){

    // --- 1. Área Superior (Info Jogador e Round) ---
    
    // Jogador 1 (Esquerda)
    DrawText("Jogador 1", 20, 15, 20, RAYWHITE);
    DrawRectangle(130, 10, 200, 30, GREEN); // Barra de vida J1
    DrawRectangleLines(130, 10, 200, 30, BLACK); 

    // Contagem de Rounds (Centro)
    DrawText("Round", (screenWidth / 2) - 40, 15, 20, RAYWHITE);
    DrawText("3", (screenWidth / 2) - 10, 40, 30, RAYWHITE);

    // IA (Direita) - MUDANÇA REQUISITADA
    int iaBarX = screenWidth - 20 - 200; // Posição X da barra da IA
    DrawText("IA", iaBarX - 35, 15, 20, RAYWHITE); // Texto "IA"
    DrawRectangle(iaBarX, 10, 200, 30, GREEN); // Barra de vida IA
    DrawRectangleLines(iaBarX, 10, 200, 30, BLACK);

    
    // --- 2. Área Central (Campo de Batalha) ---
    int arenaY = 80;
    int arenaHeight = 450;
    DrawRectangleLines(10, arenaY, screenWidth - 20, arenaHeight, LIGHTGRAY);


    // --- 3. Área Inferior (Menu de Ataque) ---
    int menuY = arenaY + arenaHeight + 10;
    int menuHeight = screenHeight - menuY - 10;
    Color menuBG = (Color){ 40, 40, 40, 255 }; // Fundo cinza escuro
    DrawRectangle(10, menuY, screenWidth - 20, menuHeight, menuBG);
    DrawRectangleLines(10, menuY, screenWidth - 20, menuHeight, RAYWHITE);


    // --- 4. Conteúdo do Menu de Ataque (BASEADO NA NOVA IMAGEM) ---

    // Posições
    int colAtaquesX = 35;
    int colSpecsX = 300; // Posição X da coluna de especificações
    int textoYBase = menuY + 20;

    // Título "Ataque:"
    DrawText("Ataque:", colAtaquesX, textoYBase, 20, GREEN);

    // --- Botões de Ataque ---
    Color corBotaoNormal = LIGHTGRAY;
    Color corBotaoSelecionado = YELLOW;
    Color corTexto = BLACK;
    float espessuraBorda = 2.0f; // <-- Defini a espessura aqui

    // Botão 1: Soco (Simulando seleção)
    Rectangle btnSoco = { colAtaquesX, textoYBase + 35, 150, 40 };
    DrawRectangleRounded(btnSoco, 0.2f, 4, corBotaoSelecionado);
    // CORREÇÃO AQUI (adicionado espessuraBorda)
    DrawRectangleRoundedLines(btnSoco, 0.2f, 4, espessuraBorda, BLACK); 
    // Centraliza o texto dentro do botão
    DrawText("Soco", btnSoco.x + (btnSoco.width / 2) - (MeasureText("Soco", 20) / 2), btnSoco.y + 10, 20, corTexto);

    // Botão 2: Cuspe (Normal)
    Rectangle btnCuspe = { colAtaquesX, textoYBase + 90, 150, 40 };
    DrawRectangleRounded(btnCuspe, 0.2f, 4, corBotaoNormal);
    // CORREÇÃO AQUI (adicionado espessuraBorda)
    DrawRectangleRoundedLines(btnCuspe, 0.2f, 4, espessuraBorda, BLACK); 
    DrawText("Cuspe", btnCuspe.x + (btnCuspe.width / 2) - (MeasureText("Cuspe", 20) / 2), btnCuspe.y + 10, 20, corTexto);


    // Título "especificações:"
    DrawText("especificações:", colSpecsX, textoYBase, 20, GREEN);

    // Descrição (Simulando a descrição do "Soco", que está selecionado)
    DrawText("Um soco direto no oponente.", colSpecsX, textoYBase + 40, 20, RAYWHITE);
    DrawText("Causa 10 de Dano.", colSpecsX, textoYBase + 70, 20, RAYWHITE);
    DrawText("Custo: 5 MP", colSpecsX, textoYBase + 100, 20, RAYWHITE);
}