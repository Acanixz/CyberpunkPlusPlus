// CyberpunkPlusPlus.cpp : define o ponto de entrada para o aplicativo do console.
//

//#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <conio.h>
#include <Windows.h>
#include <locale.h>
#include <math.h>
using namespace std;

// Palheta de cores //
int corChao = 136;
int corObstaculo = 85;
int corJogador = 139;
int corInimigo = 140;

// Structs // 
struct Arma
{
	string nome;
	string descricaoAttk = "atacou";
	string killMsg = "matou";
	int dano_minimo = 5;
	int dano_maximo = 10;
	int stats[4] = { 0 };
	/* O que é cada stat:
	0 - Destreza bonus (decide quem ataca primeiro)
	1 - Chance bonus de critico (maior offset no RNG do atacante)
	2 - Chance bonus de dodge (menor offset no RNG do defensor)
	3 - Quantidade maxima de ataques por turno (exemplo: miniguns)
	*/
};

struct Jogador
{
	int nivel = 0;
	int vida;
	int vidaMaxima = 100;
	Arma* arma;
	int posicao[2] = { -1, -1 };
	COORD posicaoTela;
	int stats[6] = { 0 };
	/* Stats do jogador:
	0 - Sobrecarga, uma contagem de 0 a 100, aumenta a cada uso de implante
	Obs: se chegar a 100, a quant. de inimigos nas proximas fases dobra e
	você perde o controle do personagem no combate até o fim do jogo,
	além de os usos de implantes nesse estado causam dano ao jogador

	1 - Destreza (decide quem ataca primeiro)
	2 - Bonus Sandevistan (quantidade de ações extras no mesmo turno)
	3 - Bonus Implante curativo (cura 20% da vida maxima por turno)
	4 - Bonus Kiroshi Optic (próximo ataque tem +50% de chance de acertar)
	5 - Overshield (valor aleatório concedido ao defender por um turno)
	*/
};

struct Inimigo
{
	string nome = "Inimigo sem nome";
	int vida;
	int vidaMaxima = 100;
	Arma* arma;
	string spriteFile = "Placeholder.txt";
	int spriteColor = 7;
	int stats[4] = { 0 };
	/* Stats do inimigo:
	0 - Destreza (decide quem ataca primeiro)
	1 - Crit base
	2 - Dodge base
	3 - Sandevistan bonus (exclusivo da equipe arasaka e Adam Smasher)
	Não concede turnos extras, ao invés disso, permite que o jogador erre
	ataques mesmo com o sandevistan ativo
	*/
};

struct Bloco
{
	bool bloqueado = false;
	Inimigo* inimigo = NULL;
};

struct Mapa {
	int A; // Altura
	int L; // Largura
	Bloco** blocos;
};

struct Fase
{
	string nome;
	Mapa mapa;
	Inimigo inimigos[5];
	bool ganhou = false;
};

// Funções //

COORD ObterPosicaoCursor(HANDLE hConsoleOutput, bool isjogador = false)
{
	CONSOLE_SCREEN_BUFFER_INFO consoleBuffer;
	if (GetConsoleScreenBufferInfo(hConsoleOutput, &consoleBuffer))
	{
		COORD cursorCoords = consoleBuffer.dwCursorPosition;
		if (isjogador) {
			cursorCoords.X -= 1;
		}
		return cursorCoords;
	}
	else
	{
		return { 0,0 };
	}
}

int RNG(int offset = 0, int max = 1) {
	int num = 0;
	num = rand() % max + offset;
	return num;
}

int Aproximacao(int valor) {
	if (valor % 2 != 0) {
		valor++;
	}
	return valor;
}

