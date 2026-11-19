#include "Generator.h"

#include <algorithm>   // std::max
#include <cmath>       // std::lround
#include <fstream>     // std::ofstream
#include <random>      // std::mt19937_64, std::exponential_distribution, std::discrete_distribution,
                       // std::uniform_real_distribution
#include <stdexcept>   // std::runtime_error

namespace {
    int zaokraglij(double x) { return static_cast<int>(std::lround(x)); }
}

std::vector<Zlecenie> generujZlecenia(const ParametryGeneratora& p) {
    // GENERATOR — tworzony RAZ. Wszystkie rozkłady niżej losują z tego samego "gen".
    std::mt19937_64 gen(p.ziarno);

    // ROZKŁADY — "przepisy", jak zamienić surową losową liczbę na wartość o danym kształcie.
    std::exponential_distribution<double> odstep(p.zlecenNaGodzine / 3600.0);    // odstępy między nadejściami
    std::exponential_distribution<double> obsluga(1.0 / p.sredniaRealizacja);    // czas realizacji
    std::discrete_distribution<int> priorytet({ 0, 30, 30, 20, 12, 8 });           // wagi priorytetów 0..5
    std::discrete_distribution<int> specjalizacja({ 50, 30, 10, 10 });             // KOMPL, PAK, WOZEK, CHLODNIA
    std::uniform_real_distribution<double> zapas(600.0, 3600.0);                 // zapas do terminu: 10–60 min

    std::vector<Zlecenie> zlecenia;
    zlecenia.reserve(static_cast<std::size_t>(p.liczbaZlecen));   // od razu miejsce na wszystkie

    double czas = 0.0;   // zegar w double, żeby zaokrąglenia się nie sumowały
    for (int id = 1; id <= p.liczbaZlecen; ++id) {
        czas += odstep(gen);
        Zlecenie z;
        z.id = id;
        z.nadejscie = zaokraglij(czas);
        z.realizacja = std::max(1, zaokraglij(obsluga(gen)));
        z.priorytet = priorytet(gen);
        z.termin = z.nadejscie + z.realizacja + zaokraglij(zapas(gen));
        z.specjalizacja = static_cast<Specjalizacja>(specjalizacja(gen));
        zlecenia.push_back(z);
    }
    return zlecenia;
}

void zapiszZlecenia(const std::string& sciezka, const std::vector<Zlecenie>& zlecenia, const ParametryGeneratora& p) {
    std::ofstream plik(sciezka);
    if (!plik) {
        throw std::runtime_error("Nie mogę utworzyć pliku " + sciezka);
    }
    plik << "# Wygenerowano: N=" << p.liczbaZlecen << ", zlecen/h=" << p.zlecenNaGodzine
        << ", srednia realizacja=" << p.sredniaRealizacja << " s, ziarno=" << p.ziarno << '\n';
    plik << "# id nadejscie realizacja priorytet termin specjalizacja\n";
    for (const Zlecenie& z : zlecenia) {
        plik << z.id << ' ' << z.nadejscie << ' ' << z.realizacja << ' ' << z.priorytet << ' ' << z.termin << ' '
            << nazwa(z.specjalizacja) << '\n';
    }
}
