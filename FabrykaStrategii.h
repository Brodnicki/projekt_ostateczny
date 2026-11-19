#pragma once
#include "IStrategia.h"

#include <memory>   // std::unique_ptr
#include <string>   // std::string

// "FIFO" / "HPF" / "AGING" -> obiekt strategii. Okres używa tylko AGING.
// Domyślny okres 1800 s: każde 30 min czekania jest warte +1 do priorytetu.
std::unique_ptr<IStrategia> utworzStrategie(const std::string& nazwaStrategii, int okresStarzenia = 1800);