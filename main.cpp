// =============================================================================
//  main.cpp — symulator magazynu: porównanie strategii FIFO / HPF / AGING
// =============================================================================
//  Co robi program (jedno uruchomienie = komplet wyników):
//    1) wczytuje konfiguracja.txt: załogę i strategie do porównania,
//    2) generator tworzy 3 pliki zgłoszeń po 12 000 zleceń (ziarno 2026):
//         zgloszenia_male.txt, zgloszenia_rownowaga.txt, zgloszenia_przeciazenie.txt,
//    3) każdy plik jest wczytywany i przechodzi przez każdą strategię
//       -> tabele w wyniki.txt i na ekranie, te same liczby w wyniki.csv,
//    4) seria natężeń od 15 do 65 zleceń na godzinę (zlecenia tylko w pamięci)
//       -> średnie oczekiwanie w funkcji obciążenia: wyniki.txt i obciazenie.csv.
//  Czas w całym programie liczymy w sekundach.
// =============================================================================
#include "FabrykaStrategii.h"
#include "Generator.h"
#include "Statystyki.h"
#include "Symulator.h"
#include "Wczytywanie.h"

#include <algorithm>   // std::max
#include <chrono>      // std::chrono::steady_clock, std::chrono::duration
#include <filesystem>  // std::filesystem::absolute
#include <fstream>     // std::ofstream
#include <iomanip>     // std::setw, std::setprecision, std::fixed
#include <iostream>    // std::cout, std::cerr
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <vector>      // std::vector

namespace {

    const int OKRES_STARZENIA = 1800;   // AGING: +1 do priorytetu co 30 min czekania

    // Jeden scenariusz = nazwa, plik i natężenie napływu. Reszta parametrów generatora jest wspólna.
    struct Scenariusz {
        std::string nazwa;
        std::string plik;
        int zlecenNaGodzine;   // ile zleceń średnio wpada w ciągu godziny
    };

    // Trzy zestawy różnią się TYLKO liczbą zleceń na godzinę (to samo ziarno, ta sama załoga).
    const std::vector<Scenariusz> SCENARIUSZE = {
        { "male",         "zgloszenia_male.txt",         25 },
        { "rownowaga",    "zgloszenia_rownowaga.txt",    45 },
        { "przeciazenie", "zgloszenia_przeciazenie.txt", 60 },
    };

    // Wynik jednego przebiegu: statystyki + to, czego nie da się policzyć z samych wyników
    struct Pomiar {
        Statystyki st;
        double wykorzystanie = 0.0;   // średnie wykorzystanie wykonawców [%]
        double ms = 0.0;              // czas działania symulacji [ms]
    };

    // Jedna symulacja: te same zlecenia, ta sama załoga, wybrana strategia
    Pomiar symuluj(const std::vector<Zlecenie>& zlecenia, const std::vector<Wykonawca>& wykonawcy,
        const std::string& strategia) {
        const auto start = std::chrono::steady_clock::now();   // stoper, który nigdy się nie cofa

        Symulator symulator(zlecenia, wykonawcy, utworzStrategie(strategia, OKRES_STARZENIA));
        const std::vector<Wynik> wyniki = symulator.uruchom();

        const auto koniec = std::chrono::steady_clock::now();

        Pomiar pomiar;
        pomiar.ms = std::chrono::duration<double, std::milli>(koniec - start).count();
        pomiar.st = policzStatystyki(wyniki);

        // Średnie wykorzystanie = łączny czas pracy / (liczba wykonawców × czas symulacji)
        double sumaPracy = 0.0;
        for (const Wykonawca& w : symulator.wykonawcy()) {
            sumaPracy += w.czasPracy();
        }
        const double czasSymulacji = static_cast<double>(std::max(1, symulator.czasKonca()));
        pomiar.wykorzystanie = 100.0 * sumaPracy / (static_cast<double>(wykonawcy.size()) * czasSymulacji);
        return pomiar;
    }

    // Tabela jednego scenariusza. Dostaje std::ostream&, więc raz piszemy nią na ekran (std::cout),
    // a raz do pliku wyniki.txt - ta sama funkcja, te same liczby.
    void wypiszScenariusz(std::ostream& out, const Scenariusz& sc, std::size_t liczbaZlecen,
        const std::vector<std::string>& strategie, const std::vector<Pomiar>& pomiary) {
        out << "\n--- " << sc.nazwa << ": " << sc.zlecenNaGodzine << " zlecen/h, N = " << liczbaZlecen
            << " (" << sc.plik << ") ---\n";
        wypiszNaglowekTabeli(out);
        for (std::size_t i = 0; i < pomiary.size(); ++i) {
            wypiszWierszTabeli(out, strategie[i], pomiary[i].st, pomiary[i].wykorzystanie, pomiary[i].ms);
        }
        out << "Srednie oczekiwanie wg priorytetu [s]:   prio1      prio2      prio3      prio4      prio5\n";
        for (std::size_t i = 0; i < pomiary.size(); ++i) {
            wypiszPriorytety(out, strategie[i], pomiary[i].st);
        }
    }

