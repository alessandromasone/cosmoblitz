#define SCREEN_HEIGHT 30
#define SCREEN_WIDTH 40

#define VELOCITA_GIOCATORE 3

#include "lib.h"

#include <iostream>
#include <string>
#include <conio.h>
#include <vector>
#include <time.h>

#define LIVELLI_TOTALI 10

int nemici_per_livello[LIVELLI_TOTALI] = {38, 38 * 2, 38 * 3, 38 * 4, 38 * 5, 38 * 6, 38 * 7, 38 * 8, 38 * 9, 38 * 10};

struct Giocatore
{
    int x, y, potenza, raggioFuoco, tempo, vita, lifeMax, granate, scudi;
    bool morte;
};
struct Nemico
{
    int x, y, potenza, raggioFuoco, tempo, vita;
    bool morte;
    char c;
    unsigned char attr;
};
struct Sparo
{
    int x, y, danno, velocitaX, velocitaY, tempo, tempoVariabile;
    bool morte;
    char c;
    unsigned char attr;
};
struct Miglioramento
{
    int x, y;
    bool ePotenza, eRaggioFuoco, eVita, eCura, eGranata, eScudo, morte;
    char c;
    unsigned char attr;
};
struct Opzioni
{
    bool sparoContinuo;
    unsigned char grigliaAttributo, giocatoreAttributo, parolaAttibuto, vitaAttributo, morteVitaAttributo,
        potenzaAttributo, sfondoAttributo, raggioFuocoAttributo, scudoAttributo, granataAttributo;
};

class Corpo : public Game
{
public:
    Corpo()
    {
        srand(time(NULL));

        o.sparoContinuo = true;
        o.grigliaAttributo = F_GREEN;
        o.giocatoreAttributo = F_LIGHTBLUE;
        o.vitaAttributo = F_RED;
        o.morteVitaAttributo = F_GREY;
        o.parolaAttibuto = F_BRIGTHWHITE;
        o.potenzaAttributo = F_LIGHTYELLOW;
        o.sfondoAttributo = B_BLACK;
        o.raggioFuocoAttributo = F_LIGHTRED;
        o.scudoAttributo = F_BLUE;
        o.granataAttributo = F_GREY;

        attivo = true;
    }

