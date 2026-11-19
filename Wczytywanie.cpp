#include "Wczytywanie.h"

#include <charconv>   //std::from_chars
#include <filesystem> //std::filesystem::current_path
#include <fstream>    //std::ifstream
#include <iostream>   //std::cerr
#include <sstream>    //std::istringstream
#include <stdexcept>  //std::runtime_error

//Schowek na funkcje pomocnicze, dzięki zastosowaniu namespace
//funckje pisane w tym pliku nie będą zderzać się z funkcjami z innych plików
namespace { //wszystko do zamknięcia jest widoczne TYLKO w tym pliku

	//Usuwa '\r' i komentarz, a reszte lini dzieli na słowa
	//Każda linia kończy się dwoma niewidocznymi znakami, \r\n
	std::vector<std::string> podzielLinie(std::string linia) {
		if (!linia.empty() && linia.back() == '\r') { //Linia back to ostatni znak
			linia.pop_back();		              //pułapka z Windowsa, pop_back usuwa ostatni znak
		}
		//find(#) szuka pozycji tego znaku. substr(0, n) bieerze pierwsze n znaków czyli wszystko przed #.
		//gdy nie ma # find zwraca specjalną wartość nie znaleziono (npos), a substr bierze wtedy całą linie
		//Jedna linia obsługuje wywalanie komentarzy i 
		linia = linia.substr(0, linia.find('#')); //Obcinamy od '#' do końca

		std::istringstream strumien(linia); //Pozwala czytać ze zwykłego testu tak jak cin z klawiatury
		std::vector<std::string> pola;
		std::string slowo;
		//Operator >> wyciaga słowo pomijając spacje i tabulatory
		while (strumien >> slowo) { //Wykonuje się dopóki uda się wyciągnąc słowo
			pola.push_back(slowo); 
		}
		return pola;
	}


	//true = cały tekst jest poprawną liczbą całkowitą
	bool naLiczbe(const std::string& tekst, int& wynik) {
		const char* poczatek = tekst.data();
		const char* koniec = tekst.data() + tekst.size();
		const std::from_chars_result r = std::from_chars(poczatek, koniec, wynik);
		return r.ec == std::errc() && r.ptr == koniec;
	}

	//To samo dla liczb z kropką (tempo)
	bool naLiczbe(const std::string& tekst, double& wynik) {
		const char* poczatek = tekst.data();
		//char* wskaźnik na znak, a tekst.data() to adres pierwszej litery, a + tekst.size() przesuwa go na ostatnią literę
		const char* koniec = tekst.data() + tekst.size();
		//Czyta liczbę z tekstu i zwraca dwie informacje
		//r.ec kod błędu (std::errc() oznacza brak błędu
		//r.ptr czyli miejsce gdzie czytanie zatrzymało się
		//from_chars nie rzuca wyjątków, a do tego jest szybkie.
		const std::from_chars_result r = std::from_chars(poczatek, koniec, wynik);
		return r.ec == std::errc() && r.ptr == koniec;
	}

	//PAKOWANIE -> Specjalizacja::Pakowanie
	//Sprawdza po kolei wszystkie 4 specjalizacje, zamienia numer na enum
	bool naSpecjalizacje(const std::string& tekst, Specjalizacja& wynik) {
		for (int i = 0; i < LICZBA_SPECJALIZACJI; ++i) {
			const Specjalizacja s = static_cast<Specjalizacja>(i);
			if (tekst == nazwa(s)) {
				wynik = s;
				return true;
			}
		}
		return false;
	}

	std::ifstream otworz(const std::string& sciezka) {
		std::ifstream plik(sciezka); //Otwiera plik do czytania
		if (!plik) { //Nie udało się otworzyć pliku więc false
			//Wyjątek
			throw std::runtime_error("Nie mogę otworzyc pliku " + sciezka + " (szukam w folderze " + std::filesystem::current_path().string() + ")");
		}
		return plik;
	}

	void ostrzez(const std::string& plik, int nrLinii, const std::string& opis) {
	//std:cerr działa jak std::cout ale przeznaczony jest na komunikaty o błędach
	std::cerr << "UWAGA: " << plik << ", linia " << nrLinii << ": " << opis << " - pomijam tę linię\n";
	}

} //koniec namespace

