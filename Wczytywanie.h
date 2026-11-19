#pragma once
#include "Wykonawca.h"
#include "Zlecenie.h"

#include <string>
#include <vector>

//Wszystko co jest w konfiguracja.txt
//Struct w przeciwieństwie do funkcji może zwrócić 2 rzeczy
//a my akurat ich potrzebujemy tj. listy strategii i listy wykonawców
struct Konfiguracja {
	//Strategie do porównania, z linii "strategia FIFO HPF AGING".
	//Bez tej linii w pliku program sprawdza wszystkie trzy.
	std::vector<std::string> strategie = { "FIFO", "HPF", "AGING" };
	std::vector<Wykonawca> wykonawcy;
};
//nazwa pliku przekazywana bez prawa zmiany
std::vector<Zlecenie> wczytajZlecenia(const std::string& sciezka);
Konfiguracja wczytajKonfiguracje(const std::string& sciezka);