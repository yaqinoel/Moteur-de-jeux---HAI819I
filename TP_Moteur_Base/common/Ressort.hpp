#ifndef RESSORT_HPP
#define RESSORT_HPP
#include "GameObjet.hpp"

class Ressort {
public:
    float m_length;
    float m_k;
    GameObjet* m_objetStart;
    GameObjet* m_objetEnd;

    Ressort(GameObjet* objetStart, GameObjet* objetEnd, float length, float k) : m_objetStart(objetStart), m_objetEnd(objetEnd), m_length(length), m_k(k) {}
};

#endif //RESSORT_HPP