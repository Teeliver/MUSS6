// Offline test against the original GameServer/Effect.cpp, not a reimplementation.
#include "stdafx.h"
#include "Effect.h"
#include <cassert>
#include <cstdint>
#include <ctime>

int main()
{
    static_assert(sizeof(BYTE) == 1, "MU byte ABI");
    static_assert(sizeof(WORD) == 2, "MU word ABI");
    static_assert(sizeof(DWORD) == 4, "MU DWORD ABI");
    static_assert(sizeof(QWORD) == 8, "MU QWORD ABI");
    static_assert(GAMESERVER_UPDATE == 603 && GAMESERVER_LANGUAGE == 1 && GAMESERVER_SHOP == 3,
                  "This test verifies only EX603ENG");

    CEffect effect;
    assert(!effect.IsEffect());
    assert(effect.Set(7, 3, 1, 0, 0, 0, 30, 1, 2, 3, 4));
    assert(effect.IsEffect());
    assert(effect.m_index == 7 && effect.m_group == 3 && effect.m_count == 30);
    assert(effect.m_value[0] == 1 && effect.m_value[3] == 4);
    assert(!effect.Set(8, 3, 1, 0, 0, 0, 10, 0, 0, 0, 0));
    assert(effect.m_index == 7); // Set refuses to replace an already active effect.
    effect.Clear();
    assert(!effect.IsEffect() && effect.m_count == 0);

    const DWORD now = static_cast<DWORD>(std::time(nullptr));
    assert(!effect.Set(9, 1, 1, 2, 0, 0, now - 1, 0, 0, 0, 0));
    assert(!effect.IsEffect()); // Type2 is an absolute expiry timestamp, not a millisecond duration.
    const DWORD expiry = static_cast<DWORD>(std::time(nullptr)) + 300;
    assert(effect.Set(9, 1, 1, 2, 0, 0, expiry, 0, 0, 0, 0));
    assert(effect.m_time == expiry && effect.m_count > 290 && effect.m_count <= 300);
    return 0;
}
