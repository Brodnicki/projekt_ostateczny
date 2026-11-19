#include "FabrykaStrategii.h"

#include "StrategiaAging.h"
#include "StrategiaFifo.h"
#include "StrategiaHpf.h"

#include <stdexcept>   // std::runtime_error

std::unique_ptr<IStrategia> utworzStrategie(const std::string& nazwaStrategii, int okresStarzenia) {
    if (nazwaStrategii == "FIFO") {
        return std::make_unique<StrategiaFifo>();
    }
    if (nazwaStrategii == "HPF") {
        return std::make_unique<StrategiaHpf>();
    }
    if (nazwaStrategii == "AGING") {
        return std::make_unique<StrategiaAging>(okresStarzenia);
    }
    throw std::runtime_error("Nieznana strategia: " + nazwaStrategii + " (dostępne: FIFO, HPF, AGING)");
}