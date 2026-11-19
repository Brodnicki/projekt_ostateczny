#pragma once
#include "IStrategia.h"
#include <queue>
#include <vector>

//Dla algorytmu HPF najważniejszy jest priorytet zlecenia

//Reguła porównywania dla prioryty_qeque
//Zwraca true, gdy zlecenie z jest gorsze od b, czyli ma wyjść później.
struct KomparatorPriorytetu {
	bool operator()(const Zlecenie& a, const Zlecenie& b) const {
		if (a.priorytet != b.priorytet) {
			return a.priorytet < b.priorytet; //niższy priorytet = gorszy
		}
		if (a.nadejscie != b.nadejscie) {
			return a.nadejscie > b.nadejscie; // Kiedy pirorytet jest taki sam to zleceniem gorszym jest te które przybyło później
		}
		return a.id > b.id; //Jeśli wcześniejsze porównania nie rozstrzygają to id to zrobi, bo tam nie może być remisu
	}
};

class StrategiaHpf : public IStrategia {
public:
	//Kopiec sam ustawia zlecenia na właściwym miejscu

	void dodaj(const Zlecenie& z) override { kopiec_.push(z); }

	bool pusta() const override { return kopiec_.empty(); }

	//Na szczycie kopca (top) zawsze leży najważniejsze zlecenie.
	Zlecenie pobierz() override {
		Zlecenie z = kopiec_.top();
		kopiec_.pop();
		return z;
	}
	std::unique_ptr<IStrategia> nowaPusta() const override { return std::make_unique<StrategiaHpf>(); }
private:
	std::priority_queue<Zlecenie, std::vector<Zlecenie>, KomparatorPriorytetu> kopiec_;
};