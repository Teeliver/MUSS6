#pragma once
#include "callback_tokens.hpp"

namespace mu {
// Channel values are the unchanged GAMESERVER_SHOP profile values.
CallbackWireToken registerStorePurchase(int buyerIndex, int sellerIndex, DWORD slot, DWORD channel);
void dispatchStorePurchase(int buyerIndex, const char* account, CallbackWireToken wire,
                           DWORD channel, DWORD coin1, DWORD coin2, DWORD coin3);
void cancelStorePurchase(CallbackWireToken wire);
void invalidateCashCallbackObject(int index);
void cancelAllCashCallbacks();
void pruneCashCallbacks();
}
