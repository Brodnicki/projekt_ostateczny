#pragma once
#include "Zlecenie.h"

#include <array>
#include <ostream>
#include <string>
#include <vector>

//Wszystkie policzone liczby w jednym miejscu
struct Statystyki {
	int liczbaZlecen = 0;
	double srednieOczekiwanie = 0.0;
	int maksOczekiwanie = 0;
	double odchylenie = 0.0;
	int p90 = 0;
	int p95 = 0;
	int p99 = 0;
	int wTerminie = 0;
	int spoznione = 0;
	double procentSpoznionych = 0.0;               // tylko wśród zleceń, które MAJĄ termin
	std::array<double, 6> sredniaWgPriorytetu{};   // [1]..[5]; pozycja [0] nieużywana
};

Statystyki policzStatystyki(const std::vector<Wynik>& wyniki);

//std::ostream& out to klucz do wymagania „ta sama tabela na ekran i do pliku”.
//std::ostream to wspólny „interfejs” dla wszystkiego, do czego można pisać przez <<:
//raz podajemy std::cout, raz std::ofstream z wyniki.txt.
void wypiszNaglowekTabeli(std::ostream& out);
void wypiszWierszTabeli(std::ostream& out, const std::string& strategia, const Statystyki& st,
	double wykorzystanie, double ms);
//Średnie oczekiwanie osobno dla priorytetów 1..5 (do analizy głodzenia)
void wypiszPriorytety(std::ostream& out, const std::string& strategia, const Statystyki& st);
