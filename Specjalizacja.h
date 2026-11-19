#pragma once
#include <string>

//Lista możliwych specjalizacji. "enum class" = zamknięty zestaw nazwanych wartości
enum class Specjalizacja { Kompletacja, Pakowanie, Wozek, Brygadzista };

constexpr int LICZBA_SPECJALIZACJI = 4;

//Zmienia specjalizację na numer 0..3, żeby móc wybrać kolejkę z tablicy.
//Bez inline zgłoszono by błąd że funkcja zdefiniowana kilka razy
inline int indeks(Specjalizacja s) { return static_cast<int>(s); }

//Do wypisania na ekran
inline std::string nazwa(Specjalizacja s) {
	switch (s) {
		case Specjalizacja::Kompletacja: return "KOMPLETACJA";
		case Specjalizacja::Pakowanie: return "PAKOWANIE";
		case Specjalizacja::Wozek: return "WOZEK";
		case Specjalizacja::Brygadzista: return "BRYGADZISTA";
	}
	return "?";
}