Arma* GerarArmas() {
	Arma* armas = new Arma[7];
	armas[0].nome = "Punhos";
	armas[0].descricaoAttk = " deu um soco em ";
	armas[0].killMsg = " saiu na pancadaria e venceu de ";
	armas[0].dano_minimo = 3;
	armas[0].dano_maximo = 7;
	armas[0].stats[0] = 2; // Destreza
	armas[0].stats[1] = 1; // Crit
	armas[0].stats[2] = 3; // Dodge
	armas[0].stats[3] = 0; // Multi-Shot

	armas[1].nome = "Gorilla Arms";
	armas[1].descricaoAttk = " deu um socao em ";
	armas[1].killMsg = " quebrou a mandibula de ";
	armas[1].dano_minimo = 3;
	armas[1].dano_maximo = 7;
	armas[1].stats[0] = 0; // Destreza
	armas[1].stats[1] = 3; // Crit
	armas[1].stats[2] = -2; // Dodge
	armas[1].stats[3] = 0; // Multi-Shot

	armas[2].nome = "Mantis Blade";
	armas[2].descricaoAttk = " deu um corte em ";
	armas[2].killMsg = " desconectou a cabeca de ";
	armas[2].dano_minimo = 6;
	armas[2].dano_maximo = 12;
	armas[2].stats[0] = 2; // Destreza
	armas[2].stats[1] = 4; // Crit
	armas[2].stats[2] = 3; // Dodge
	armas[2].stats[3] = 2; // Multi-Shot

	armas[3].nome = "Rifle de assalto";
	armas[3].descricaoAttk = " atirou em ";
	armas[3].killMsg = " fuzilou ";
	armas[3].dano_minimo = 10;
	armas[3].dano_maximo = 15;
	armas[3].stats[0] = 1; // Destreza
	armas[3].stats[1] = 2; // Crit
	armas[3].stats[2] = 1; // Dodge
	armas[3].stats[3] = 4; // Multi-Shot

	armas[4].nome = "Minigun";
	armas[4].descricaoAttk = " atirou em ";
	armas[4].killMsg = " aniquilou ";
	armas[4].dano_minimo = 5;
	armas[4].dano_maximo = 8;
	armas[4].stats[0] = -1; // Destreza
	armas[4].stats[1] = 2; // Crit
	armas[4].stats[2] = -2; // Dodge
	armas[4].stats[3] = 8; // Multi-Shot

	armas[5].nome = "Pistola";
	armas[5].descricaoAttk = " atirou em ";
	armas[5].killMsg = " estorou o miolos de ";
	armas[5].dano_minimo = 12;
	armas[5].dano_maximo = 14;
	armas[5].stats[0] = 2; // Destreza
	armas[5].stats[1] = 2; // Crit
	armas[5].stats[2] = 2; // Dodge
	armas[5].stats[3] = 0; // Multi-Shot

	armas[6].nome = "Revolver";
	armas[6].descricaoAttk = " atirou em ";
	armas[6].killMsg = " fez um que ota em ";
	armas[6].dano_minimo = 5;
	armas[6].dano_maximo = 20;
	armas[6].stats[0] = 2; // Destreza
	armas[6].stats[1] = 3; // Crit
	armas[6].stats[2] = 2; // Dodge
	armas[6].stats[3] = 0; // Multi-Shot
	return armas;
}

Jogador* GerarJogador(Arma* arma) {
	Jogador* jogador = new Jogador;
	jogador->vida = jogador->vidaMaxima;
	jogador->arma = arma;
	//jogador->stats[1] = 1;
	//jogador->stats[2] = 1;
	//jogador->stats[3] = 1;
	//jogador->stats[4] = 1;
	//jogador->stats[0] = 99;
	return jogador;
}

Inimigo* GerarInimigosPreset(Arma* listaArmas, int levelId = 0) {
	Inimigo* inimigos = new Inimigo[3];
	switch (levelId)
	{
	case 0:
		inimigos[0].nome = "Valentino";
		inimigos[0].vidaMaxima = 30;
		inimigos[0].vida = inimigos[0].vidaMaxima;
		inimigos[0].arma = &listaArmas[RNG(0, 2)]; // Items 0-1
		//inimigos[0].spriteFile = "Valentino_0.txt";

		inimigos[1].nome = "Valentino";
		inimigos[1].vidaMaxima = 25;
		inimigos[1].vida = inimigos[1].vidaMaxima;
		inimigos[1].arma = &listaArmas[3]; // Rifle de assalto
		//inimigos[1].spriteFile = "Valentino_0.txt";

		inimigos[2].nome = "Valentino";
		inimigos[2].vidaMaxima = 20;
		inimigos[2].vida = inimigos[2].vidaMaxima;
		inimigos[2].arma = &listaArmas[RNG(5, 2)]; // Items 5-6
		//inimigos[2].spriteFile = "Valentino_0.txt";
		break;
	}
	return inimigos;
}

Inimigo* EscolherInimigos(Inimigo* inimigosPreset, int numInimigos = 3) {
	Inimigo* inimigos = new Inimigo[numInimigos];

	for (int i = 0; i < numInimigos; i++) {
		int indexEscolhido = RNG(0, 3);
		Inimigo inimigoEscolhido = inimigosPreset[indexEscolhido];
		inimigos[i].nome = inimigoEscolhido.nome;
		inimigos[i].vida = inimigoEscolhido.vida;
		inimigos[i].arma = inimigoEscolhido.arma;
	}

	return inimigos;
}

