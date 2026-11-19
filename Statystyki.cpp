#include "Statystyki.h"

#include <algorithm>	//std::sort, std::max_element, std::max 
#include <cmath>		//std:sqrt, std::ceil
#include <iomanip>		//std::setw, std::setprecision, std::fixed
#include <numeric>		//std::accumulate

namespace {
	//Percentyl p z POSORTOWANEJ lsity(metoda najbliższej rangi).
	int percentyl(const std::vector<int>& posortowane, int p) {
		const double n = static_cast<double>(posortowane.size());
		//posób liczenia: posortuj czasy rosnąco. Dla 200 zleceń p90 to 180.
		// wartość (90% z 200). std::ceil zaokrągla w górę, np. 3,6 daje 4.
		std::size_t ranga = static_cast<std::size_t>(std::ceil(p / 100.0 * n));
		if (ranga == 0) {
			ranga = 1;
		}
		return posortowane[ranga - 1]; //ranga - 1, bo „4. wartość” leży pod indeksem 3.
	}

} //namespace
//Najpierw średnia, potem odchylenie, bo odchylenie potrzebuje średniej.
// Sortowanie jest na końcu, bo potrzebują go tylko percentyle.
Statystyki policzStatystyki(const std::vector<Wynik>& wyniki) {
	Statystyki st;
	st.liczbaZlecen = static_cast<int>(wyniki.size());
	if (wyniki.empty()) {
		return st; //Nie dzielimy przez zero (cholero)
	}

	//1)zmieramy czas oczekiwania i przy okazji sprawdzamy terminy
	std::vector<int> czasy;
	std::array<double, 6> sumaWgPriorytetu{};
	std::array<int, 6> ileWgPriorytetu{};
	for (const Wynik& w : wyniki) {
		czasy.push_back(w.oczekiwanie());
		const std::size_t p = static_cast<std::size_t>(w.priorytet);
		sumaWgPriorytetu[p] += w.oczekiwanie();
		++ileWgPriorytetu[p];
		if (w.termin > 0) { //0 = zlecenie bez terminu
			if (w.koniec <= w.termin) {
				++st.wTerminie;
			} else {
				++st.spoznione;
			}
		}
	}

	//2) Srednia i maksimum
	const double n = static_cast<double>(czasy.size());
	const double suma = std::accumulate(czasy.begin(), czasy.end(), 0.0);
	st.srednieOczekiwanie = suma / n;
	//max_element zwraca pozycje największego elementu, a nie wartość
	//* oznacza weź wartośc z tej pozycji, tak jak przy wskaźnikach
	st.maksOczekiwanie = *std::max_element(czasy.begin(), czasy.end());

	//3)Odchylenie standardowe
	double sumaKwadratow = 0.0;
	//for (int c : czasy) to „dla każdego c z listy czasy”.
	for (int c : czasy) {
		const double roznica = c - st.srednieOczekiwanie;
		sumaKwadratow += roznica * roznica;
	}
	st.odchylenie = std::sqrt(sumaKwadratow / n);
	for (std::size_t p = 1; p <= 5; ++p) {
		if (ileWgPriorytetu[p] > 0) {
			st.sredniaWgPriorytetu[p] = sumaWgPriorytetu[p] / ileWgPriorytetu[p];
		}
	}

	//4) Percentyle (wymagają posortowania)
	std::sort(czasy.begin(), czasy.end());
	st.p90 = percentyl(czasy, 90);
	st.p95 = percentyl(czasy, 95);
	st.p99 = percentyl(czasy, 99);

	//5) Procent spóźnionych liczymy tylko wśród zleceń, które MAJĄ termin
	const int zTerminem = st.wTerminie + st.spoznione;
	if (zTerminem > 0) {
		st.procentSpoznionych = 100.0 * st.spoznione / zTerminem;
	}
	return st;
}

//std::setw(n) = następna wartość zajmie n znaków, dzięki temu kolumny są równe
void wypiszNaglowekTabeli(std::ostream& out) {
	out << "Strategia   srednie[s]  maks[s]  odchylenie[s]  p90[s]  p95[s]  p99[s]  spoznione[%]  wykorzystanie[%]  czas[ms]\n";
}

void wypiszWierszTabeli(std::ostream& out, const std::string& strategia, const Statystyki& st,
	double wykorzystanie, double ms) {
	out << std::fixed << std::setprecision(1);
	out << std::left << std::setw(9) << strategia << std::right << std::setw(13) << st.srednieOczekiwanie
		<< std::setw(9) << st.maksOczekiwanie << std::setw(15) << st.odchylenie << std::setw(8) << st.p90
		<< std::setw(8) << st.p95 << std::setw(8) << st.p99 << std::setw(14) << st.procentSpoznionych
		<< std::setw(18) << wykorzystanie << std::setw(10) << ms << '\n';
}

void wypiszPriorytety(std::ostream& out, const std::string& strategia, const Statystyki& st) {
	out << std::fixed << std::setprecision(1);
	out << "  " << std::left << std::setw(9) << strategia << std::right;
	for (std::size_t p = 1; p <= 5; ++p) {
		out << std::setw(11) << st.sredniaWgPriorytetu[p];
	}
	out << '\n';
}
