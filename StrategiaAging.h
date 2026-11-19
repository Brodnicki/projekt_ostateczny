#pragma once
#include "IStrategia.h"

#include <queue>    // std::priority_queue
#include <vector>   // std::vector (pojemnik wewnątrz kopca)

// Klucz = nadejscie - priorytet * okres. Mniejszy klucz = wcześniej obsłużony.
// Każde "okres" sekund czekania jest warte tyle, co +1 do priorytetu.
struct KomparatorAging {
    int okres;
    bool operator()(const Zlecenie& a, const Zlecenie& b) const {
        const int kluczA = a.nadejscie - a.priorytet * okres;
        const int kluczB = b.nadejscie - b.priorytet * okres;
        if (kluczA != kluczB) {
            return kluczA > kluczB;   // większy klucz = gorszy
        }
        return a.id > b.id;
    }
};

class StrategiaAging : public IStrategia {
public:
    explicit StrategiaAging(int okres) : okres_(okres), kopiec_(KomparatorAging{ okres }) {}

    void dodaj(const Zlecenie& z) override { kopiec_.push(z); }
    bool pusta() const override { return kopiec_.empty(); }
    Zlecenie pobierz() override {
        Zlecenie z = kopiec_.top();
        kopiec_.pop();
        return z;
    }
    std::unique_ptr<IStrategia> nowaPusta() const override { return std::make_unique<StrategiaAging>(okres_); }

private:
    int okres_;   // zapamiętany, żeby nowaPusta() mogła zrobić kopię z tym samym okresem
    std::priority_queue<Zlecenie, std::vector<Zlecenie>, KomparatorAging> kopiec_;
};