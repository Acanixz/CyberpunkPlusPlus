#pragma region includes

//#include "stdafx.h" // Usado no pc da univali
#include <iostream>
#include <fstream> // Animações
#include <string> // Texto
#include <time.h> // RNG
#include <conio.h> // Coordenadas console
#include <Windows.h> // Cor console

#pragma endregion

using namespace std;

#pragma region Variaveis

// Palheta de cores //
int corChao = 136;
int corObstaculo = 85;
int corJogador = 139;
int corInimigo = 140;

// Custos de Implantes //
int custoSandevistan = 50;
int custoImpReg = 25;
int custoKiroshiOptics = 25;

#pragma endregion

#pragma region Structs

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
	bool temInimigo = false;
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
	Inimigo inimigos[10]; // Maximo 10 inimigos
	int inimigosRestantes = 0;
	bool ganhou = false;
	bool reset = false; // Recria a fase se R for apertado
};

#pragma endregion

#pragma region Templates

template <typename T>
bool morreu(T personagem);

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

#pragma endregion

#pragma region Ações Jogador

void ataque(Jogador* atacante, Inimigo* defensor, int& Ylog);
void ataque(Inimigo* atacante, Jogador* defensor, int& Ylog, bool bloqueando = false);
void jogarFase(Jogador* jogador, Fase* fase);

#pragma endregion

#pragma region Declaração das Funções

COORD ObterPosicaoCursor(HANDLE hConsoleOutput, bool isjogador = false);
int RNG(int offset = 0, int max = 1);
int Aproximacao(int valor);
Arma* GerarArmas();
Jogador* GerarJogador(Arma* arma);
Inimigo* GerarInimigosPreset(Arma* listaArmas, int levelId = 0);
Inimigo* EscolherInimigos(Inimigo* inimigosPreset, int numInimigos = 3);
bool VerificarOpcao(int valor = 0, int min = 0, int max = 0);
void LimparInputBuffer();
void EsperarInput(char teclaEsperada = '0', int waitTimer = 50);
void Display(string msg, int coordX = -1, int coordY = -1, int textColor = 7, bool showCursor = false, bool centralize = false);
void DisplayAnimation(string filePath, string fileName, int frames = -1, int textColor = 7, int cornerX = -1, int cornerY = -1, int waitTime = 1000);
void LimparCores();
void LimparTela(int tipo = 0);
void Carregar_Menu();
void Carregar_Tutorial();
bool VerificarCoord(Fase* fase, int tipo, int coords[2] = { 0 });
Mapa CriarMapa(int A, int L);
Fase* CriarFase(int numInimigos, Inimigo* inimigos, string nome, int alturaMapa, int larguraMapa);
void DisplayFase(Fase* fase, Jogador* jogador);
void DisplayStatusJogador(Jogador* jogador);
void Movimentar(Jogador* jogador, Fase* fase);
void escolhaSaque(Jogador* jogador, Inimigo* inimigo);
void iniciarCombate(Jogador* jogador, Inimigo* inimigo);
void ataque(Jogador* atacante, Inimigo* defensor, int& Ylog);
void ataque(Inimigo* atacante, Jogador* defensor, int& Ylog, bool bloqueando);
void jogarFase(Jogador* jogador, Fase* fase);

#pragma endregion

