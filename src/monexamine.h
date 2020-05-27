#pragma once
#ifndef MONEXAMINE_H
#define MONEXAMINE_H

#include "custom_activity.h"

class monster;

namespace monexamine
{
bool pet_menu( monster &z );
bool mech_hack( monster &z );
bool pay_bot( monster &z );
void remove_battery( monster &z );
void insert_battery( monster &z );
void swap( monster &z );
void push( monster &z );
void rename_pet( monster &z );
void attach_bag_to( monster &z );
void remove_bag_from( monster &z );
void dump_items( monster &z );
bool give_items_to( monster &z );
bool add_armor( monster &z );
void remove_armor( monster &z );
void remove_harness( monster &z );
void play_with( monster &z );
void kill_zslave( monster &z );
void tie_or_untie( monster &z );
void mount_pet( monster &z );
void attach_or_remove_saddle( monster &z );
/*
*Manages the milking and milking cool down of monsters.
*Milked item uses starting_ammo, where ammo type is the milked item
*and amount the times per day you can milk the monster.
*/
void milk_source( monster &source_mon );

void maid_stay_or_follow( monster &z );
void maid_itemize( monster &z );
void maid_toggle_speak( monster &z );
void maid_toggle_wipe_floor( monster &z );
void maid_toggle_pickup( monster &z );
void shoggothmaid_toggle_cooking( monster &z );
void shoggothmaid_toggle_cooking_menu( monster &z );
void maid_play( monster &z );
void maid_change_costume( monster &z );
void shoggothmaid_get_hug( monster &z );
void start_custom_activity( monster &z, custom_activity *c_act);
void cubi_allow_seduce_friendlyfire( monster &z );
void cubi_allow_seduce_player( monster &z );
void cubi_toggle_ban_love_flame( monster &z );
void toggle_pet_stay( monster &z );
void heal_pet( monster &z );

} // namespace monexamine
#endif
