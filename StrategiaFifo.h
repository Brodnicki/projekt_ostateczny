#pragma once
#include "IStrategia.h"
#include <queue>
//Fifo polega na tym że kto pierwszy przychodzi jest lepszy.
//Każda z strategii będzie dziedziczyć po interfejsie strategii
//Niech żyje podejście obiektowe dla wyższej oceny!!!
class StrategiaFifo : public IStrategia {
public:
	//Nowe zlecenie będzie na końcu kolejki
	void dodaj(const Zlecenie& z) override {
		kolejka_.push(z);
	}

	//Sprawdzanie czy ktoś czeka
	bool pusta() const override { return kolejka_.empty(); }


	//Zabieranie najstarszych zleceń
	//front() pobiera ostatnie zlecenie i zapisuje ją w zmiennej z typu Zlecenie
	//pop() usuwa ostatnie zlecenie które właśnie pobraliśmy żeby nam nie przeszkadzało
	//Zlecenie zapisane pod zmienną typu Zlecenie z zostaje zwrócone (dlatego typ metody to zlecenie)
	Zlecenie pobierz() override {
		Zlecenie z = kolejka_.front();
		kolejka_.pop();
		return z;
	}
	std::unique_ptr<IStrategia> nowaPusta() const override { return std::make_unique<StrategiaFifo>(); }
private:
	std::queue<Zlecenie> kolejka_;
};