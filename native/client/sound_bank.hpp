#pragma once
#import <Cocoa/Cocoa.h>
#include <filesystem>
// Decode Data/Sound once at scene preparation, never from a render callback.
namespace mu::audio {
inline NSMutableDictionary<NSString*,NSSound*>* bank(){static auto value=[NSMutableDictionary<NSString*,NSSound*> new];return value;}
inline void prepare(const std::filesystem::path& data){for(NSString* name in @[@"pWalk(Soil).wav",@"pWalk(Grass).wav",@"pWalk(Snow).wav",@"mBudgeAttack1.wav",@"mBudgeDie.wav",@"mSpider1.wav",@"mGhostAttack1.wav",@"mGhostDie.wav",@"mLarva1.wav",@"mLarva2.wav",@"eBow.wav",@"pDropItem.wav",@"pDropMoney.wav",@"eSwingWeapon1.wav",@"sMagic.wav",@"iTitle.wav",@"iButtonClick.wav",@"iCreateWindow.wav",@"iFLogInAlert.wav",@"aWind.wav",@"aForest.wav",@"aDungeon.wav"]){auto path=data/"Sound"/name.UTF8String;auto sound=[[NSSound alloc] initWithContentsOfFile:[NSString stringWithUTF8String:path.c_str()] byReference:NO];if(sound&&!bank()[name])bank()[name]=sound;}}
inline void play(NSString* name,float volume=1){if(!name)return;dispatch_async(dispatch_get_main_queue(),^{auto sound=bank()[name];if(!sound||sound.playing)return;sound.volume=volume;[sound play];});}
inline NSSound* ambientSound;
inline void ambient(NSString* name){if(ambientSound==(name?bank()[name]:nil))return;[ambientSound stop];ambientSound=name?bank()[name]:nil;ambientSound.loops=YES;ambientSound.volume=.15;[ambientSound play];}
inline NSString* monsterAttack(unsigned type){switch(type){case 2:return @"mBudgeAttack1.wav";case 3:return @"mSpider1.wav";case 11:return @"mGhostAttack1.wav";case 12:return @"mLarva1.wav";default:return nil;}}
inline NSString* monsterDeath(unsigned type){switch(type){case 2:return @"mBudgeDie.wav";case 3:return @"mSpider1.wav";case 11:return @"mGhostDie.wav";case 12:return @"mLarva2.wav";default:return nil;}}
}