int main()
{
	srand(time(NULL));

	// DADOS DO JOGO, GERADO APENAS UMA VEZ
	Arma* armas = GerarArmas();
	string nomeFases[3] = { "Heywood", "South Pacifica", "Torre Arasaka" };
	int alturaFases[3] = { 10,5,15 };
	int larguraFases[3] = { 20,30,40 };
	int quantidadeInimigos[3] = { RNG(1, 5), RNG(1, 5), RNG(1, 5) };
	bool gameLoop = true;
	////////////////////

	while (gameLoop == true) { // Loop para voltar ao menu ao fim de jogo
		gameLoop = false; // Decidido no final
		LimparTela();
		Carregar_Menu();
		LimparInputBuffer();
		EsperarInput();
		LimparTela();

		Carregar_Tutorial();
		LimparInputBuffer();
		EsperarInput();

		Jogador* jogador = GerarJogador(&armas[0]);

		int ultimoLevelId = -1;
		for (int levelId = 0; levelId < 3; levelId++) {
			LimparTela();
			Display("Criando " + nomeFases[levelId], 50, 10, 10, false, true);
			Sleep(500);
			int numInimigos = 3; // Inicialização
			if (ultimoLevelId != levelId) {
				/* Aleatoriza quantidade inimigos se, e apenas se
				* é uma fase nova, se for um reset, a quantidade se manterá igual
				*/
				ultimoLevelId = levelId;
				numInimigos = RNG(3, 3); // Entre 3 a 5 inimigos por fase
				if (jogador->stats[0] >= 100) { // Dobro em caso de ciberpsicopatia
					numInimigos *= 2;
				}
			}
			Inimigo* inimigosPreset = GerarInimigosPreset(armas, levelId);
			Inimigo* inimigosEscolhidos = EscolherInimigos(inimigosPreset, numInimigos);
			Fase* fase = CriarFase(numInimigos, inimigosEscolhidos, nomeFases[levelId], alturaFases[levelId], larguraFases[levelId]);

			Display("Gerando jogador", 50, 14, 10, false, true);

			while (jogador->posicao[0] == -1) { // Posicionamento aleatório do jogador
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
			// Fim da fase, desalocação de memória e conclusões abaixo
			delete[] inimigosEscolhidos;
			delete[] inimigosPreset;
			for (int i = 0; i < fase->mapa.A; i++) {
				delete[] fase->mapa.blocos[i];
			}
			if (fase->ganhou == false && fase->reset == false) {
				levelId = 999; // Encerramento do for loop, game over!
			}
			if (fase->reset) {
				levelId--; // Recriando fase, remover incremento
			}

			delete fase;
			jogador->posicao[0] = -1; // Reposicionamento obrigatório
		}

		LimparTela();

		if (jogador->vida > 0) { // Luta final
			Inimigo* adam_Smasher = new Inimigo;
			adam_Smasher->nome = "Adam Smasher";
			adam_Smasher->vidaMaxima = 100;
			adam_Smasher->vida = adam_Smasher->vidaMaxima;
			adam_Smasher->arma = &armas[4]; // Metralhadora Pesada
			adam_Smasher->spriteFile = "AdamSmasher.txt";
			adam_Smasher->spriteColor = 12; // Vermelho
			adam_Smasher->stats[0] = -3; // Destreza
			adam_Smasher->stats[1] = 2; // Crit base
			adam_Smasher->stats[2] = 2; // Dodge base
			adam_Smasher->stats[3] = 1; // Sandevistan Bonus

			iniciarCombate(jogador, adam_Smasher);
			delete adam_Smasher;
		}

		LimparTela();

		if (jogador->vida > 0) { // Venceu o game
			Display("Parabens!", 50, 1, 10, false, true);
			Display("Voce derrotou Adam Smasher e provou-se digno de Night City", 50, 2, 10, false, true);
		}
		else { // Game over
			Display("Game over!", 50, 1, 10, false, true);
		}

		Display("Deseja voltar ao menu ou encerrar o jogo?", 50, 4, 10, false, true);
		Display("1 - Voltar ao menu", 50, 5, 10, false, true);
		Display("2 - Fechar jogo", 50, 6, 10, false, true);
		int escolhaGameLoop = 0;
		while (VerificarOpcao(escolhaGameLoop, 1, 2) == false) {
			if (escolhaGameLoop == -1) {
				Display("Opcao invalida!", 50, 9, 4, false, true);
			}
			Display("                   ", 50, 8, 7, true, true);
			Display("", 50, 8, 10, true, true);
			cin >> escolhaGameLoop;
			if (VerificarOpcao(escolhaGameLoop, 1, 2) == false) {
				escolhaGameLoop = -1;
			}
		}

		if (escolhaGameLoop == 1) { // Voltar ao menu e continuar execução
			gameLoop = true;
		}
		else {
			LimparTela();
		}
		delete jogador;
	}
}

#pragma region Funções

COORD ObterPosicaoCursor(HANDLE hConsoleOutput, bool isjogador)
{
	/*
	* Obtém a posição do cursor do console, usado principalmente
	* para o posicionamento do player
	*/
	CONSOLE_SCREEN_BUFFER_INFO consoleBuffer;
	if (GetConsoleScreenBufferInfo(hConsoleOutput, &consoleBuffer))
	{
		COORD cursorCoords = consoleBuffer.dwCursorPosition;
		if (isjogador) { // 
						 /*
						 * Pois o jogador é desenhado primeiro, logo o cursor é
						 * 1 bloco para frente do esperado
						 */
			cursorCoords.X -= 1;
		}
		return cursorCoords;
	}
	else
	{
		return { 0,0 };
	}
}

int RNG(int offset, int max) { // Numeros aleatórios
	int num = 0;
	num = rand() % max + offset;
	return num;
}

int Aproximacao(int valor) { // Aproximação para par
	if (valor % 2 != 0) {
		valor++;
	}
	return valor;
}

Arma* GerarArmas() { // Cria endereço de armas
	Arma* armas = new Arma[7];
	armas[0].nome = "Punhos";
	armas[0].descricaoAttk = " socou ";
	armas[0].killMsg = " saiu na pancadaria e venceu de ";
	armas[0].dano_minimo = 1;
	armas[0].dano_maximo = 5;
	armas[0].stats[0] = 0; // Destreza
	armas[0].stats[1] = 1; // Crit
	armas[0].stats[2] = 2; // Dodge
	armas[0].stats[3] = 1; // Multi-Shot

	armas[1].nome = "Gorilla Arms";
	armas[1].descricaoAttk = " espancou ";
	armas[1].killMsg = " quebrou a mandibula de ";
	armas[1].dano_minimo = 3;
	armas[1].dano_maximo = 6;
	armas[1].stats[0] = 0; // Destreza
	armas[1].stats[1] = 2; // Crit
	armas[1].stats[2] = 1; // Dodge
	armas[1].stats[3] = 2; // Multi-Shot

	armas[2].nome = "Mantis Blade";
	armas[2].descricaoAttk = " cortou ";
	armas[2].killMsg = " multilou ";
	armas[2].dano_minimo = 2;
	armas[2].dano_maximo = 4;
	armas[2].stats[0] = 1; // Destreza
	armas[2].stats[1] = 3; // Crit
	armas[2].stats[2] = 3; // Dodge
	armas[2].stats[3] = 3; // Multi-Shot

	armas[3].nome = "Rifle de assalto";
	armas[3].descricaoAttk = " atirou em ";
	armas[3].killMsg = " fuzilou ";
	armas[3].dano_minimo = 5;
	armas[3].dano_maximo = 8;
	armas[3].stats[0] = -1; // Destreza
	armas[3].stats[1] = 2; // Crit
	armas[3].stats[2] = 1; // Dodge
	armas[3].stats[3] = 2; // Multi-Shot

	armas[4].nome = "Metralhadora pesada";
	armas[4].descricaoAttk = " atirou em ";
	armas[4].killMsg = " aniquilou ";
	armas[4].dano_minimo = 3;
	armas[4].dano_maximo = 5;
	armas[4].stats[0] = -4; // Destreza
	armas[4].stats[1] = -1; // Crit
	armas[4].stats[2] = -3; // Dodge
	armas[4].stats[3] = 4; // Multi-Shot

	armas[5].nome = "Pistola";
	armas[5].descricaoAttk = " atirou em ";
	armas[5].killMsg = " estorou o miolos de ";
	armas[5].dano_minimo = 5;
	armas[5].dano_maximo = 7;
	armas[5].stats[0] = 1; // Destreza
	armas[5].stats[1] = 1; // Crit
	armas[5].stats[2] = 1; // Dodge
	armas[5].stats[3] = 1; // Multi-Shot

	armas[6].nome = "Revolver";
	armas[6].descricaoAttk = " atirou em ";
	armas[6].killMsg = " fez um que ota em ";
	armas[6].dano_minimo = 4;
	armas[6].dano_maximo = 10;
	armas[6].stats[0] = 2; // Destreza
	armas[6].stats[1] = 6; // Crit
	armas[6].stats[2] = 3; // Dodge
	armas[6].stats[3] = 1; // Multi-Shot
	return armas;
}

Jogador* GerarJogador(Arma* arma) {
	Jogador* jogador = new Jogador;
	jogador->vida = jogador->vidaMaxima;
	jogador->arma = arma;
	return jogador;
}

Inimigo* GerarInimigosPreset(Arma* listaArmas, int levelId) {
	/* Cada fase possui uma lista de inimigos que podem surgir
	* essa é a lista contendo todos os inimigos que PODEM surgir
	*/
	Inimigo* inimigos = new Inimigo[4];
	switch (levelId)
	{
	case 0:
		inimigos[0].nome = "Valentino";
		inimigos[0].vidaMaxima = 30;
		inimigos[0].vida = inimigos[0].vidaMaxima;
		inimigos[0].arma = &listaArmas[RNG(0, 2)]; // Items 0-1
		inimigos[0].spriteFile = "Valentino.txt";
		inimigos[0].spriteColor = 5; // Roxo
		inimigos[0].stats[0] = 1; // Destreza
		inimigos[0].stats[1] = 1; // Crit base
		inimigos[0].stats[2] = 1; // Dodge base

		inimigos[1].nome = "Valentino";
		inimigos[1].vidaMaxima = 20;
		inimigos[1].vida = inimigos[1].vidaMaxima;
		inimigos[1].arma = &listaArmas[RNG(5, 2)]; // Items 5-6
		inimigos[1].spriteFile = "Valentino.txt";
		inimigos[1].spriteColor = 14; // Amarelo
		inimigos[1].stats[0] = 1; // Destreza
		inimigos[1].stats[1] = 1; // Crit base
		inimigos[1].stats[2] = 1; // Dodge base

		inimigos[2].nome = "Tyger Claw";
		inimigos[2].vidaMaxima = 25;
		inimigos[2].vida = inimigos[2].vidaMaxima;
		inimigos[2].arma = &listaArmas[3]; // Rifle de assalto
		inimigos[2].spriteFile = "TygerClaw.txt";
		inimigos[2].spriteColor = 3; // Azul
		inimigos[2].stats[0] = 3; // Destreza
		inimigos[2].stats[1] = 0; // Crit base
		inimigos[2].stats[2] = 2; // Dodge base

		inimigos[3].nome = "Tyger Claw";
		inimigos[3].vidaMaxima = 30;
		inimigos[3].vida = inimigos[3].vidaMaxima;
		inimigos[3].arma = &listaArmas[2]; // Mantis Blade
		inimigos[3].spriteFile = "TygerClaw.txt";
		inimigos[3].spriteColor = 3; // Azul
		inimigos[3].stats[0] = 3; // Destreza
		inimigos[3].stats[1] = 0; // Crit base
		inimigos[3].stats[2] = 2; // Dodge base
		break;

	case 1:
		inimigos[0].nome = "Maelstrom";
		inimigos[0].vidaMaxima = 30;
		inimigos[0].vida = inimigos[0].vidaMaxima;
		inimigos[0].arma = &listaArmas[RNG(0, 3)]; // Items 0-2
		inimigos[0].spriteFile = "Maelstrom.txt";
		inimigos[0].spriteColor = 15; // Cinza
		inimigos[0].stats[0] = 1; // Destreza
		inimigos[0].stats[1] = 2; // Crit base
		inimigos[0].stats[2] = 1; // Dodge base

		inimigos[1].nome = "Maelstrom";
		inimigos[1].vidaMaxima = 35;
		inimigos[1].vida = inimigos[1].vidaMaxima;
		inimigos[1].arma = &listaArmas[RNG(5, 2)]; // Items 5-6
		inimigos[1].spriteFile = "Maelstrom.txt";
		inimigos[1].spriteColor = 12; // Carmesin
		inimigos[1].stats[0] = 1; // Destreza
		inimigos[1].stats[1] = 2; // Crit base
		inimigos[1].stats[2] = 1; // Dodge base

		inimigos[2].nome = "Animal";
		inimigos[2].vidaMaxima = 35;
		inimigos[2].vida = inimigos[2].vidaMaxima;
		inimigos[2].arma = &listaArmas[3]; // Rifle de assalto
		inimigos[2].spriteFile = "Animal.txt";
		inimigos[2].spriteColor = 3; // Azul
		inimigos[2].stats[0] = 2; // Destreza
		inimigos[2].stats[1] = 1; // Crit base
		inimigos[2].stats[2] = 3; // Dodge base

		inimigos[3].nome = "Cyberpsycho";
		inimigos[3].vidaMaxima = 50;
		inimigos[3].vida = inimigos[3].vidaMaxima;
		inimigos[3].arma = &listaArmas[RNG(0, 7)]; // Tudo
		inimigos[3].spriteFile = "Cyberpsycho.txt";
		inimigos[3].spriteColor = 3; // Azul
		inimigos[3].stats[0] = -3; // Destreza
		inimigos[3].stats[1] = 1; // Crit base
		inimigos[3].stats[2] = 1; // Dodge base
		break;

	case 2:
		inimigos[0].nome = "Max-Tac";
		inimigos[0].vidaMaxima = 40;
		inimigos[0].vida = inimigos[0].vidaMaxima;
		inimigos[0].arma = &listaArmas[3]; // Rifle de assalto
		inimigos[0].spriteFile = "MaxTac.txt";
		inimigos[0].spriteColor = 11; // Ciano
		inimigos[0].stats[0] = 2; // Destreza
		inimigos[0].stats[1] = 2; // Crit base
		inimigos[0].stats[2] = 2; // Dodge base
		inimigos[0].stats[3] = 1; // Sandevistan Bonus

		inimigos[1].nome = "Tril.-Redes";
		inimigos[1].vidaMaxima = 35;
		inimigos[1].vida = inimigos[1].vidaMaxima;
		inimigos[1].arma = &listaArmas[3]; // Mantis blade
		inimigos[1].spriteFile = "MaxTac.txt";
		inimigos[1].spriteColor = 2; // Verde escuro
		inimigos[1].stats[0] = -2; // Destreza
		inimigos[1].stats[1] = 4; // Crit base
		inimigos[1].stats[2] = -2; // Dodge base
		inimigos[1].stats[3] = 1; // Sandevistan Bonus

		inimigos[2].nome = "Cyberpsycho";
		inimigos[2].vidaMaxima = 50;
		inimigos[2].vida = inimigos[2].vidaMaxima;
		inimigos[2].arma = &listaArmas[RNG(0, 7)]; // Tudo
		inimigos[2].spriteFile = "Cyberpsycho.txt";
		inimigos[2].spriteColor = 3; // Azul
		inimigos[2].stats[0] = -3; // Destreza
		inimigos[2].stats[1] = 1; // Crit base
		inimigos[2].stats[2] = 1; // Dodge base

		inimigos[3].nome = "Cyberpsycho";
		inimigos[3].vidaMaxima = 50;
		inimigos[3].vida = inimigos[3].vidaMaxima;
		inimigos[3].arma = &listaArmas[RNG(0, 7)]; // Tudo
		inimigos[3].spriteFile = "Cyberpsycho.txt";
		inimigos[3].spriteColor = 3; // Azul
		inimigos[3].stats[0] = -3; // Destreza
		inimigos[3].stats[1] = 1; // Crit base
		inimigos[3].stats[2] = 1; // Dodge base
		break;
	}
	return inimigos;
}

Inimigo* EscolherInimigos(Inimigo* inimigosPreset, int numInimigos) {
	// Escolhe os inimigos para gerar com base na função anterior
	Inimigo* inimigos = new Inimigo[numInimigos];

	for (int i = 0; i < numInimigos; i++) {
		int indexEscolhido = RNG(0, 3);
		Inimigo inimigoEscolhido = inimigosPreset[indexEscolhido];
		inimigos[i].nome = inimigoEscolhido.nome;
		inimigos[i].vida = inimigoEscolhido.vida;
		inimigos[i].arma = inimigoEscolhido.arma;
		inimigos[i].spriteFile = inimigoEscolhido.spriteFile;
		inimigos[i].spriteColor = inimigoEscolhido.spriteColor;
	}

	return inimigos;
}

bool VerificarOpcao(int valor, int min, int max) {
	// Verifica se o valor está dentro do valor minimo e maximo
	if (valor >= min && valor <= max) {
		return true;
	}
	return false;
}

void LimparInputBuffer() {
	/*Apertar teclas antes da leitura resulta
	no armazenamento da tecla no buffer
	então essa função é chamada antes para a limpeza do mesmo*/
	while (_kbhit()) _getch();
}

void EsperarInput(char teclaEsperada, int waitTimer) {
	// Usado para os "Aperte qualquer tecla para continuar"
	// ele também pode esperar uma tecla especifica
	while (true) {
		char tecla = _getch();
		if (teclaEsperada == '0' || tecla == teclaEsperada)
			break;
		Sleep(waitTimer);
	}
	LimparInputBuffer();
}

void Display(string msg, int coordX, int coordY, int textColor, bool showCursor, bool centralize) {
	/*Função usada para todo o display do jogo
	* com parametros para posicionamento via coordenadas,
	* ajuste de visibilidade do cursor, centralização e cores
	*/
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
}

void DisplayAnimation(string filePath, string fileName, int frames, int textColor, int cornerX, int cornerY, int waitTime) {
	// Função para display de arquivos .txt
	// Usado no display das ASCII Arts
	string linha;
	ifstream frame;
	int i = 0; // Linha atual
	int j = 0; // Frame atual
	switch (frames)
	{
	case -1: // Mostra apenas um frame
		frame.open(filePath + fileName);
		while (getline(frame, linha)) {
			Display(linha, cornerX, cornerY + i, textColor);
			i++;
		}

		frame.close();
		break;

	default: // Abre sequencia de frames com esperas de waitTime(milisegundos)
		for (j; j < frames; j++) {
			i = 0;
			frame.open(filePath + fileName + to_string(j) + ".txt");
			while (getline(frame, linha)) {
				Display(linha, cornerX, cornerY + i, textColor);
				i++;
			}

			frame.close();
			Sleep(waitTime);
		}
		break;
	}
}

void LimparCores() {
	/* Se o console escrever novos textos (incluindo ajustar tela)
	* ele irá usar a ultima cor usada, podendo quebrar o jogo
	* Esaa função cria faz um display vazio, porém cor a cor padrão
	* para resolver isso
	*/
	Display("", 50, 12, 7);
}

void LimparTela(int tipo) {
	// Limpeza total da tela ou em espaços especificos
	int Y = 0;
	switch (tipo)
	{
	case 1: // Limpa informações inferiores da tela de combate
		Y = 23;
		for (Y; Y < 30; Y++) {
			Display("                                               ", 0, Y, 10);
		}

		Y = 23;
		for (Y; Y < 30; Y++) {
			Display("                             ", 50, Y, 10);
		}
		break;

	case 2: // Limpa log de combate
		Y = 1;
		for (Y; Y < 29; Y++) {
			Display("                                      ", 100, Y, false, 10, true);
		}
		break;

	case 3: // Limpa menu de escolhas de combate
		Y = 23;
		for (Y; Y < 30; Y++) {
			Display("                                               ", 0, Y, 10);
		}
		break;

	default:
		LimparCores();
		system("cls");
		break;
	}
}

void Carregar_Menu() {
	Display("", 50, 12, 7, true);
	Sleep(1000);
	DisplayAnimation("Frames/MenuAnimated/", "MenuFrame_", 9, 10, 10, 2, 50);
	Display("Cyberpunk++", 50, 1, 10, false, true);
	Sleep(500);
	Display("Desenvolvido por:", 85, 3, 14, false, true);
	Display("Eduardo da Rocha Weber", 85, 5, 14, false, true);
	Display("Herick Vitor Vieira Bittencourt", 85, 6, 14, false, true);
	Display("Eduardo Miguel Fuchs Perez", 85, 7, 14, false, true);
	Display("Aperte qualquer tecla para comecar", 50, 25, 160, false, true);
	LimparCores();
}

void Carregar_Tutorial() {
	DisplayAnimation("Frames/Tutorial/", "Tutorial.txt", -1, 7, 50, 1);

	Display("Tutorial Basico:", 30, 2, 7, false, true);

	Display("#", 20, 4, corJogador);
	Display("<--- Jogador", 30, 4, 7, false, true);
	Display("#", 20, 7, corInimigo);
	Display("<--- Inimigo", 30, 7, 7, false, true);

	Display(" ", 20, 11, corChao);
	Display("<--- Chao", 30, 11, 7, false, true);
	Display(" ", 20, 14, corObstaculo);
	Display("<--- Parede", 30, 14, 7, false, true);

	Display("Use as teclas WASD para se movimentar pelo mapa (Incluindo diagonalmente)", 50, 23, 7, false, true);
	Display("Objetivo: mate todos os inimigos e derrote Adam Smasher", 50, 25, 7, false, true);
	Sleep(1000);
	Display("Aperte qualquer tecla para comecar", 50, 27, 160, false, true);
	LimparCores();
}

bool VerificarCoord(Fase* fase, int tipo, int coords[]) {
	/* Verificações da coordenada fornecida dependendo do tipo
	* fornecido
	*/
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
	mapa.blocos = new Bloco *[A]; // Geração de linhas

	for (int i = 0; i < A; i++) { // Geração de colunas
		mapa.blocos[i] = new Bloco[L];
	}

	Display("Gerando obstaculos", 50, 12, 10, false, true);
	for (int i = 0; i < A; i++) {
		for (int j = 0; j < L; j++) {
			int temObstaculo = RNG(1, 10); // 1 até 10
			if (temObstaculo <= 2) { // 20% chance de bloquear
				mapa.blocos[i][j].bloqueado = true;
			}
		}
	}

	return mapa;
}

Fase* CriarFase(int numInimigos, Inimigo* inimigos, string nome, int alturaMapa, int larguraMapa) {
	Fase* fase = new Fase;
	fase->nome = nome;
	fase->inimigosRestantes = numInimigos;
	Display("Gerando mapa", 50, 11, 10, false, true);
	fase->mapa = CriarMapa(alturaMapa, larguraMapa);

	Display("Gerando inimigos", 50, 13, 10, false, true);
	for (int i = 0; i < numInimigos;) { // For loop sem incremento automatico
		int coordEscolhida[2] = { RNG(0, fase->mapa.A), RNG(0, fase->mapa.L) };

		if (VerificarCoord(fase, 0, coordEscolhida)) { // Espaço existe?
			if (VerificarCoord(fase, 1, coordEscolhida) == false && VerificarCoord(fase, 2, coordEscolhida) == false) {
				// Bloco livre, sem inimigos também
				fase->mapa.blocos[coordEscolhida[0]][coordEscolhida[1]].inimigo = &inimigos[i];
				fase->mapa.blocos[coordEscolhida[0]][coordEscolhida[1]].temInimigo = true;
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

	for (int i = 0; i < L; i++) { // Criação da borda horizontal
		borda = borda + "-";
	}
	borda = borda + "@";
	Display(borda, bordaX - 1, 10, 7);

	for (int i = 0; i < A; i++) { // Display da matriz mapa
		for (int j = 0; j < L; j++) {

			Display(" ", bordaX + j, 11 + i, corChao);


			if (fase->mapa.blocos[i][j].bloqueado == true) {
				Display(" ", bordaX + j, 11 + i, corObstaculo);
			}

			if (fase->mapa.blocos[i][j].inimigo != NULL) {
				Display("#", bordaX + j, 11 + i, corInimigo);
			}

			if (jogador->posicao[0] == i && jogador->posicao[1] == j) {
				Display("#", bordaX + j, 11 + i, corJogador);
				jogador->posicaoTela = ObterPosicaoCursor(GetStdHandle(STD_OUTPUT_HANDLE), true);
			}

			Display("|", bordaX - 1, 11 + i);
			Display("|", bordaX + j + 1, 11 + i);
		}
	}
	Display(borda, bordaX - 1, 11 + A);
	Display("Fase: " + fase->nome, 50, 1, 10, false, true);
}

void DisplayStatusJogador(Jogador* jogador) { // Status do jogador no mapa
	Display("          ", 50, 2, 10, false, true);
	Display("Vida: " + to_string(jogador->vida), 50, 2, 10, false, true);
	Display("                  ", 50, 3, 10, false, true);
	Display("Sobrecarga: " + to_string(jogador->stats[0]) + "%", 50, 3, 10, false, true);
}

void Movimentar(Jogador* jogador, Fase* fase) {
	DisplayStatusJogador(jogador);
	int moveDelta[2] = { 0 };
	bool debounce[4] = { false }; // Evita repetir tecla no mesmo frame
	for (int i = 0; i < 2; i++) {
		if (_kbhit()) { // Detecção de até 2 teclas em um frame, permitindo diagonais
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

			if (tecla == 'R' || tecla == 'r') { // Keybind p/ recriar fase
				fase->reset = true;
			}
		}
	}

	if (moveDelta[0] != 0 || moveDelta[1] != 0) { // Personagem se moveu?
		int novaPosicao[2] = { 0 };
		novaPosicao[0] = jogador->posicao[0] + moveDelta[0]; // Y
		novaPosicao[1] = jogador->posicao[1] + moveDelta[1]; // X

		COORD novaPosicaoTela = { jogador->posicaoTela.X, jogador->posicaoTela.Y };
		novaPosicaoTela.X += moveDelta[1];
		novaPosicaoTela.Y += moveDelta[0];

		if (VerificarCoord(fase, 0, novaPosicao)) { // Espaço existe?
			if (VerificarCoord(fase, 1, novaPosicao) == false) { // Espaço está livre?
				Display(" ", jogador->posicaoTela.X, jogador->posicaoTela.Y, corChao);
				jogador->posicao[0] = novaPosicao[0];
				jogador->posicao[1] = novaPosicao[1];
				Display("#", novaPosicaoTela.X, novaPosicaoTela.Y, corJogador);
				jogador->posicaoTela = novaPosicaoTela;

				if (jogador->stats[0] > 0 && jogador->stats[0] < 100) { // Descarregamento por passos
					jogador->stats[0] -= 5; // -5% sobrecarga por passo
					if (jogador->stats[0] <= 0) {
						jogador->stats[0] = 0;
					}
				}

				if (jogador->vida > 0) { // Regeneração por passos
					jogador->vida += 5; // +5 vida por passo
					if (jogador->vida >= jogador->vidaMaxima) {
						jogador->vida = jogador->vidaMaxima;
					}
				}
			}
		}
	}
	Sleep(50);
}

void escolhaSaque(Jogador* jogador, Inimigo* inimigo) {
	/*Tela para decidir roubar a arma do inimigo falecido
	* ou manter a arma
	*/
	LimparTela();
	Display("Item encontrado", 50, 10, 10, false, true);
	Display(inimigo->arma->nome + " de " + inimigo->nome + " parece ainda funcionar, substituir arma atual?", 50, 11, 10, false, true);

	Display("Sua arma: " + jogador->arma->nome, 15, 13, 10);
	Display("Dano minimo: " + to_string(jogador->arma->dano_minimo), 15, 14, 10);
	Display("Dano maximo (sem crits): " + to_string(jogador->arma->dano_maximo), 15, 15, 10);
	Display("Destreza bonus: " + to_string(jogador->arma->stats[0]), 15, 16, 10);
	Display("Critico bonus: " + to_string(jogador->arma->stats[1]), 15, 17, 10);
	Display("Dodge bonus: " + to_string(jogador->arma->stats[2]), 15, 18, 10);

	if (jogador->arma->stats[3] > 1) {
		Display("Multiplos ataques? SIM (" + to_string(jogador->arma->stats[3]) + ")", 15, 19, 10);
	}
	else {
		Display("Multiplos ataques? NAO", 15, 19, 10);
	}

	Display("Arma do inimigo: " + inimigo->arma->nome, 65, 13, 10);
	Display("Dano minimo: " + to_string(inimigo->arma->dano_minimo), 65, 14, 10);
	Display("Dano maximo (sem crits): " + to_string(inimigo->arma->dano_maximo), 65, 15, 10);
	Display("Destreza bonus: " + to_string(inimigo->arma->stats[0]), 65, 16, 10);
	Display("Critico bonus: " + to_string(inimigo->arma->stats[1]), 65, 17, 10);
	Display("Dodge bonus: " + to_string(inimigo->arma->stats[2]), 65, 18, 10);

	if (inimigo->arma->stats[3] > 1) {
		Display("Multiplos ataques? SIM (" + to_string(inimigo->arma->stats[3]) + ")", 65, 19, 10);
	}
	else {
		Display("Multiplos ataques? NAO", 65, 19, 10);
	}

	Display("1 - Sim", 50, 21, 10, false, true);
	Display("2 - Nao", 50, 22, 10, false, true);
	int escolha = 0;
	while (escolha <= 0) {
		if (escolha == -1) {
			Display("Opcao invalida! ", 50, 25, 4, true, true);
		}
		Display("                                    ", 50, 24, 10, true, true);
		Display("Opcao Escolhida: ", 50, 24, 10, true, true);
		cin >> escolha;

		if (VerificarOpcao(escolha, 1, 2)) {
			if (escolha == 1) { // Trocar de arma
				jogador->arma = inimigo->arma;
			}
		}
		else {
			escolha = -1;
		}
	}
}

void iniciarCombate(Jogador* jogador, Inimigo* inimigo) {
	// LOG DE COMBATE
	Display("LOG DE COMBATE:", 100, 0, 10, false, true);
	Display("Voce encontrou " + inimigo->nome, 100, 1, 10, false, true);
	int Ylog = 2;

	while (morreu(jogador) == false && morreu(inimigo) == false) {
		DisplayAnimation("Frames/Enemies/", inimigo->spriteFile, -1, inimigo->spriteColor, 0, 0);
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
		Display("Relatorio de integridade:", 50, 23, 10);

		if (jogador->vida > jogador->vidaMaxima / 2) {
			Display("Vida: " + to_string(jogador->vida), 50, 24, 10);
		}
		else {
			Display("Vida: " + to_string(jogador->vida) + "(PERIGO!)", 50, 24, 4);
		}


		if (jogador->stats[0] >= 50) { // Sobrecarga
			if (jogador->stats[0] >= 75) {
				if (jogador->stats[0] >= 100) {
					Display("Sobrecarga: CIBERPSICOSE", 50, 25, 4);
				}
				else {
					Display("Sobrecarga: " + to_string(jogador->stats[0]) + "% (PERIGO)", 50, 25, 4);
				}

			}
			else {
				Display("Sobrecarga: " + to_string(jogador->stats[0]) + "%", 50, 25, 14);
			}
		}
		else {
			Display("Sobrecarga: " + to_string(jogador->stats[0]) + "%", 50, 25, 10);
		}

		if (jogador->stats[2] > 0) { // Sandevistan status
			Display("Sandevistan ativo! (" + to_string(jogador->stats[2]) + ")", 50, 26, 10);
		}

		if (jogador->stats[3] > 0) { // Imp. Curativo status
			Display("Regeneracao em progresso (" + to_string(jogador->stats[3]) + ")", 50, 27, 10);
		}

		if (jogador->stats[4] > 0) { // Kiroshi Optic status
			Display("Assistencia de mira em uso!", 50, 28, 10);
		}

		// DECISÃO DE ATAQUE
		int escolha = 0;
		int escolhaAux = 0; // Variavel temporaria para decisões
		if (jogador->stats[0] < 100) { // Livre escolha caso n tenha ciberpsicose
			while (escolha <= 0) {
				LimparTela(3);
				if (escolha == -1) {
					Display("Opcao invalida!", 1, 29, 4);
				}

				Display("O que fazer? [[Vida do inimigo: " + to_string(inimigo->vida) + "]]", 1, 23, 10);
				Display("1 - Atacar", 1, 24, 10);
				Display("2 - Bloquear", 1, 25, 10);
				Display("3 - Usar Implante", 1, 26, 10);
				Display("                                               ", 1, 28);
				Display("Opcao Escolhida: ", 1, 28, 10, true);
				LimparInputBuffer();
				cin >> escolhaAux;
				if (VerificarOpcao(escolhaAux, 1, 3)) {
					if (VerificarOpcao(escolhaAux, 1, 2)) { // Escolhas primarias
						escolha = escolhaAux;

					}
					else { // Abrir menu de implantes
						LimparTela(3);
						Display("Escolha um implante:", 1, 23, 10);
						Display("1 - Sandevistan (varias acoes) (" + to_string(custoSandevistan) + "%)", 1, 24, 10);
						Display("2 - Imp. Regenerativo (+20%HP, 2 turnos) (" + to_string(custoImpReg) + "%)", 1, 25, 10);
						Display("3 - Kiroshi Opctics (Precisao aumentada) (" + to_string(custoKiroshiOptics) + "%)", 1, 26, 10);
						Display("4 - Voltar", 1, 27, 10);
						Display("                                               ", 1, 28);
						Display("Opcao Escolhida: ", 1, 28, 10, true);
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


		}
		else { // Ciberpsicose, combate automatico
			escolhaAux = RNG(1, 3);

			if (VerificarOpcao(escolhaAux, 1, 2)) { // Escolhas Principais
				escolha = escolhaAux;
			}
			else { // Escolha de implante
				escolhaAux = RNG(1, 3);
				escolha = 10 + escolhaAux;
			}
		}

		if (Ylog > 20) { // Limpeza do log de combate
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
			Display("Voce ativou OverS. (" + to_string(jogador->stats[5]) + "dmg DEF)", 100, Ylog, 10, false, true);
			Ylog++;
			if (jogador->stats[2] <= 0) { // Sandevistan check
				ataque(inimigo, jogador, Ylog);
			}
			break;

		case 11: // Sandevistan
			Display("Voce usou o Sandevistan!", 100, Ylog, 10, false, true);
			Ylog++;
			jogador->stats[2] = 3; // Sandevistan = +2 ações + ação atual
			if (jogador->stats[0] < 100) {
				jogador->stats[0] += custoSandevistan; // + Sobrecarga
			}
			else {
				jogador->vida -= custoSandevistan; // Perca de vida por ciberpsicose
			}
			break;

		case 12: // Implante Regenerativo
			Display("Voce usou o I. Regenerativo!", 100, Ylog, 10, false, true);
			Ylog++;
			jogador->stats[3] = 2; // Implante renegerativo = 2 turnos de cura

			if (jogador->stats[0] < 100) {
				jogador->stats[0] += custoImpReg; // + Sobrecarga - 10
			}
			else {
				jogador->vida -= custoImpReg; // Perca de vida por ciberpsicose
			}

			if (jogador->stats[2] <= 0) { // Sandevistan check
				ataque(inimigo, jogador, Ylog);
			}
			break;

		case 13: // Kiroshi Optics
			Display("Voce usou o Kiroshi Opctics!", 100, Ylog, 10, false, true);
			Ylog++;
			jogador->stats[4] = 2; // Kiroshi Optics = Prox. Turno c/ precisão aumentada

			if (jogador->stats[0] < 100) {
				jogador->stats[0] += custoKiroshiOptics; // + Sobrecarga - 10
			}
			else {
				jogador->vida -= custoKiroshiOptics; // Perca de vida por ciberpsicose
			}

			if (jogador->stats[2] <= 0) { // Sandevistan check
				ataque(inimigo, jogador, Ylog);
			}
			break;

		default:
			break;
		}
		Sleep(10);
	}

	LimparTela(1);
	if (morreu(inimigo) && morreu(jogador) == false) { // Vitoria
		Display("Voce" + jogador->arma->killMsg + inimigo->nome, 1, 23, 10);
		// Remove todos os buffs de implantes, combate acabou
		jogador->stats[2] = 0; // -Sandevistan
		jogador->stats[3] = 0; // -Implante Regenerativo
		jogador->stats[4] = 0; // -Kiroshi Optics
	}
	else { // Derrota
		Display(inimigo->nome + inimigo->arma->killMsg + "voce", 1, 23, 10);
	}
	Display("Aperte qualquer tecla p/ continuar", 1, 24, 160);
	LimparCores();
	LimparInputBuffer();
	EsperarInput();

	if (morreu(inimigo) && morreu(jogador) == false && jogador->arma != inimigo->arma && inimigo->arma->nome != "Punhos" && RNG(0, 2) == 1) {
		/*Condições para tela de saque:
		1 - Você precisa estar vivo e o inimigo morreu

		2 - Os items não podem ser iguais

		3 - O item inimigo não pode ser punhos

		4 - Existe apenas 50% de chance de este evento acontecer
		*/
		escolhaSaque(jogador, inimigo);
	}
}

void ataque(Jogador* atacante, Inimigo* defensor, int &Ylog)
{
	int totalTiros = atacante->arma->stats[3];
	if (atacante->arma->stats[3] <= 0) {
		totalTiros = 1;
	}
	for (int i = 0; i < totalTiros; i++) {
		int dano = RNG(atacante->arma->dano_minimo, atacante->arma->dano_maximo - atacante->arma->dano_minimo + 1);

		int acertou = RNG(1 - defensor->stats[2], 20); // 1 - Dodge base inimigo
		if (atacante->stats[4]) {
			acertou = RNG(5 - defensor->stats[2], 20); // Kiroshi Optics ativo
		}

		if (atacante->stats[2] > 0 && defensor->stats[3] <= 0) { // Sandevistan check
			acertou = 10; // Se sandevistan ativo e inimigo não tem sandevistan, acertou!
		}
		if (acertou >= 10) { // Alvo acertado
			DisplayAnimation("Frames/Enemies/", defensor->spriteFile, -1, 4, 0, 0);
			int crit = RNG(1 + atacante->arma->stats[1], 20); // 1 + Crit bonus arma

			if (atacante->stats[4]) {
				crit = RNG(3 + atacante->arma->stats[1], 20); // Kiroshi Crit bonus + Crit arma
			}

			if (crit >= 10) {
				dano *= 1.25;
			}
			defensor->vida -= dano;
			if (defensor->vida < 0) {
				defensor->vida = 0;
			}

			if (crit < 10) {
				Display("Voce" + atacante->arma->descricaoAttk + defensor->nome + " (-" + to_string(dano) + "HP)", 100, Ylog, 10, false, true);
			}
			else {
				Display("Voce" + atacante->arma->descricaoAttk + defensor->nome + " (-" + to_string(dano) + "HP!)", 100, Ylog, 10, false, true);
			}
		}
		else {
			Display("Voce errou!", 100, Ylog, 10, false, true);
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
			Display("Bloqueado " + to_string(danoOriginal) + "dmg", 100, Ylog, 10, false, true);
			Ylog++;
		}
		else {
			if (overshieldDestruido == false && bloqueando) {
				overshieldDestruido = true;
				Display("Overshield quebrou!", 100, Ylog, 10, false, true);
				Ylog++;
			}
		}

		if (acertou >= 10) { // Alvo acertado
			int crit = RNG(1 + atacante->stats[1], 20); // 1 + Crit base inimigo

			if (crit >= 10) {
				dano *= 1.25;
			}
			defensor->vida -= dano;
			if (defensor->vida < 0) {
				defensor->vida = 0;
			}

			if (crit < 10) {
				Display(atacante->nome + atacante->arma->descricaoAttk + +"voce (-" + to_string(dano) + "HP)", 100, Ylog, 10, false, true);
			}
			else {
				Display(atacante->nome + atacante->arma->descricaoAttk + "voce (-" + to_string(dano) + "HP!)", 100, Ylog, 10, false, true);
			}
		}
		else {
			Display(atacante->nome + " errou!", 100, Ylog, 10, false, true);
		}
		Ylog++;
		Sleep(100);
	}
	defensor->stats[5] = 0; // Remove overshield do jogador
}

void jogarFase(Jogador* jogador, Fase* fase)
{
	while (morreu(jogador) == false && fase->ganhou == false && fase->reset == false)
	{
		Movimentar(jogador, fase);
		if (fase->mapa.blocos[jogador->posicao[0]][jogador->posicao[1]].temInimigo) {
			// Inimigo encontrado, iniciar combate
			LimparTela();
			DisplayAnimation("Frames/CombatInitiation/", "Shatter_", 12, 10, 4, 2, 10);
			LimparTela();
			iniciarCombate(jogador, fase->mapa.blocos[jogador->posicao[0]][jogador->posicao[1]].inimigo);
			// Combate encerrou
			if (morreu(jogador) == false) {
				fase->mapa.blocos[jogador->posicao[0]][jogador->posicao[1]].inimigo = NULL;
				fase->mapa.blocos[jogador->posicao[0]][jogador->posicao[1]].temInimigo = false;
				fase->inimigosRestantes--;
				if (fase->inimigosRestantes <= 0) {
					fase->ganhou = true;
				}
				LimparTela();
				DisplayFase(fase, jogador);
				LimparInputBuffer();
			}
		}
	}
}

#pragma endregion
