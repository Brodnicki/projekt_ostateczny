#include "Wykonawca.h"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <cmath>

Wykonawca::Wykonawca(std::string nazwa, std::vector<Specjalizacja> umiejetnosci, double tempo) : nazwa_(std::move(nazwa)), umiejetnosci_(std::move(umiejetnosci)), tempo_(tempo) {}

void Wykonawca::rozpocznij(const Zlecenie& z, int teraz) {
    if (!wolny()) {
        // Programowanie defensywne: taki błąd oznacza pomyłkę w symulatorze.
        throw std::logic_error(nazwa_ + " jest już zajęty");
    }
    biezace_ = z;
    startBiezacego_ = teraz;
}

Zlecenie Wykonawca::zakoncz(int teraz) {
    if (wolny()) {
        throw std::logic_error(nazwa_ + " nie ma czego kończyć");
    }
    Zlecenie z = *biezace_;
    biezace_.reset();  // znów wolny
    czasPracy_ += teraz - startBiezacego_;
    ++obsluzone_;
    return z;
}

//Czy na liście umiejętności jest s?
bool Wykonawca::umie(Specjalizacja s) const {
    return std::find(umiejetnosci_.begin(), umiejetnosci_.end(), s) != umiejetnosci_.end();
}

//Ile sekund ten pracownik będzie robił to zlecenie
int Wykonawca::czasObslugi(const Zlecenie& z) const {
    const int czas = static_cast<int>(std::lround(z.realizacja / tempo_));
    return std::max(1, czas); //Co najmniej 1 sekudna
}
