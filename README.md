# 🧸 Ecos da infância 🕹️

## Sobre o jogo:
Este jogo nasceu da ideia de misturar personagens classicos da nossa infancia com a mecanica de jogos de turno, como Darkest Dungeon. Consiste na seleção de personagens infântis para montar um time e derrotar o time adversário, reimaginando uma "luta de brinquedos" que brincamos na infância.
---
<details>
<summary> 🚀 COMO INSTALAR  </summary>

## 🚀 Instale a Raylib no Linux (Zorin OS / Ubuntu)
Este guia rápido mostra como instalar a biblioteca **Raylib** em distros Linux baseadas no Ubuntu, como o **Zorin OS**. .

---

## ✅ Passo a passo

### 1. Atualize os pacotes do sistema

Abra o terminal(<kbd>Ctrl + Alt + T</kbd>)e execute:

```
sudo apt update && sudo apt upgrade
```
OBS: caso não tenha instalado o make/gcc/git:
```
 sudo apt install build-essential git
```
### 2. Instalar as dependencias e tudo que a raylib precisa para compilar e rodar corretamente
```
sudo apt install build-essential git cmake libasound2-dev libpulse-dev libx11-dev libxcursor-dev libxinerama-dev libxrandr-dev libxi-dev libgl1-mesa-dev
```
### 3. Baixar o repositorio da raylib no computador
```
git clone https://github.com/raysan5/raylib.git
```
### 4. Agora vamos entrar na pasta que foi criada:
```
cd raylib
```
### 5. Vamos criar uma pasta e depois entrar nela:
```
mkdir build && cd build
```
### 6. Gere os arquivos de compilação com CMake:
```
cmake ..
```
### 7. Compile a Raylib:
```
make
```
### 8. Por fim, instale a biblioteca no sistema:
```
sudo make install
```

---

## 🕹️ Como rodar o jogo?

### 1. Clone o repositório do jogo:
```
git clone https://github.com/Brendalu2005/Jogo-AED.git
```
ou 
```
git clone git@github.com:Brendalu2005/Jogo-AED.git
```
### 2. Entre na pasta do jogo:
```
cd Jogo-AED
```
### 3. compile e depois rode o jogo:
```
make
```
depois
```
./main
```
</details>

---

<details>
<summary> 🧸 SOBRE O JOGO 🕹️</summary>

## 🕹️ Modos de Jogo

### 👤 1 Jogador
- Monte seu time com seus personagens favoritos.
- Lute contra o time adversário (IA).
- A partida acaba quando uma das equipes tiver todos os seus heróis derrotados.

### 👥 2 Jogadores
- Dois jogadores montam seus times.
- Lutam um contra o outro.
- A partida acaba quando uma das equipes tiver todos os seus heróis derrotados.

---

## 📋 Menu Principal

O menu inicial possui três opções:

- **Jogar** ->Escolha entre jogar com 1 ou 2 jogadores.  
- **Sobre** -> Informações sobre o jogo e como jogar.  
- **Personagens** -> Exibe todos os personagens do jogo, suas descrições e ataques.

---


</details>

---

<details>
<summary> 🛠️TECNOLOGIAS UTILIZADAS</summary>

## 🛠️ Tecnologias e Ferramentas Utilizadas 

Este jogo foi desenvolvido utilizando a linguagem **C**, aprendida na disciplina de **AED (Algoritmos e Estrutura de Dados)**. Além da linguagem **C**, utilizamos a biblioteca:

- 🎮 [**Raylib**](https://www.raylib.com/)  
  Optamos pela raylib pois ela é voltada para o desenvolvimento de jogos 2D e 3D, oferecendo suporte a sprites, imagens, sons e outros elementos gráficos e sonoros essenciais.Além disso a comunidade é bastante ativa, facilitanto achar tutoriais e documentação.

</details>

---

<details>
<summary>🎬VIDEO DEMONSTRATIVO</summary>

[**Video demonstrando a jogabilidade**]

https://youtu.be/L2_9O7PGsJo

</details>

---




## 🫂 Equipe de desenvolvimento
| Nome                                  | Email da school    |
| ------------------------------------- | ------------------ | 
| [**Augusto Malheiros de Souza**](https://github.com/goodguto)            | ams10@cesar.school | 
| [**Brenda Luana Correia Bezerra**](https://github.com/Brendalu2005)          | blcb@cesar.school  |
| [**Eduardo Albuquerque Alves Barbosa**](https://github.com/eduaab)     | eaab@cesar.school  |

---