std::vector<Zlecenie> wczytajZlecenia(const std::string& sciezka) {
	std::ifstream plik = otworz(sciezka);
	std::vector<Zlecenie> zlecenia;
	std::string linia;
	int nrLinii = 0;

	//Wczytuje linie i sprawdza na wszystkie możliwe sposoby żeby nie było błędu
	//Przechodza jedynie te w 100% poprawne
	while (std::getline(plik, linia)) {
		++nrLinii; //Liczy wszystkie linie by ewentualny kod błędu zgadzał się z tym co w natatniku
		const std::vector<std::string> pola = podzielLinie(linia);
		if (pola.empty()) {
			continue; //pusta linia albo komentarz
		}
		if (pola.size() != 6) {
			//To string zmienia liczbe na tekst by można było dokleić + do komunikatu
			ostrzez(sciezka, nrLinii, "oczekiwano 6 pol, jest " + std::to_string(pola.size()));
			continue;
		}
		//użyto || bo sprawdza warunki od lewej, tj. jeden z lewej zły i wywalamy
		Zlecenie z;
		if (!naLiczbe(pola[0], z.id) || !naLiczbe(pola[1], z.nadejscie) || !naLiczbe(pola[2], z.realizacja)
			|| !naLiczbe(pola[3], z.priorytet) || !naLiczbe(pola[4], z.termin)) {
			ostrzez(sciezka, nrLinii, "jedno z pol 1-5 nie jest liczba calkowita");
			continue;
		}
		if (!naSpecjalizacje(pola[5], z.specjalizacja)) {
			ostrzez(sciezka, nrLinii, "nieznana specjalizacja '" + pola[5] + "'");
			continue;
		}
		if (z.nadejscie < 0 || z.realizacja <= 0 || z.termin < 0) {
			ostrzez(sciezka, nrLinii, "czas nie może być ujemny (a realizacja musi być > 0)");
			continue;
		}
		if(z.priorytet < 1 || z.priorytet > 5) {
			ostrzez(sciezka, nrLinii, "priorytet musi być od 1 do 5");
			continue;
		}
		zlecenia.push_back(z); //Linia przeszła wszystkie kontrole
	}
	return zlecenia;
}

Konfiguracja wczytajKonfiguracje(const std::string& sciezka) {
	std::ifstream plik = otworz(sciezka);
	Konfiguracja k;
	std::string linia;
	int nrLinii = 0;

	while (std::getline(plik, linia)) {
		++nrLinii;
		const std::vector<std::string> pola = podzielLinie(linia);
		if (pola.empty()) {
			continue;
		}
		//Sprawdzanie warunków
		if (pola[0] == "strategia" && pola.size() >= 2) {
			//Wszystkie słowa po "strategia" to nazwy strategii, np. "strategia FIFO HPF AGING"
			k.strategie.assign(pola.begin() + 1, pola.end());
		}
		else if (pola[0] == "wykonawca" && pola.size() >= 4) {
			double tempo = 1.0;
			if (!naLiczbe(pola[2], tempo) || tempo <= 0) {
			ostrzez(sciezka, nrLinii, "tempo musi byc liczba wieksza od 0");
			continue;
			}
			std::vector<Specjalizacja> umiejetnosci;
			bool wszystkoDobrze = true; //Flaga, jak coś jest źle to mówi żeby nie dodawać wykonawcy
			for (std::size_t i = 3; i < pola.size(); ++i) {// od 4. słowa do końca, czyli zbiera wszystkie pozostałe słowa jako umiejetności
				Specjalizacja s = Specjalizacja::Kompletacja;
				if (!naSpecjalizacje(pola[i], s)) {
					ostrzez(sciezka, nrLinii, "nieznana specjalizacja '" + pola[i] + "'");
					wszystkoDobrze = false;
					break;
				}
				umiejetnosci.push_back(s);
			}
			if (wszystkoDobrze) {
				k.wykonawcy.push_back(Wykonawca(pola[1], umiejetnosci, tempo));
			}
		}
		else {
			ostrzez(sciezka, nrLinii, "Nieznane polecenie albo niepoprawna liczba pol");
		}
	}

	if (k.wykonawcy.empty()) { //Jak nikt nie przyszedł do pracy, to nie ma komu pracować
		throw std::runtime_error("W pliku " + sciezka + " nie ma zadnego poprawnego wykonawcy");
	}
	return k;
}