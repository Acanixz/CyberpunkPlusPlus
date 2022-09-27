#include <iostream>
#include <string>
#include <time.h>
using namespace std;

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
};

struct Inimigo
{
    string nome;
    int vida;
    Arma arma;
};

struct Fase
{
    string nome;
    Inimigo inimigos[5];
};

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
                cout << "O jogador morreu, o jogo acabou";
                return;
            }
        }

        cout << fase.inimigos[atual].nome << " foi morto" << endl
             << endl;
    }

    cout << "O jogador passou a fase";
}

int main()
{
    srand(time(NULL));

    Arma aI = {1, 5};
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

    jogar_fase(jog, fase);
}