bool VerificarOpcao(int valor = 0, int min = 0, int max = 0) {
	if (valor >= min && valor <= max) {
		return true;
	}
	return false;
}

void LimparInputBuffer() {
	while (_kbhit()) _getch();
}

void EsperarInput(char teclaEsperada = '0', int waitTimer = 50) {
	while (true) {
		char tecla = _getch();
		if (teclaEsperada == '0' || tecla == teclaEsperada)
			break;
		Sleep(waitTimer);
	}
	LimparInputBuffer();
}

void Display(string msg, int coordX = -1, int coordY = -1, bool Newline = true, int textColor = 7, bool showCursor = false, bool centralize = false) {
	HANDLE STD_OH = GetStdHandle(STD_OUTPUT_HANDLE);
	if (coordX >= 0 || coordY >= 0) {
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(STD_OH, &cursorInfo);
		COORD coord{};

		if (coordX >= 0)
			coord.X = coordX;
		if (centralize)
			coord.X = coord.X - msg.length() / 2;
		if (coordY >= 0)
			coord.Y = coordY;

		SetConsoleCursorPosition(STD_OH, coord);

		cursorInfo.bVisible = showCursor;
		SetConsoleCursorInfo(STD_OH, &cursorInfo);
	}

	SetConsoleTextAttribute(STD_OH, textColor);
	cout << msg;
	if (Newline)
		cout << endl;
}

void DisplayAnimation(string filePath, string fileName, int frames = -1, int textColor = 7, int cornerX = -1, int cornerY = -1, int waitTime = 1000) {
	string linha;
	ifstream frame;
	int i = 0; // Linha atual
	int j = 0; // Frame atual
	switch (frames)
	{
	case -1: // Mostra apenas um frame
		/* code */
		frame.open(filePath + fileName);
		while (getline(frame, linha)) {
			Display(linha, cornerX, cornerY + i, false, textColor);
			i++;
		}

		frame.close();
		break;

	default: // Abre sequencia de frames com esperas de waitTime(milisegundos)
		for (j; j < frames; j++) {
			i = 0;
			frame.open(filePath + fileName + to_string(j) + ".txt");
			while (getline(frame, linha)) {
				Display(linha, cornerX, cornerY + i, false, textColor);
				i++;
			}

			frame.close();
			Sleep(waitTime);
		}
		break;
	}
}

void LimparCores() {
	Display("", 50, 12, false, 7);
}

void LimparTela(int tipo = 0) {
	int Y = 0;
	switch (tipo)
	{
	case 1: // Limpa informações inferiores da tela de combate
		Y = 23;
		for (Y; Y < 30; Y++) {
			Display("                                              ", 0, Y, false, 10);
		}

		Y = 23;
		for (Y; Y < 30; Y++) {
			Display("                             ", 50, Y, false, 10);
		}
		break;

	case 2: // Limpa log de combate
		Y = 1;
		for (Y; Y < 29; Y++) {
			Display("                                      ", 100, Y, false, 10, false, true);
		}
		break;

	case 3: // Limpa menu de escolhas de combate
		Y = 23;
		for (Y; Y < 30; Y++) {
			Display("                                              ", 0, Y, false, 10);
		}
		break;

	default:
		LimparCores();  // Se der cls antes disso, a tela fica com a ultima cor usada
		system("cls");
		break;
	}
}

void Carregar_Menu() {
	Display("", 50, 12, false, 7, true);
	Sleep(1000);
	DisplayAnimation("Frames/MenuAnimated/", "MenuFrame_", 9, 10, 10, 2, 50);
	Display("Cyberpunk++", 50, 1, false, 10, false, true);
	Sleep(500);
	Display("Desenvolvido por:", 85, 3, false, 14, false, true);
	Display("Eduardo da Rocha Weber", 85, 5, false, 14, false, true);
	Display("Herick Vitor Vieira Bittencourt", 85, 6, false, 14, false, true);
	Display("Eduardo Miguel Fuchs Perez", 85, 7, false, 14, false, true);
	Display("Aperte qualquer tecla para comecar", 50, 25, false, 160, false, true);
	LimparCores();
}

