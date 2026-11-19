#pragma once
// =============================================================================
//  Generator.h — losowe, ale POWTARZALNE zlecenia
// =============================================================================
//  To samo ziarno (seed) = te same zlecenia = ten sam plik. Dzięki temu każdą
//  strategię można uruchomić na dokładnie tych samych danych.
//  Czas liczymy w sekundach, tak jak w całym programie.
// =============================================================================
#include "Zlecenie.h"

#include <string>   // std::string
#include <vector>   // std::vector

// Wszystkie ustawienia jednego pliku w jednym miejscu.
struct ParametryGeneratora {
    int liczbaZlecen = 12000;          // N > 10000 (z zadania: więcej niż 10000)
    double zlecenNaGodzine = 50.0;     // natężenie napływu
    double sredniaRealizacja = 300.0;  // średni czas obsługi [s] = 5 minut
    unsigned long long ziarno = 2026;  // seed: to samo ziarno = te same zlecenia
};

// Losuje zlecenia i zwraca je jako listę (niczego nie zapisuje).
std::vector<Zlecenie> generujZlecenia(const ParametryGeneratora& p);

// Zapisuje listę w formacie pliku zgłoszeń, żeby wczytajZlecenia() mogło ją przeczytać.
void zapiszZlecenia(const std::string& sciezka, const std::vector<Zlecenie>& zlecenia, const ParametryGeneratora& p);
