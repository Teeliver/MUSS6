#include "stdafx.h"
#include "CashShop.h"
#include "PcPoint.h"
#include "MuCashShop.h"
#include <cstddef>

// These raw inter-server messages retain the Windows x86 default-alignment contract.
static_assert(sizeof(SDHP_CASH_SHOP_RECIEVE_POINT_SEND) == 32);
static_assert(sizeof(SDHP_CASH_SHOP_RECIEVE_POINT_RECV) == 44);
static_assert(sizeof(SDHP_PC_POINT_RECIEVE_POINT_SEND) == 32);
static_assert(sizeof(SDHP_PC_POINT_RECIEVE_POINT_RECV) == 36);
static_assert(sizeof(SDHP_MU_CASH_SHOP_RECIEVE_POINT_SEND) == 32);
static_assert(sizeof(SDHP_MU_CASH_SHOP_RECIEVE_POINT_RECV) == 36);
static_assert(offsetof(SDHP_CASH_SHOP_RECIEVE_POINT_SEND, CallbackFunc) == 20);
static_assert(offsetof(SDHP_CASH_SHOP_RECIEVE_POINT_SEND, CallbackArg1) == 24);
static_assert(offsetof(SDHP_CASH_SHOP_RECIEVE_POINT_SEND, CallbackArg2) == 28);
static_assert(sizeof(decltype(SDHP_CASH_SHOP_RECIEVE_POINT_SEND::CallbackFunc)) == 4);
static_assert(sizeof(decltype(SDHP_CASH_SHOP_RECIEVE_POINT_SEND::CallbackArg1)) == 4);
static_assert(sizeof(decltype(SDHP_CASH_SHOP_RECIEVE_POINT_SEND::CallbackArg2)) == 4);
static_assert(alignof(PMSG_CASH_SHOP_POINT_SEND) == 1);
static_assert(alignof(PMSG_MU_CASH_SHOP_ITEM_SEND) == 1);
int main() { return 0; }
