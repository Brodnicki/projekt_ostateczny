cmake_minimum_required(VERSION 3.16)
project(SymulatorMagazynu LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Jeden program: generator + symulacja + porownanie strategii
add_executable(symulator
    main.cpp
    Generator.cpp
    Symulator.cpp
    Wykonawca.cpp
    Wczytywanie.cpp
    Statystyki.cpp
    FabrykaStrategii.cpp)

if(MSVC)
  target_compile_options(symulator PRIVATE /W4 /utf-8)
else()
  target_compile_options(symulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Kopia konfiguracji obok programu (do out/build/...), zeby program ja znalazl
configure_file(konfiguracja.txt konfiguracja.txt COPYONLY)
