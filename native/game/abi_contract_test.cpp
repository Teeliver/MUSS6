#include "stdafx.h"
#include "Protocol.h"
#include "GameMain.h"
#include "BloodCastle.h"
#include "EffectManager.h"
#include "PacketManager.h"
static_assert(sizeof(PBMSG_HEAD)==3&&sizeof(PSBMSG_HEAD)==4);
static_assert(sizeof(ENCDEC_HEADER)==6&&offsetof(ENCDEC_HEADER,size)==2);
static_assert(sizeof(PMSG_CONNECT_ACCOUNT_RECV)==59);
static_assert(offsetof(PMSG_CONNECT_ACCOUNT_RECV,TickCount)==34);
static_assert(offsetof(PMSG_CONNECT_ACCOUNT_RECV,ClientSerial)==43);
static_assert(sizeof(PMSG_BLOOD_CASTLE_SCORE_SEND)==29);
static_assert(offsetof(PMSG_BLOOD_CASTLE_SCORE_SEND,Score)==17);
static_assert(sizeof(PMSG_PARTY_EFFECT_LIST)==5&&offsetof(PMSG_PARTY_EFFECT_LIST,count)==1);
static_assert(sizeof(SDHP_GAME_SERVER_LIVE_SEND)==16);
static_assert(offsetof(SDHP_GAME_SERVER_LIVE_SEND,ServerCode)==4);
int main(){return 0;}