void Carregar_Tutorial() {
	DisplayAnimation("Frames/Tutorial/", "Tutorial.txt", -1, 7, 50, 1);

	Display("Tutorial Basico:", 30, 2, false, 7, false, true);

	Display("#", 20, 4, false, corJogador);
	Display("<--- Jogador", 30, 4, false, 7, false, true);
	Display("#", 20, 7, false, corInimigo);
	Display("<--- Inimigo", 30, 7, false, 7, false, true);

	Display(" ", 20, 11, false, corChao);
	Display("<--- Chao", 30, 11, false, 7, false, true);
	Display(" ", 20, 14, false, corObstaculo);
	Display("<--- Parede", 30, 14, false, 7, false, true);

	Display("Use as teclas WASD para se movimentar pelo mapa", 50, 23, false, 7, false, true);
	Display("Objetivo: mate todos os inimigos e derrote Adam Smasher", 50, 25, false, 7, false, true);
	Sleep(1000);
	Display("Aperte qualquer tecla para comecar", 50, 27, false, 160, false, true);
}

bool VerificarCoord(Fase* fase, int tipo, int coords[2] = { 0 }) {
	switch (tipo)
	{
	case 0: // Verificação por existência do bloco na grade (Y e X)
		if ((coords[0] >= 0 && coords[0] < fase->mapa.A) && (coords[1] >= 0 && coords[1] < fase->mapa.L)) {
			return true;
		}
		break;

	case 1: // Verificação por espaço bloqueado
		return fase->mapa.blocos[coords[0]][coords[1]].bloqueado;
		break;

	case 2: // Verificação por inimigo
		if (fase->mapa.blocos[coords[0]][coords[1]].inimigo != NULL) {
			return true;
		}
		else {
			return false;
		}
		break;

	default:
		break;
	}
	return false;
}

Mapa CriarMapa(int A, int L) {
	Mapa mapa;

	mapa.A = A;
	mapa.L = L;
	mapa.blocos = new Bloco * [A]; // Geração de linhas

	for (int i = 0; i < A; i++) { // Geração de colunas
		mapa.blocos[i] = new Bloco[L];
	}

	return mapa;
}

Fase* CriarFase(int numInimigos, Inimigo* inimigos, string nome, int alturaMapa, int larguraMapa) {
	Fase* fase = new Fase;
	fase->nome = nome;
	Display("Gerando mapa", 50, 11, false, 10, false, true);
	fase->mapa = CriarMapa(alturaMapa, larguraMapa);
	Display("Gerando obstaculos", 50, 12, false, 10, false, true);
	int quantObstaculos = (alturaMapa / 4 + larguraMapa / 4);
	Aproximacao(quantObstaculos);

	for (int i = 0; i < quantObstaculos;) { // For loop sem incremento automatico
		int localEscolhido[2] = { RNG(0, alturaMapa), RNG(0, larguraMapa) };
		if (VerificarCoord(fase, 0, localEscolhido)) { // Espaço existe?
			if (VerificarCoord(fase, 1, localEscolhido) == false) { // Espaço está livre?
				fase->mapa.blocos[localEscolhido[0]][localEscolhido[1]].bloqueado = true;
				i++;
			}
		}
	}

	Display("Gerando inimigos", 50, 13, false, 10, false, true);
	for (int i = 0; i < numInimigos;) { // For loop sem incremento automatico
		int coordEscolhida[2] = { RNG(0, fase->mapa.A), RNG(0, fase->mapa.L) };

		if (VerificarCoord(fase, 0, coordEscolhida)) { // Espaço existe?
			if (VerificarCoord(fase, 1, coordEscolhida) == false && VerificarCoord(fase, 2, coordEscolhida) == false) {
				// Bloco livre, sem inimigos também
				fase->mapa.blocos[coordEscolhida[0]][coordEscolhida[1]].inimigo = &inimigos[i];
				i++;
			}
		}
	}
	return fase;
}

