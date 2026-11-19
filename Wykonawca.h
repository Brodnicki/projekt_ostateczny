#pragma once
// =============================================================================
//  Wykonawca.h — pracownik obsługujący zlecenia
// =============================================================================
//  Pracownik NIE "pracuje" w pętli. To tylko kilka zmiennych:
//    • bieżące zlecenie (puste = wolny, zawiera coś = zajęty),
//    • licznik czasu pracy i obsłużonych zleceń.
//  Kiedy praca się skończy, wie symulator — zapisuje to w kalendarzu zdarzeń.
// =============================================================================

#include "Zlecenie.h"
#include "Specjalizacja.h"
#include <optional>
#include <string>
#include <vector>

class Wykonawca {
public:
    //std::vector<Specjalizacja> umiejetnosci_ to lista specjalizacji jaką pracownik potrafi
    //może zawierać jeden element, albo wiele w zależności od tego ile pracownik potrafi.
    Wykonawca(std::string nazwa, std::vector<Specjalizacja> umiejetnosci, double tempo = 1.0);

    const std::string& nazwa() const { return nazwa_; }
    bool wolny() const { return !biezace_.has_value(); }

    bool umie(Specjalizacja s) const;
    int czasObslugi(const Zlecenie& z) const;
    int liczbaUmiejetnosci() const { return static_cast<int>(umiejetnosci_.size()); } //Podglądanie do wybrania najlepszego wykonawcy, size zwrcaca długość listy.
    double tempo() const { return tempo_; } //Podglądanie do wybrania najlepszego wykonawcy



    // WOLNY -> ZAJĘTY
    void rozpocznij(const Zlecenie& z, int teraz);
    // ZAJĘTY -> WOLNY; zwraca ukończone zlecenie
    Zlecenie zakoncz(int teraz);

    int czasPracy() const { return czasPracy_; }
    int obsluzone() const { return obsluzone_; }

private:
    std::string nazwa_;
    std::vector<Specjalizacja> umiejetnosci_;
    double tempo_ = 1.0; //Tempo pracy pracownika (domyślnie w konstruktorze wynosi 1)
    std::optional<Zlecenie> biezace_;  // std::optional = "zlecenie albo nic"
    int startBiezacego_ = 0;
    int czasPracy_ = 0;
    int obsluzone_ = 0;
};
