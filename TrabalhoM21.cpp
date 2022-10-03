#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <conio.h>
#include <Windows.h>
#include <locale.h>
using namespace std;

// Structs // 
struct Arma
{
    int dano_minimo;
    int dano_maximo;
};

struct Jogador
{
    int nivel;
    int vida;
    Arma arma;
    int posicao[2];
};

struct Inimigo
{
    string nome;
    int vida;
    Arma arma;
};

struct Bloco
{
    bool bloqueado = false;
    Inimigo *inimigo;
};

struct Fase
{
    string nome;
    Bloco* mapa[];
    Inimigo inimigos[5];
};

// Funções //

int VerificarOpcao(int valor = 0, int min = 0, int max = 0){
    if (valor >= min || valor <= max){
        return valor;
    }
    return -1;
}

void LimparInputBuffer(){
    while(kbhit()) getch();
}

void EsperarInput(char teclaEsperada = '0', int waitTimer = 50){
    while (true){
        char tecla = getch();
        if (kbhit())
            cout << "Pressionado!" << endl;
            if (teclaEsperada == '0' || tecla == teclaEsperada)
                break;
        Sleep(waitTimer);
    }
    LimparInputBuffer();
}

void Display(string msg, int coordX = -1, int coordY = -1, bool Newline = true, int textColor = 7, bool showCursor = false, bool centralize = false){
    HANDLE STD_OH = GetStdHandle(STD_OUTPUT_HANDLE);
    if (coordX >= 0 || coordY >= 0){
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(STD_OH, &cursorInfo);
        COORD coord;
        
        if (coordX >= 0)
            coord.X = coordX;
            if (centralize)
                coord.X = coord.X - msg.length()/2;
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

void DisplayAnimation(string filePath, string fileName, int frames = -1, int textColor = 7, int cornerX = -1, int cornerY = -1, int waitTime = 1000){
    string linha;
    ifstream frame;
    int i = 0; // Linha atual
    int j = 0; // Frame atual
    switch (frames)
    {
    case -1: // Mostra apenas um frame
        /* code */
        frame.open(filePath + fileName);
        while (getline(frame, linha)){
            Display(linha, cornerX, cornerY + i, false, textColor);
            i++;
        }
        
        frame.close();
        break;
    
    default: // Abre sequencia de frames com esperas de waitTime(milisegundos)
        for (j; j < frames; j++){
            i = 0;
            frame.open(filePath + fileName + to_string(j) + ".txt");
            while (getline(frame, linha)){
                Display(linha, cornerX, cornerY + i, false, textColor);
                i++;
            }
            
            frame.close();
            Sleep(waitTime);
        }
        break;
    }
}

void LimparTela(){
    Display("", 50, 12, false, 7); // Se der cls antes disso, a tela fica com a ultima cor usada
    system("cls");
}

void Carregar_Menu(){
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
}

void Carregar_Tutorial(){
    DisplayAnimation("Frames/Tutorial/", "Tutorial.txt", -1, 7, 50, 1);

    Display("Tutorial Basico:", 30, 2, false, 7, false, true);
    
    Display("*", 20, 4, false, 7);
    Display("<--- Jogador", 30, 4, false, 7, false, true);
    Display("*", 20, 7, false, 4);
    Display("<--- Inimigo", 30, 7, false, 7, false, true);

    Display(" ", 20, 11, false, 38);
    Display("<--- Chao", 30, 11, false, 7, false, true);
    Display(" ", 20, 14, false, 119);
    Display("<--- Parede", 30, 14, false, 7, false, true);

    Display("Use as teclas WASD para se movimentar pelo mapa", 50, 23, false, 7, false, true);
    Display("Objetivo: mate todos os inimigos e derrote Adam Smasher", 50, 25, false, 7, false, true);
    Sleep(2000);
    Display("Aperte qualquer tecla para comecar", 50, 27, false, 160, false, true);
}

/*template <int A, int L>
Fase* GerarFase(){
    Fase* fase = new Fase;
    fase->nome = "Night City";

    //Bloco** mapa = new Bloco*[A]; // Geração de linhas

    auto mapa = new Bloco*[A][L];
    //for (int i = 0; i < A; i++){ // Geração de colunas
    //    mapa[i] = new Bloco[L];
    //}

    cout << mapa[0][0] << endl;
    for (int i = 0; i < A; i++){
        cout << endl;
        for (int j = 0; j < L; j++){
            //cout << mapa[A][L].bloqueado << endl;
        }
    }
    
    

    fase->mapa = *mapa;
    return fase;
}*/

Mapa CriarMapa(int altura, int largura){
    Mapa mapa;
    return mapa;
}

Fase CriarFase(int numInimigos, Inimigo* inimigos, int alturaMapa, int larguraMapa){

}

void DisplayFase(Fase* fase, int A = 10, int L = 20){
    LimparTela();
    string borda = "@";
    int bordaX = 50 - L + 1;

    for (int i = 0; i < L; i++){
        borda = borda + "-";
    }
    Display(borda, 50, 10, false, 10, false, true);

    for (int i = 0; i < A; i++){
        for (int j = 0; j < L; j++){

            Display(" ", bordaX + j, 11 + i, false, 38);

            //if (mapa[i][j].bloqueado)
            //    Display(" ", bordaX + j, 11 + i, false, 119);
        }
    }
}

template <typename T>
bool morreu(T personagem);

template <typename Tata, typename Tdef>
Tdef ataque(Tata atacante, Tdef defensor);

void jogar_fase(Jogador jog, Fase fase);


int main()
{
    setlocale(LC_ALL, "Portuguese");
    srand(time(NULL));

    Carregar_Menu();
    EsperarInput();
    LimparTela();

    Carregar_Tutorial();
    LimparInputBuffer();
    EsperarInput();

    LimparTela();

    Display("Criando Fase..", 50, 5, false, 10, true, true);
    //Fase* fase = GerarFase<10,10>();
    
    //DisplayFase(fase);

    LimparInputBuffer();
    EsperarInput();
    /*Arma aI = {1, 5};
    Arma aJ = {4, 10};

    Inimigo goblin1 = {"Goblin", 20, aI};
    Inimigo goblin2 = {"Goblerto", 30, aI};
    Inimigo goblin3 = {"Gobo", 40, aI};
    Inimigo goblin4 = {"Goblinio", 50, aI};
    Inimigo chefao = {"Juca", 95, aI};

    Jogador jog = {1, 100, aJ};

    Fase fase;
    fase.nome = "Fase 1";
    fase.inimigos[0] = goblin1;
    fase.inimigos[1] = goblin2;
    fase.inimigos[2] = goblin3;
    fase.inimigos[3] = goblin4;
    fase.inimigos[4] = chefao;

    jogar_fase(jog, fase);*/
}

template <typename T>
bool morreu(T personagem)
{
    if (personagem.vida < 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

template <typename Tata, typename Tdef>
Tdef ataque(Tata atacante, Tdef defensor)
{
    // 2 - 5
    int intervalo_dano = atacante.arma.dano_maximo - atacante.arma.dano_minimo + 1;
    int dano = atacante.arma.dano_minimo + rand() % intervalo_dano;

    defensor.vida = defensor.vida - dano;

    return defensor;
}

void jogar_fase(Jogador jog, Fase fase)
{
    cout << "Começou " << fase.nome << endl
         << endl;

    for (int atual = 0; atual < 5; atual++)
    {
        while (!morreu(fase.inimigos[atual]))
        {
            jog = ataque(fase.inimigos[atual], jog);
            fase.inimigos[atual] = ataque(jog, fase.inimigos[atual]);

            cout << "O jogador atacou " << fase.inimigos[atual].nome << " e ele ficou com " << fase.inimigos[atual].vida << " de vida" << endl;
            cout << "O " << fase.inimigos[atual].nome << "atacou o jogador ao mesmo tempo e o deixou com " << jog.vida << " de vida" << endl;

            if (morreu(jog))
            {
                cout << "O jogador morreu, o jogo acabou" << endl;
                return;
            }
        }

        cout << fase.inimigos[atual].nome << " foi morto" << endl << endl;
    }

    cout << "O jogador passou a fase";
}