    void run()
    {
        Avvio();
        loopCentrale();
        fine();
    }

private:
    void Avvio()
    {
        ShowConsoleCursor(false);
        SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT, true);
    }

    void disegnaBenvenuto()
    {
        std::string s = "Premi Invio per iniziare!";
        if (cornice > 105)
            cornice -= 105;
        int delta = cornice / 5;
        for (int i = 0; i < 21; ++i)
        {
            if (delta == i)
                FillCell(i + 10, 9, s[i], F_RED | B_WHITE);
            else
                FillCell(i + 10, 10, s[i], F_RED | B_WHITE);
        }

    }

    void disegnaMenuAnimazioni()
    {
        static const int n = 20;
        static bool init = false;
        static std::vector<int> X(n, 0), Y(n, 0), time(n, 0), deltaTime(n, 0), speedX(n, 0), speedY(n, 0);
        static const std::vector<char> C(n, 30); // '▲'
        static std::vector<unsigned char> attr(n, F_WHITE);

        if (!init)
        {
            for (auto &x : X)
                x = rand() % SCREEN_WIDTH;
            for (auto &y : Y)
                y = rand() % SCREEN_HEIGHT;
            for (auto &s : speedY)
                s = rand() % 2 == 0 ? -1 : 1;
            for (auto &s : speedX)
                s = rand() % 2 == 0 ? -1 : 1;
            for (auto &a : attr)
                a = rand() % 16;
            for (auto &t : time)
                t = rand() % 10;
            for (auto &t : deltaTime)
                t = rand() % 10 + 5;

            init = true;
        }

        for (int i = 0; i < n; ++i)
        {
            ++time[i];
            if (time[i] >= deltaTime[i])
            {
                X[i] += speedX[i];
                Y[i] += speedY[i];

                time[i] -= deltaTime[i];

                if (X[i] <= 0 || Y[i] <= 0 || X[i] >= SCREEN_WIDTH || Y[i] >= SCREEN_HEIGHT)
                {
                    X[i] = rand() % SCREEN_WIDTH;
                    Y[i] = rand() % SCREEN_HEIGHT;
                    speedX[i] = rand() % 2 == 0 ? -1 : 1;
                    speedY[i] = rand() % 2 == 0 ? -1 : 1;
                    attr[i] = rand() % 16;
                    time[i] = rand() % 20;
                    deltaTime[i] = rand() % 20 + 10;
                }
            }
        }

        for (int i = 0; i < n; ++i)
            FillCell(X[i], Y[i], C[i], attr[i] | o.sfondoAttributo);
    }

    void disegnaNemici(int toDraw)
    {
        int i = 0;
        while (toDraw > 0)
        {
            if (!nemici[i].morte)
            {
                FillCell(nemici[i].x, nemici[i].y, nemici[i].c, nemici[i].attr | o.sfondoAttributo);
                --toDraw;
            }
            ++i;
        }
    }

    void disegnaGriglia()
    {
        // Testi
        std::string s = "LEVEL " + std::to_string(livello) + " SCORE " + std::to_string(punti);
        for (int i = 0; i < s.size(); ++i)
            FillCell(i + 1, 1, s[i], o.parolaAttibuto | o.sfondoAttributo);

        // Tag speciali
        for (int i = 0; i < g.vita; ++i)
            FillCell(2 + i, 2, 3, o.vitaAttributo | o.sfondoAttributo); // Life '♥'
        for (int i = g.vita; i < g.lifeMax; ++i)
            FillCell(2 + i, 2, 3, o.morteVitaAttributo | o.sfondoAttributo); // Life '♥'
        for (int i = 0; i < g.potenza; ++i)
            FillCell(8 + i, 2, 4, o.potenzaAttributo | o.sfondoAttributo); // Power '♦'
        for (int i = 0; i < (20 - g.raggioFuoco) / 2; ++i)
            FillCell(14 + i, 2, 175, o.raggioFuocoAttributo | o.sfondoAttributo); // FireRate '»'
        for (int i = 0; i < g.scudi / 100; ++i)
            FillCell(20 + i, 2, 157, o.scudoAttributo | o.sfondoAttributo); // Shield 'Ø'
        for (int i = 0; i < g.granate; ++i)
            FillCell(26 + i, 2, 184, o.granataAttributo | o.sfondoAttributo); // Granade '©'

        // Bordi
        for (int i = 1; i < SCREEN_WIDTH - 1; ++i)
        {
            FillCell(i, 0, 205, o.grigliaAttributo | o.sfondoAttributo);                 // '═'
            FillCell(i, 3, 205, o.grigliaAttributo | o.sfondoAttributo);                 // '═'
            FillCell(i, SCREEN_HEIGHT - 1, 205, o.grigliaAttributo | o.sfondoAttributo); // '═'
        }
        FillCell(0, 0, 201, o.grigliaAttributo | o.sfondoAttributo);                // '╔'
        FillCell(0, 1, 186, o.grigliaAttributo | o.sfondoAttributo);                // '║'
        FillCell(0, 2, 186, o.grigliaAttributo | o.sfondoAttributo);                // '║'
        FillCell(0, 3, 204, o.grigliaAttributo | o.sfondoAttributo);                // '╠'
        FillCell(SCREEN_WIDTH - 1, 0, 187, o.grigliaAttributo | o.sfondoAttributo); // '╗'
        FillCell(SCREEN_WIDTH - 1, 1, 186, o.grigliaAttributo | o.sfondoAttributo); // '║'
        FillCell(SCREEN_WIDTH - 1, 2, 186, o.grigliaAttributo | o.sfondoAttributo); // '║'
        FillCell(SCREEN_WIDTH - 1, 3, 185, o.grigliaAttributo | o.sfondoAttributo); // '╣'
        for (int i = 4; i < SCREEN_HEIGHT - 1; ++i)
        {
            FillCell(0, i, 186, o.grigliaAttributo | o.sfondoAttributo);                // '║'
            FillCell(SCREEN_WIDTH - 1, i, 186, o.grigliaAttributo | o.sfondoAttributo); // '║'
        }
        FillCell(0, SCREEN_HEIGHT - 1, 200, o.grigliaAttributo | o.sfondoAttributo);                // '╚'
        FillCell(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 188, o.grigliaAttributo | o.sfondoAttributo); // '╝'
    }

    void disegnaGiocatore()
    {
        FillCell(g.x, g.y, 30, o.giocatoreAttributo | o.sfondoAttributo); // '▲'
    }

    void disegnaSpari()
    {
        for (auto &s : spari)
            FillCell(s.x, s.y, s.c, s.attr | o.sfondoAttributo);
    }

    void disegnaMiglioramenti()
    {
        for (auto &u : miglioramenti)
            FillCell(u.x, u.y, u.c, u.attr | o.sfondoAttributo);
    }

    void spariGiocatore()
    {
        if (g.tempo >= g.raggioFuoco)
        {
            g.tempo = 0;
            spari.push_back({g.x, g.y - 1, g.potenza, 0, -1, 0, g.raggioFuoco / 3 * 2, false, 24, F_LIGHTAQUA}); // '↑'
        }
    }

    void lancioGranata()
    {
        if (g.granate <= 0)
            return;

        for (auto &s : spari)
            if (s.velocitaY > 0)
                s.morte = true;
        --g.granate;
    }

    void norteNemico(Nemico e)
    {
        punti += 10;

        int choice = rand() % 100;

        if (choice == 0)
            miglioramenti.push_back({e.x, e.y, true, false, false, false, false, false, false, (char)4, o.potenzaAttributo}); // Power '♦'
        else if (choice == 10)
            miglioramenti.push_back({e.x, e.y, false, true, false, false, false, false, false, (char)175, o.raggioFuocoAttributo}); // FireRate '»'
        else if (choice == 20)
            miglioramenti.push_back({e.x, e.y, false, false, true, false, false, false, false, (char)3, o.morteVitaAttributo}); // Life '♥'
        else if (choice == 30)
            miglioramenti.push_back({e.x, e.y, false, false, false, true, false, false, false, (char)3, o.vitaAttributo}); // Life '♥'
        else if (choice == 40)
            miglioramenti.push_back({e.x, e.y, false, false, false, false, true, false, false, (char)184, o.granataAttributo}); // Granade '©'
        else if (choice == 50)
            miglioramenti.push_back({e.x, e.y, false, false, false, false, false, true, false, (char)157, o.scudoAttributo}); // Shield 'Ø'
    }

    void applicaMiglioramenti(Miglioramento &u)
    {
        if (u.ePotenza && g.potenza < 5)
            ++g.potenza;
        if (u.eGranata && g.granate < 3)
            ++g.granate;
        if (u.eCura && g.vita < g.lifeMax)
            ++g.vita;
        if (u.eScudo)
            g.scudi = 500;
        if (u.eVita && g.lifeMax < 5)
            ++g.lifeMax;
        if (u.eRaggioFuoco && g.raggioFuoco > 10)
            g.raggioFuoco -= 2;

        u.morte = true;
    }

    void logicaGioco()
    {
        // Movimento e tiro del giocatore
        ++g.tempo;
        if (g.scudi > 0)
            --g.scudi;

        if (cornice % VELOCITA_GIOCATORE == 0)
        {
            if (IsKeyDown(KEY_A))
            {
                if (g.x > 1)
                    --g.x;
            }
            else if (IsKeyDown(KEY_D))
            {
                if (g.x < SCREEN_WIDTH - 2)
                    ++g.x;
            }
        }
        if (o.sparoContinuo | IsKeyDown(KEY_BACKSPACE))
            spariGiocatore();
        if (IsKeyDown(KEY_G))
            lancioGranata();

        // Spara movimento e collisione
        for (auto &s : spari)
        {
            ++s.tempo;

            // Spara movimento
            if (s.tempoVariabile <= s.tempo)
            {
                s.tempo -= s.tempoVariabile;
                s.y += s.velocitaY;
                s.x += s.velocitaX;
            }

            // Spara fuori dallo schermo
            if (s.x < 0 || s.y < 4 || s.x >= SCREEN_WIDTH || s.y >= SCREEN_HEIGHT)
            {
                s.morte = true;
                continue;
            }

            // Controlla la collisione del giocatore
            if (g.x == s.x && g.y == s.y)
            {
                if (g.scudi <= 0)
                {
                    g.vita -= s.danno;
                    g.scudi = 110;
                }

                s.morte = true;
            }

            // Controlla la collisione dei nemici
            if (s.velocitaY > 0)
                continue;
            for (auto &e : nemici)
                if (!e.morte && !s.morte && e.x == s.x && e.y == s.y)
                {
                    s.morte = true;
                    e.vita -= s.danno;
                }
        }

        // Il nemico spara
        for (auto &e : nemici)
        {
            if (e.vita <= 0)
            {
                e.morte = true;
                continue;
            }
            ++e.tempo;

            if (e.raggioFuoco <= e.tempo)
            {
                bool canShoot = true;
                for (auto &s1 : spari)
                    if (s1.x == e.x && s1.y == e.y + 1)
                    {
                        canShoot = false;
                        break;
                    }

                if (canShoot)
                {
                    e.tempo -= e.raggioFuoco;
                    spari.push_back({e.x, e.y + 1, e.potenza, 0, 1, 0, 10, false, 124, F_LIGHTYELLOW}); // '|'
                }
            }
        }

        // Aggiorna movimento e collisione
        for (auto &u : miglioramenti)
        {
            if (u.morte)
                continue;

            if (cornice % 15 == 0)
                ++u.y;
            if (g.x == u.x && g.y == u.y)
                applicaMiglioramenti(u);
            if (u.y >= SCREEN_HEIGHT)
                u.morte = true;
        }

        // Controlla la morte del giocatore
        if (g.vita <= 0)
            g.morte = true;

        // Distruggi i nemici morti
        for (int i = nemici.size() - 1; i >= 0; --i)
            if (nemici[i].morte)
            {
                norteNemico(nemici[i]);
                nemici.erase(nemici.begin() + i);
            }

        // Distruggi i germogli morti
        for (int i = spari.size() - 1; i >= 0; --i)
            if (spari[i].morte)
                spari.erase(spari.begin() + i);

        // Distruggi gli aggiornamenti morti
        for (int i = miglioramenti.size() - 1; i >= 0; --i)
            if (miglioramenti[i].morte)
                miglioramenti.erase(miglioramenti.begin() + i);
    }

    void impostaLivello()
    {
        g.tempo = 0;
        spari.clear();
        for (int i = 0; i < nemici_per_livello[livello - 1]; ++i)
        {
            Nemico enemy;
            enemy.x = 1 + i % (SCREEN_WIDTH - 2);
            enemy.y = 4 + i / (SCREEN_WIDTH - 2);
            enemy.vita = 1;
            enemy.potenza = 1;
            enemy.raggioFuoco = 1000 + rand() % 500;
            enemy.tempo = 500 + rand() % 500;
            enemy.morte = false;
            int v = rand() % 4;
            if (v == 0)
                enemy.c = 228; // 'õ'
            else if (v == 1)
                enemy.c = 224; // 'Ó'
            else if (v == 2)
                enemy.c = 226; // 'Ô'
            else if (v == 3)
                enemy.c = 227; // 'Ò'
            enemy.attr = F_RED;
            nemici.push_back(enemy);
        }
    }

    void riavvia()
    {
        livello = 1;
        punti = 0;
        cornice = 0;

        g.x = 12;
        g.y = 25;
        g.potenza = 1;
        g.raggioFuoco = 20;
        g.tempo = 0;
        g.vita = 3;
        g.lifeMax = 3;
        g.granate = 1;
        g.morte = false;
        g.scudi = 300;

        spari.clear();
        nemici.clear();
        miglioramenti.clear();
    }

    void loopCentrale()
    {
        while (attivo)
        {
            riavvia();

            // Menù
            bool menu = true;
            while (attivo && menu)
            {
                FillBuffer(' ', F_BLACK | o.sfondoAttributo);
                disegnaMenuAnimazioni();
                disegnaBenvenuto();
                DrawBuffer();
                if (IsKeyDown(KEY_ENTER))
                    menu = false;
                Sleep(10);
                ++cornice;
            }

            FillBuffer(' ', F_BLACK | o.sfondoAttributo);
            DrawBuffer();

            // FASE DI GIOCO
            bool play = true;
            while (attivo && play)
            {
                // DISEGNA NEMICI
                cornice = 0;
                impostaLivello();
                while (cornice < nemici_per_livello[livello - 1])
                {
                    FillBuffer(' ', F_BLACK | o.sfondoAttributo);
                    disegnaNemici(cornice);
                    disegnaGriglia();
                    DrawBuffer();
                    Sleep(10);
                    ++cornice;
                };

                // INIZIA A GIOCARE
                while (!g.morte && nemici.size() > 0)
                {
                    FillBuffer(' ', F_BLACK | o.sfondoAttributo);
                    disegnaSpari();
                    disegnaMiglioramenti();
                    disegnaSpari();
                    disegnaNemici(nemici.size());
                    disegnaGiocatore();
                    disegnaGriglia();
                    DrawBuffer();
                    logicaGioco();
                    Sleep(10);
                    ++cornice;
                }

                play = !g.morte;
                ++livello;
            }
        }
    }

    void fine()
    {
        FillBuffer(' ', F_BLACK | o.sfondoAttributo);
        DrawBuffer();
    }

private:
    int livello, punti;
    long cornice;
    bool attivo;
    Giocatore g;
    std::vector<Nemico> nemici;
    std::vector<Sparo> spari;
    std::vector<Miglioramento> miglioramenti;
    Opzioni o;
};

int main()
{
    Corpo c;
    c.run();
    return EXIT_SUCCESS;
}
