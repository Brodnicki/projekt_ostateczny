#include "Symulator.h"

#include <algorithm>
#include <iostream>
#include <utility>

//std::move konieczny do przekazania obiektu bo unique_ptr to jedyny właściciel.
Symulator::Symulator(std::vector<Zlecenie> zlecenia, std::vector<Wykonawca> wykonawcy, std::unique_ptr<IStrategia> strategia):
    zlecenia_(std::move(zlecenia)), wykonawcy_(std::move(wykonawcy)), rekordWykonawcy_(wykonawcy_.size(), -1) {
    for (auto& kolejka : kolejki_) {
        kolejka = strategia->nowaPusta();   // 4 × nowa pusta strategia tego samego rodzaju
    }
    // Zlecenia muszą być ułożone chronologicznie (przy remisie — po id).
    std::sort(zlecenia_.begin(), zlecenia_.end(), [](const Zlecenie& a, const Zlecenie& b) {
        return a.nadejscie != b.nadejscie ? a.nadejscie < b.nadejscie : a.id < b.id;
        });
}

void Symulator::zaplanuj(int czas, TypZdarzenia typ, int indeks) {
    kalendarz_.push(Zdarzenie{czas, typ, indeks, licznikZdarzen_++});
}

std::vector<Wynik> Symulator::uruchom() {
    if (!zlecenia_.empty()) {
        // Planujemy tylko PIERWSZE nadejście — każde kolejne zaplanuje poprzednie.
        zaplanuj(zlecenia_.front().nadejscie, TypZdarzenia::Nadejscie, 0);
    }

    while (!kalendarz_.empty()) {
        // ----- Faza A: przesuń zegar -----
        teraz_ = kalendarz_.top().czas;

        // ----- Faza B: wszystkie zdarzenia z chwilą teraz_ -----
        while (!kalendarz_.empty() && kalendarz_.top().czas == teraz_) {
            const Zdarzenie z = kalendarz_.top();  // kopia, bo pop() usunie element
            kalendarz_.pop();
            if (z.typ == TypZdarzenia::Nadejscie) {
                obsluzNadejscie(z.indeks);
            } else {
                obsluzZakonczenie(z.indeks);
            }
        }

        // ----- Faza C: przydział -----
        przydzielZlecenia();
    }
    return wyniki_;
}

void Symulator::obsluzNadejscie(int indeksZlecenia) {
    const Zlecenie& z = zlecenia_[static_cast<std::size_t>(indeksZlecenia)];
    if (dziennik_) {   // klamry: bez nich if działa tylko na jedną następną instrukcję
        std::cout << "[t=" << teraz_ << "] nadeszlo zlecenie #" << z.id << '\n';
    }
    //kolejka_.push(z) zmienione na  -> dodaj bo std::unique_ptr<IStrategia> nie ma składowej push
    kolejki_[indeks(z.specjalizacja)]->dodaj(z);

    // Samoplanujące się nadejścia: w kalendarzu jest zawsze tylko jedno przyszłe nadejście.
    const int nastepne = indeksZlecenia + 1;
    if (nastepne < static_cast<int>(zlecenia_.size())) {
        zaplanuj(zlecenia_[static_cast<std::size_t>(nastepne)].nadejscie, TypZdarzenia::Nadejscie, nastepne);
    }
}

void Symulator::obsluzZakonczenie(int indeksWykonawcy) {
    Wykonawca& w = wykonawcy_[static_cast<std::size_t>(indeksWykonawcy)];
    const Zlecenie z = w.zakoncz(teraz_);
    const int rekord = rekordWykonawcy_[static_cast<std::size_t>(indeksWykonawcy)];
    wyniki_[static_cast<std::size_t>(rekord)].koniec = teraz_;
    if (dziennik_) {
        std::cout << "[t=" << teraz_ << "] " << w.nazwa() << " konczy zlecenie #" << z.id << '\n';
    }
}

// Reguła wyboru: najpierw specjalista (mniej umiejętności), przy remisie najszybszy.
// Zwraca indeks wykonawcy albo -1, gdy nikt wolny tego nie umie.
int Symulator::wybierzWykonawce(Specjalizacja s) const {
    int najlepszy = -1;
    for (std::size_t i = 0; i < wykonawcy_.size(); ++i) {
        const Wykonawca& w = wykonawcy_[i];
        if (!w.wolny() || !w.umie(s)) {
            continue;                       // zajęty albo nie umie: pomijamy
        }
        if (najlepszy == -1) {
            najlepszy = static_cast<int>(i);  // pierwszy pasujący
            continue;
        }
        const Wykonawca& obecny = wykonawcy_[static_cast<std::size_t>(najlepszy)];
        if (w.liczbaUmiejetnosci() < obecny.liczbaUmiejetnosci() ||
            (w.liczbaUmiejetnosci() == obecny.liczbaUmiejetnosci() && w.tempo() > obecny.tempo())) {
            najlepszy = static_cast<int>(i);  // lepszy od dotychczasowego
        }
    }
    return najlepszy;
}

void Symulator::przydzielZlecenia() {
    for (int s = LICZBA_SPECJALIZACJI - 1; s >= 0; --s) {
        IStrategia& kolejka = *kolejki_[static_cast<std::size_t>(s)];
        while (!kolejka.pusta()) {
            const int i = wybierzWykonawce(static_cast<Specjalizacja>(s));
            if (i == -1) {
                break;                      // nikt wolny tego nie umie: następna kolejka
            }
            Wykonawca& w = wykonawcy_[static_cast<std::size_t>(i)];
            const Zlecenie z = kolejka.pobierz();
            w.rozpocznij(z, teraz_);

            Wynik wynik;
            wynik.idZlecenia = z.id;
            wynik.nadejscie = z.nadejscie;
            wynik.start = teraz_;
            wynik.wykonawca = i;
            wynik.termin = z.termin; //Potrzebne do sprawdzenia czy zamówienie skończyło się w terminie
            wynik.priorytet = z.priorytet;
            rekordWykonawcy_[static_cast<std::size_t>(i)] = static_cast<int>(wyniki_.size());
            wyniki_.push_back(wynik);

            zaplanuj(teraz_ + w.czasObslugi(z), TypZdarzenia::Zakonczenie, i);   // ZMIANA: czas z tempem
            if (dziennik_) {
                std::cout << "[t=" << teraz_ << "] " << w.nazwa() << " zaczyna zlecenie #" << z.id << " ("
                    << nazwa(z.specjalizacja) << ", czekalo " << wynik.oczekiwanie() << " s)\n";
            }
        }
    }
}

