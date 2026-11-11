// Offline link test for original RandomManager.cpp. Only RNG input is deterministic here.
// This function is a test seam, never linked into the production GameServer.
#include "stdafx.h"
#include "RandomManager.h"
#include <cassert>
#include <cstdint>
#include <climits>

static long nextRandom = 0;
long GetLargeRand() { return nextRandom++; }

int main()
{
    static_assert(sizeof(std::intptr_t) == sizeof(void*), "weighted pointers retain the full address");
    CRandomManager manager;
    int unchanged = 42;
    assert(!manager.GetRandomElement(&unchanged) && unchanged == 42);
    manager.AddElement(9, 0);
    assert(!manager.GetRandomElement(&unchanged));

    int first = 11, second = 22;
    manager.AddElement(&first, 2);
    manager.AddElement(&second, 1);
    int* selected = nullptr;
    nextRandom = 0;
    assert(manager.GetRandomElement(&selected) && selected == &first);
    assert(manager.GetRandomElement(&selected) && selected == &first);
    assert(manager.GetRandomElement(&selected) && selected == &second);
    assert(*selected == 22);

    manager.Clear();
    selected = &first;
    assert(!manager.GetRandomElement(&selected) && selected == &first);

#if INTPTR_MAX > INT_MAX
    const std::intptr_t wideValue = static_cast<std::intptr_t>(INT_MAX) + 0x123456;
    manager.AddElement(wideValue, 1);
    std::intptr_t selectedValue = 0;
    assert(manager.GetRandomElement(&selectedValue) && selectedValue == wideValue);
    manager.Clear();
#endif

    manager.AddElement(-5, 1);
    int scalar = 0;
    assert(manager.GetRandomElement(&scalar) && scalar == -5);
    manager.Clear();
    manager.AddElement(70000, 1);
    WORD word = 0;
    assert(manager.GetRandomElement(&word) && word == static_cast<WORD>(70000));
    manager.Clear();
    manager.AddElement(511, 1);
    BYTE byte = 0;
    assert(manager.GetRandomElement(&byte) && byte == 255);
    return 0;
}