void DisplayFase(Fase* fase, Jogador* jogador) {
	LimparTela();
	int A = fase->mapa.A;
	int L = fase->mapa.L;
	string borda = "@";
	int bordaX = 50 - L / 2;
	Aproximacao(bordaX);

	for (int i = 0; i < L; i++) {
		borda = borda + "-";
	}
	borda = borda + "@";
	Display(borda, bordaX - 1, 10, false, 7);

	for (int i = 0; i < A; i++) {
		for (int j = 0; j < L; j++) {

			Display(" ", bordaX + j, 11 + i, false, corChao);


			if (fase->mapa.blocos[i][j].bloqueado == true) {
				Display(" ", bordaX + j, 11 + i, false, corObstaculo);
			}

			if (fase->mapa.blocos[i][j].inimigo != NULL) {
				Display("#", bordaX + j, 11 + i, false, corInimigo);
			}

			if (jogador->posicao[0] == i && jogador->posicao[1] == j) {
				Display("#", bordaX + j, 11 + i, false, corJogador);
				jogador->posicaoTela = ObterPosicaoCursor(GetStdHandle(STD_OUTPUT_HANDLE), true);
			}

			Display("|", bordaX - 1, 11 + i, false);
			Display("|", bordaX + j + 1, 11 + i, false);
		}
	}
	Display(borda, bordaX - 1, 11 + A, false);
	Display(fase->nome, 50, 1, false, 10, false, true);
	Display(to_string(jogador->vida), 50, 2, false, 10, false, true);
}

void Movimentar(Jogador* jogador, Fase* fase) {
	int moveDelta[2] = { 0 };
	bool debounce[4] = { false };
	for (int i = 0; i < 4; i++) {
		if (_kbhit()) {
			char tecla = _getch();
			if ((tecla == 'W' || tecla == 'w') && debounce[0] == false) {
				debounce[0] = true;
				moveDelta[0] -= 1;
			}

			if ((tecla == 'S' || tecla == 's') && debounce[1] == false) {
				debounce[1] = true;
				moveDelta[0] += 1;
			}

			if ((tecla == 'A' || tecla == 'a') && debounce[2] == false) {
				debounce[2] = true;
				moveDelta[1] -= 1;
			}

			if ((tecla == 'D' || tecla == 'd') && debounce[3] == false) {
				debounce[3] = true;
				moveDelta[1] += 1;
			}
		}
	}

	if (moveDelta[0] != 0 || moveDelta[1] != 0) {
		int novaPosicao[2] = { 0 };
		novaPosicao[0] = jogador->posicao[0] + moveDelta[0]; // Y
		novaPosicao[1] = jogador->posicao[1] + moveDelta[1]; // X

		COORD novaPosicaoTela = { jogador->posicaoTela.X, jogador->posicaoTela.Y };
		novaPosicaoTela.X += moveDelta[1];
		novaPosicaoTela.Y += moveDelta[0];

		if (VerificarCoord(fase, 0, novaPosicao)) { // Espaço existe?
			if (VerificarCoord(fase, 1, novaPosicao) == false) { // Espaço está livre?
				Display(" ", jogador->posicaoTela.X, jogador->posicaoTela.Y, false, corChao);
				jogador->posicao[0] = novaPosicao[0];
				jogador->posicao[1] = novaPosicao[1];
				Display("#", novaPosicaoTela.X, novaPosicaoTela.Y, false, corJogador);
				jogador->posicaoTela = novaPosicaoTela;
			}
		}
	}
	Sleep(50);
}

template <typename T>
bool morreu(T personagem);

void ataque(Jogador* atacante, Inimigo* defensor, int &Ylog);
void ataque(Inimigo* atacante, Jogador* defensor, int &Ylog, bool bloqueando = false);

void jogarFase(Jogador* jogador, Fase* fase);

