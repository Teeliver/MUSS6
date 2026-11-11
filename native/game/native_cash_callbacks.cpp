#include "stdafx.h"
#include "native_cash_callbacks.hpp"
#include "User.h"
#include "CustomStore.h"
#include "PersonalShop.h"
#include <cstring>
#include <string>

namespace mu {
namespace {
CallbackTokenRegistry& registry() { static CallbackTokenRegistry value; return value; }

struct Actor {
    CallbackObjectRef ref;
    std::string account, name;
};

Actor captureActor(int index)
{
    return {registry().capture(static_cast<DWORD>(index)), gObj[index].Account, gObj[index].Name};
}

bool alive(const Actor& actor)
{
    const int index = static_cast<int>(actor.ref.index);
    return OBJECT_USER_RANGE(index) && registry().isCurrent(actor.ref) && gObjIsConnectedGP(index) &&
        actor.account == gObj[index].Account && actor.name == gObj[index].Name;
}
}

CallbackWireToken registerStorePurchase(int buyerIndex, int sellerIndex, DWORD slot, DWORD channel)
{
    if (!OBJECT_USER_RANGE(buyerIndex) || !OBJECT_USER_RANGE(sellerIndex) ||
        !gObjIsConnectedGP(buyerIndex) || !gObjIsConnectedGP(sellerIndex) || !INVENTORY_SHOP_RANGE(slot)) return {};
    const auto buyer = captureActor(buyerIndex), seller = captureActor(sellerIndex);
    // The full function/object pointers stay in this process; DataServer receives opaque words only.
    return registry().issue(buyer.ref, seller.ref, slot, channel,
        [buyer, seller, slot](DWORD coin1, DWORD coin2, DWORD coin3) {
            CCustomStore::OnPShopBuyItemCallbackRecv(&gObj[buyer.ref.index], &gObj[seller.ref.index], slot, coin1, coin2, coin3);
        },
        [buyer, seller](CallbackCancelReason reason) {
            if (!alive(buyer)) return; // Never notify/operate on a replacement occupying the buyer's old slot.
            const BYTE result = reason == CallbackCancelReason::TargetInvalidated ? 3 : 6;
            gPersonalShop.GCPShopBuyItemSend(buyer.ref.index, seller.ref.index, 0, result);
        },
        [buyer, seller] { return alive(buyer) && alive(seller); });
}

void dispatchStorePurchase(int buyerIndex, const char* account, CallbackWireToken wire,
                           DWORD channel, DWORD coin1, DWORD coin2, DWORD coin3)
{
    if (!OBJECT_USER_RANGE(buyerIndex) || !account || !gObjIsAccountValid(buyerIndex, const_cast<char*>(account))) {
        registry().cancel(wire, CallbackCancelReason::OwnerInvalidated);
        return; // A late reply must not disconnect the new account now occupying the same object index.
    }
    registry().dispatch(wire, static_cast<DWORD>(buyerIndex), channel, coin1, coin2, coin3);
}

void cancelStorePurchase(CallbackWireToken wire) { registry().cancel(wire, CallbackCancelReason::SendFailure); }
void invalidateCashCallbackObject(int index) { if (OBJECT_RANGE(index)) registry().invalidateObject(static_cast<DWORD>(index)); }
void cancelAllCashCallbacks() { registry().cancelAll(); }
void pruneCashCallbacks() { registry().prune(); }
}