    // Krok 3: każdy plik zgłoszeń przez każdą strategię
    void porownajStrategie(const Konfiguracja& konfiguracja, std::ofstream& raport) {
        std::ofstream csv("wyniki.csv");
        if (!csv) {
            throw std::runtime_error("Nie mogę utworzyć pliku wyniki.csv");
        }
        // Nagłówek CSV: nazwy kolumn oddzielone przecinkami
        csv << "scenariusz,strategia,zlecenia,srednie_s,maks_s,odchylenie_s,p90_s,p95_s,p99_s,"
            "spoznione_proc,sr_prio1_s,sr_prio2_s,sr_prio3_s,sr_prio4_s,sr_prio5_s,wykorzystanie_proc,czas_ms\n";

        for (const Scenariusz& sc : SCENARIUSZE) {
            // Wczytujemy RAZ na scenariusz - każda strategia dostaje dokładnie te same dane
            const std::vector<Zlecenie> zlecenia = wczytajZlecenia(sc.plik);

            std::vector<Pomiar> pomiary;   // zapamiętujemy, żeby niżej wypisać tabelę priorytetów
            for (const std::string& strategia : konfiguracja.strategie) {
                pomiary.push_back(symuluj(zlecenia, konfiguracja.wykonawcy, strategia));
            }

            // Ta sama tabela raz na ekran, raz do wyniki.txt
            wypiszScenariusz(std::cout, sc, zlecenia.size(), konfiguracja.strategie, pomiary);
            wypiszScenariusz(raport, sc, zlecenia.size(), konfiguracja.strategie, pomiary);

            // Jeden wiersz CSV = jedna para (scenariusz, strategia)
            for (std::size_t i = 0; i < pomiary.size(); ++i) {
                const Statystyki& st = pomiary[i].st;
                csv << sc.nazwa << ',' << konfiguracja.strategie[i] << ',' << st.liczbaZlecen << ','
                    << st.srednieOczekiwanie << ',' << st.maksOczekiwanie << ',' << st.odchylenie << ',' << st.p90
                    << ',' << st.p95 << ',' << st.p99 << ',' << st.procentSpoznionych;
                for (std::size_t p = 1; p <= 5; ++p) {
                    csv << ',' << st.sredniaWgPriorytetu[p];
                }
                csv << ',' << pomiary[i].wykorzystanie << ',' << pomiary[i].ms << '\n';
            }
        }
    }

    // Krok 4: średnie oczekiwanie w funkcji obciążenia (do wykresu na ocenę 4.0).
    // Zlecenia generujemy tylko w pamięci, bez zapisywania 11 kolejnych plików.
    void seriaObciazenia(const Konfiguracja& konfiguracja, std::ofstream& raport) {
        std::ofstream csv("obciazenie.csv");
        if (!csv) {
            throw std::runtime_error("Nie mogę utworzyć pliku obciazenie.csv");
        }
        csv << "zlecen_na_godzine,wykorzystanie_proc";
        for (const std::string& strategia : konfiguracja.strategie) {
            csv << ',' << strategia << "_srednie_s";
        }
        csv << '\n';

        raport << std::fixed << std::setprecision(1);
        raport << "\n--- Srednie oczekiwanie [s] w funkcji obciazenia ---\n";
        raport << "zlecen/h  wykorzystanie[%]";
        for (const std::string& strategia : konfiguracja.strategie) {
            raport << std::setw(12) << strategia;
        }
        raport << '\n';

        for (int naGodzine = 15; naGodzine <= 65; naGodzine += 5) {
            ParametryGeneratora p;
            p.zlecenNaGodzine = naGodzine;
            const std::vector<Zlecenie> zlecenia = generujZlecenia(p);

            std::vector<Pomiar> pomiary;
            for (const std::string& strategia : konfiguracja.strategie) {
                pomiary.push_back(symuluj(zlecenia, konfiguracja.wykonawcy, strategia));
            }
            // Wykorzystanie prawie nie zależy od strategii, więc podajemy to z pierwszej
            csv << naGodzine << ',' << pomiary.front().wykorzystanie;
            raport << std::setw(8) << naGodzine << std::setw(18) << pomiary.front().wykorzystanie;
            for (const Pomiar& pm : pomiary) {
                csv << ',' << pm.st.srednieOczekiwanie;
                raport << std::setw(12) << pm.st.srednieOczekiwanie;
            }
            csv << '\n';
            raport << '\n';
        }
        std::cout << "\nZapisano serie obciazenia (15-65 zlecen/h) do wyniki.txt i obciazenie.csv\n";
    }

} // namespace

int main() {
    try {
        // 1) Załoga i strategie - jeden plik dla całego programu
        const Konfiguracja konfiguracja = wczytajKonfiguracje("konfiguracja.txt");
        for (const std::string& strategia : konfiguracja.strategie) {
            utworzStrategie(strategia);   // literówka w nazwie -> wyjątek od razu, a nie w połowie liczenia
        }

        // 2) Generator: 3 pliki zgłoszeń (zawsze te same, bo ziarno jest stałe)
        for (const Scenariusz& sc : SCENARIUSZE) {
            ParametryGeneratora p;               // wszystko domyślne...
            p.zlecenNaGodzine = sc.zlecenNaGodzine;   // ...zmieniamy tylko natężenie
            zapiszZlecenia(sc.plik, generujZlecenia(p), p);
            std::cout << "Zapisano " << p.liczbaZlecen << " zlecen do " << sc.plik << '\n';
        }

        std::ofstream raport("wyniki.txt");
        if (!raport) {
            throw std::runtime_error("Nie mogę utworzyć pliku wyniki.txt");
        }
        raport << "SYMULATOR MAGAZYNU - porownanie strategii\n";
        raport << "Zaloga z konfiguracja.txt: " << konfiguracja.wykonawcy.size() << " wykonawcow, okres AGING = "
            << OKRES_STARZENIA << " s\n";

        // 3) i 4) Porównanie strategii i seria obciążenia
        porownajStrategie(konfiguracja, raport);
        seriaObciazenia(konfiguracja, raport);

        std::cout << "Wyniki zapisano do: " << std::filesystem::absolute("wyniki.txt").string()
            << " (+ wyniki.csv, obciazenie.csv do wykresow)\n";
    }
    catch (const std::exception& e) {
        std::cerr << "BLAD: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