void iniciarCombate(Jogador* jogador, Inimigo* inimigo) {
	DisplayAnimation("Frames/Enemies/", inimigo->spriteFile, -1, inimigo->spriteColor, 0, 0);
	// LOG DE COMBATE
	Display("LOG DE COMBATE:", 100, 0, false, 10, false, true);
	Display("Voce encontrou " + inimigo->nome, 100, 1, false, 10, false, true);
	int Ylog = 2;

	while (morreu(jogador) == false && morreu(inimigo) == false) {
		LimparTela(1);

		if (jogador->stats[2] > 0) { // Sandevistan countdown
			jogador->stats[2] -= 1;
		}
		else {
			if (jogador->stats[0] < 100) { // Decaimento de sobrecarga
				jogador->stats[0] -= 10;
				if (jogador->stats[0] < 0) {
					jogador->stats[0] = 0;
				}
			}
		}

		if (jogador->stats[3] > 0) { // Imp. Reg. countdown
			jogador->stats[3] -= 1;
			jogador->vida += jogador->vidaMaxima * .2;
			if (jogador->vida > jogador->vidaMaxima) {
				jogador->vida = jogador->vidaMaxima;
			}
		}

		if (jogador->stats[4] > 0) { // Kiroshi countdown
			jogador->stats[4] -= 1;
		}

		// DISPLAY DE INFORMAÇÕES DO PERSONAGEM
		Display("Relatorio de integridade:", 50, 23, false, 10);
		Display("Vida: " + to_string(jogador->vida) + "    ", 50, 24, false, 10);

		if (jogador->stats[0] >= 50) { // Sobrecarga
			if (jogador->stats[0] >= 75) {
				if (jogador->stats[0] >= 100) {
					Display("Sobrecarga: ERRO (CIBERPSICOSE DETECTADA)", 50, 25, false, 4);
				}
				else {
					Display("Sobrecarga: " + to_string(jogador->stats[0]) + "% (PERIGO)", 50, 25, false, 4);
				}

			}
			else {
				Display("Sobrecarga: " + to_string(jogador->stats[0]) + "%", 50, 25, false, 14);
			}
		}
		else {
			Display("Sobrecarga: " + to_string(jogador->stats[0]) + "%", 50, 25, false, 10);
		}

		if (jogador->stats[2] > 0) { // Sandevistan status
			Display("Sandevistan ativo! (" + to_string(jogador->stats[2]) + ")", 50, 26, false, 10);
		}

		if (jogador->stats[3] > 0) { // Imp. Curativo status
			Display("Regeneracao em progresso (" + to_string(jogador->stats[3]) + ")", 50, 27, false, 10);
		}

		if (jogador->stats[4] > 0) { // Kiroshi Optic status
			Display("Assistencia de mira em uso!", 50, 28, false, 10);
		}

		int escolha = 0;
		int escolhaAux = 0; // Variavel temporaria para decisões
		if (jogador->stats[0] < 100) { // Livre escolha caso n tenha ciberpsicose
			while (escolha <= 0) {
				LimparTela(3);
				if (escolha == -1) {
					Display("Opcao invalida!", 1, 29, false, 4);
				}

				Display("O que fazer? [[Vida do inimigo: " + to_string(inimigo->vida) + "]]", 1, 23, false, 10);
				Display("1 - Atacar", 1, 24, false, 10);
				Display("2 - Bloquear", 1, 25, false, 10);
				Display("3 - Usar Implante", 1, 26, false, 10);
				Display("                                               ", 1, 28, false);
				Display("Opcao Escolhida: ", 1, 28, false, 10, true);
				LimparInputBuffer();
				cin >> escolhaAux;
				if (VerificarOpcao(escolhaAux, 1, 3)) {
					if (VerificarOpcao(escolhaAux, 1, 2)) { // Escolhas primarias
						escolha = escolhaAux;

					}
					else { // Abrir menu de implantes
						LimparTela(3);
						Display("Escolha um implante:", 1, 23, false, 10);
						Display("1 - Sandevistan (varias acoes no mesmo turno)", 1, 24, false, 10);
						Display("2 - Imp. Regenerativo (20% cura por 2 turnos)", 1, 25, false, 10);
						Display("3 - Kiroshi Opctics (Precisao aumentada)", 1, 26, false, 10);
						Display("4 - Voltar", 1, 27, false, 10);
						Display("                                               ", 1, 28, false);
						Display("Opcao Escolhida: ", 1, 28, false, 10, true);
						LimparInputBuffer();
						cin >> escolhaAux;
						if (VerificarOpcao(escolhaAux, 1, 3)) {
							escolha = 10 + escolhaAux;
						}
					}
				}
				else {
					escolha = -1;
				}
			}

			if (Ylog > 20) {
				Ylog = 1;
				LimparTela(2);
			}

			switch (escolha)
			{
			case 1: // Ataque
				if (jogador->stats[1] + jogador->arma->stats[0] > inimigo->stats[0]) {
					
					ataque(jogador, inimigo, Ylog);
					if (jogador->stats[2] <= 0) { // Sandevistan check
						ataque(inimigo, jogador, Ylog);
					}
					
				}
				else {
					if (jogador->stats[2] <= 0) { // Sandevistan check
						ataque(inimigo, jogador, Ylog);
					}
					ataque(jogador, inimigo, Ylog);
				}
				break;

			case 2: // Bloquear
				jogador->stats[5] = RNG(5, 20); // Conceder overshield aleatório
				Display("Voce ativou OverS. (" + to_string(jogador->stats[5]) + "dmg DEF)", 100, Ylog, false, 10, false, true);
				Ylog++;
				ataque(inimigo, jogador, Ylog, true);
				break;

			case 11:
				Display("Voce usou o Sandevistan!", 100, Ylog, false, 10, false, true);
				Ylog++;
				jogador->stats[2] = 3; // Sandevistan = +2 ações + ação atual
				jogador->stats[0] += 35; // + Sobrecarga
				break;

			case 12:
				Display("Voce usou o I. Regenerativo!", 100, Ylog, false, 10, false, true);
				Ylog++;
				jogador->stats[3] = 2; // Implante renegerativo = 2 turnos de cura
				jogador->stats[0] += 20; // + Sobrecarga - 10

				if (jogador->stats[2] <= 0) { // Sandevistan check
					ataque(inimigo, jogador, Ylog);
				}
				break;

			case 13:
				Display("Voce usou o Kiroshi Opctics!", 100, Ylog, false, 10, false, true);
				Ylog++;
				jogador->stats[4] = 2; // Kiroshi Optics = Prox. Turno c/ precisão aumentada
				jogador->stats[0] += 25; // + Sobrecarga - 10

				if (jogador->stats[2] <= 0) { // Sandevistan check
					ataque(inimigo, jogador, Ylog);
				}
				break;

			default:
				break;
			}
		}
		else {

		}
	}

	LimparTela(1);
	if (morreu(inimigo)) {
		Display("Voce" + jogador->arma->killMsg + inimigo->nome, 1, 23, false, 10);
	}
	else {
		Display(inimigo->nome + inimigo->arma->killMsg + "voce", 1, 23, false, 10);
	}
	Display("Aperte qualquer tecla p/ continuar", 1, 24, false, 160);
	LimparCores();
	LimparInputBuffer();
	EsperarInput();
}

int main()
{
	setlocale(LC_ALL, "Portuguese");
	srand(time(NULL));

	// DADOS DO JOGO, GERADO APENAS UMA VEZ
	Arma* armas = GerarArmas();
	string nomeFases[3] = { "Bairro 1", "Bairro 2", "Torre Arasaka" };
	int alturaFases[3] = { 10,5,15 };
	int larguraFases[3] = { 20,30,40 };
	int quantidadeInimigos[3] = { RNG(1, 5), RNG(1, 5), RNG(1, 5) };
	Inimigo** inimigos_Matriz = new Inimigo * [3];

	for (int i = 0; i < 3; i++) {
		inimigos_Matriz[i] = new Inimigo[3];
	}
	////////////////////

	Carregar_Menu();
	LimparInputBuffer();
	EsperarInput();
	LimparTela();

	Carregar_Tutorial();
	LimparInputBuffer();
	EsperarInput();

	for (int levelId = 0; levelId < 3; levelId++) {
		LimparTela();
		Display("Criando " + nomeFases[levelId], 50, 10, false, 10, false, true);
		int numInimigos = RNG(3, 3); // Entre 3 a 5 inimigos por fase
		Inimigo* inimigosPreset = GerarInimigosPreset(armas, levelId);
		Inimigo* inimigosEscolhidos = EscolherInimigos(inimigosPreset, numInimigos);
		Fase* fase = CriarFase(numInimigos, inimigosEscolhidos, nomeFases[levelId], alturaFases[levelId], larguraFases[levelId]);

		Display("Gerando jogador", 50, 14, false, 10, false, true);
		Jogador* jogador = GerarJogador(&armas[0]); // Jogador no soco
		//Jogador* jogador = GerarJogador(&armas[4]); // Jogador de minigun

		while (jogador->posicao[0] == -1) {
			int localEscolhido[2] = { RNG(0, fase->mapa.A), RNG(0, fase->mapa.L) };
			if (VerificarCoord(fase, 0, localEscolhido)) { // Espaço existe?
				if (VerificarCoord(fase, 1, localEscolhido) == false && VerificarCoord(fase, 2, localEscolhido) == false) {
					// Espaço livre, sem inimigos
					jogador->posicao[0] = localEscolhido[0]; // Posicionar Y
					jogador->posicao[1] = localEscolhido[1]; // Posicionar X
				}
			}
		}
		LimparTela();
		DisplayFase(fase, jogador);
		LimparInputBuffer();
		jogarFase(jogador, fase);
	}
}

template <typename T>
bool morreu(T personagem)
{
	if (personagem->vida <= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ataque(Jogador* atacante, Inimigo* defensor, int &Ylog)
{
	int totalTiros = atacante->arma->stats[3];
	if (atacante->arma->stats[3] <= 0) {
		totalTiros = 1;
	}
	for (int i = 0; i < totalTiros; i++) {
		int dano = RNG(atacante->arma->dano_minimo, atacante->arma->dano_maximo - atacante->arma->dano_minimo);

		int acertou = RNG(1 - defensor->stats[2], 20); // 1 - Dodge base inimigo
		if (atacante->stats[4]) {
			acertou = RNG(5 - defensor->stats[2], 20); // Kiroshi Optics ativo
		}

		if (atacante->stats[2] > 0 && defensor->stats[3] <= 0) { // Sandevistan check
			acertou = 10; // Se sandevistan ativo e inimigo não tem sandevistan, acertou!
		} 
		if (acertou >= 10) { // Alvo acertado
			int crit = RNG(1 + atacante->arma->stats[1], 20); // 1 + Crit bonus arma

			if (atacante->stats[4]) {
				crit = RNG(3 + atacante->arma->stats[1], 20); // Kiroshi Crit bonus + Crit arma
			}

			if (crit >= 10) {
				dano *= 1.5;
			}
			defensor->vida -= dano;
			if (defensor->vida < 0) {
				defensor->vida = 0;
			}

			if (crit < 10) {
				Display("Voce" + atacante->arma->descricaoAttk + defensor->nome + " (-" + to_string(dano) + "HP)", 100, Ylog, false, 10, false, true);
			}
			else {
				Display("Voce" + atacante->arma->descricaoAttk + defensor->nome + " (-" + to_string(dano) + "HP!)", 100, Ylog, false, 10, false, true);
			}
		}
		else {
			Display("Voce errou!", 100, Ylog, false, 10, false, true);
		}
		Ylog++;
		Sleep(100);
	}
}

void ataque(Inimigo* atacante, Jogador* defensor, int& Ylog, bool bloqueando)
{
	int totalTiros = atacante->arma->stats[3];
	if (atacante->arma->stats[3] <= 0) {
		totalTiros = 1;
	}
	bool overshieldDestruido = false;
	for (int i = 0; i < totalTiros; i++) {
		int dano = RNG(atacante->arma->dano_minimo, atacante->arma->dano_maximo - atacante->arma->dano_minimo);
		int danoOriginal = dano;
		dano -= defensor->stats[5];
		defensor->stats[5] -= danoOriginal;
		

		int acertou = RNG(1 - defensor->arma->stats[2], 20); // 1 - Dodge bonus jogador
		if (dano <= 0) {
			acertou = 0; // Erra automaticamente se o overshield anulou o ataque
		}

		if (defensor->stats[5] > 0) {
			Display("Bloqueado " + to_string(danoOriginal) + "dmg", 100, Ylog, false, 10, false, true);
			Ylog++;
		}
		else {
			if (overshieldDestruido == false && bloqueando) {
				overshieldDestruido = true;
				Display("Overshield quebrou!", 100, Ylog, false, 10, false, true);
				Ylog++;
			}
		}

		if (acertou >= 10) { // Alvo acertado
			int crit = RNG(1 + atacante->stats[1], 20); // 1 + Crit base inimigo

			if (crit >= 10) {
				dano *= 1.5;
			}
			defensor->vida -= dano;
			if (defensor->vida < 0) {
				defensor->vida = 0;
			}

			if (crit < 10) {
				Display(atacante->nome + atacante->arma->descricaoAttk + +"voce (-" + to_string(dano) + "HP)", 100, Ylog, false, 10, false, true);
			}
			else {
				Display(atacante->nome + atacante->arma->descricaoAttk + "voce (-" + to_string(dano) + "HP!)", 100, Ylog, false, 10, false, true);
			}
		}
		else {
			Display(atacante->nome + " errou!", 100, Ylog, false, 10, false, true);
		}
		Ylog++;
		Sleep(100);
	}
	defensor->stats[5] = 0; // Remove overshield do jogador
}

void jogarFase(Jogador* jogador, Fase* fase)
{
	while (morreu(jogador) == false && fase->ganhou == false)
	{
		Movimentar(jogador, fase);
		if (fase->mapa.blocos[jogador->posicao[0]][jogador->posicao[1]].inimigo != NULL) {
			// Inimigo encontrado, iniciar combate
			LimparTela();
			DisplayAnimation("Frames/CombatInitiation/", "Combat_", 12, 7, 10, 2, 10);
			LimparTela();
			iniciarCombate(jogador, fase->mapa.blocos[jogador->posicao[0]][jogador->posicao[1]].inimigo);
		}
	}
}
