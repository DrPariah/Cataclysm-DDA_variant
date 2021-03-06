#include "monexamine.h"

#include <climits>
#include <string>
#include <utility>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include "avatar.h"
#include "calendar.h"
#include "creature_tracker.h"
#include "game.h"
#include "game_inventory.h"
#include "handle_liquid.h"
#include "item.h"
#include "itype.h"
#include "iuse.h"
#include "map.h"
#include "messages.h"
#include "monster.h"
#include "mtype.h"
#include "npc.h"
#include "output.h"
#include "string_input_popup.h"
#include "translations.h"
#include "ui.h"
#include "units.h"
#include "bodypart.h"
#include "debug.h"
#include "enums.h"
#include "player_activity.h"
#include "rng.h"
#include "string_formatter.h"
#include "type_id.h"
#include "pimpl.h"
#include "point.h"
#include "custom_activity.h"
#include "options.h"
#include "speech.h"

static const activity_id ACT_MILK( "ACT_MILK" );
static const activity_id ACT_PLAY_WITH_PET( "ACT_PLAY_WITH_PET" );
static const activity_id ACT_CUSTOM_ACTIVITY( "ACT_CUSTOM_ACTIVITY" );

static const efftype_id effect_controlled( "controlled" );
static const efftype_id effect_harnessed( "harnessed" );
static const efftype_id effect_has_bag( "has_bag" );
static const efftype_id effect_monster_armor( "monster_armor" );
static const efftype_id effect_paid( "paid" );
static const efftype_id effect_pet( "pet" );
static const efftype_id effect_ridden( "ridden" );
static const efftype_id effect_saddled( "monster_saddled" );
static const efftype_id effect_tied( "tied" );

// littlemaid order things
const efftype_id effect_littlemaid_play( "littlemaid_play" );
const efftype_id effect_littlemaid_itemize( "littlemaid_itemize" );
const efftype_id effect_littlemaid_talk( "littlemaid_talk" );
const efftype_id effect_littlemaid_wipe_liquid( "littlemaid_wipe_liquid" );
const efftype_id effect_littlemaid_allow_pickup_item( "littlemaid_allow_pickup_item" );
const efftype_id effect_shoggothmaid_allow_cook( "shoggothmaid_allow_cook" );

// littlemaid order status things
const efftype_id effect_littlemaid_stay( "littlemaid_stay" );
const efftype_id effect_littlemaid_speak_off( "littlemaid_speak_off" );

// littlemaid play things
const efftype_id effect_littlemaid_in_kiss( "littlemaid_in_kiss" );
const efftype_id effect_littlemaid_in_petting( "littlemaid_in_petting" );
const efftype_id effect_littlemaid_in_service( "littlemaid_in_service" );
const efftype_id effect_littlemaid_in_special( "littlemaid_in_special" );
const efftype_id effect_shoggothmaid_in_hug( "shoggothmaid_in_hug" );

// littlemaid playing status things
const efftype_id effect_happiness( "happiness" );
const efftype_id effect_comfortness( "comfortness" );
const efftype_id effect_ecstasy( "ecstasy" );
const efftype_id effect_maid_fatigue( "maid_fatigue" );

// shoggoth maid cooking recipes control
const efftype_id effect_shoggothmaid_ban_recipe_bread( "shoggothmaid_ban_recipe_bread" );
const efftype_id effect_shoggothmaid_ban_recipe_scramble( "shoggothmaid_ban_recipe_scramble" );
const efftype_id effect_shoggothmaid_ban_recipe_donuts( "shoggothmaid_ban_recipe_donuts" );
const efftype_id effect_shoggothmaid_ban_recipe_pancake( "shoggothmaid_ban_recipe_pancake" );
const efftype_id effect_shoggothmaid_ban_recipe_deluxe( "shoggothmaid_ban_recipe_deluxe" );
const efftype_id effect_shoggothmaid_ban_recipe_cake( "shoggothmaid_ban_recipe_cake" );

const efftype_id effect_cooldown_of_custom_activity( "effect_cooldown_of_custom_activity" );
const efftype_id effect_cubi_allow_seduce_friendlyfire( "cubi_allow_seduce_friendlyfire" );
const efftype_id effect_cubi_allow_seduce_player( "cubi_allow_seduce_player" );
const efftype_id effect_cubi_ban_love_flame( "cubi_ban_love_flame" );

static const efftype_id effect_pet_stay_here( "pet_stay_here" );

static const efftype_id effect_pet_allow_distance_1( "pet_allow_distance_1" );
static const efftype_id effect_pet_allow_distance_2( "pet_allow_distance_2" );
static const efftype_id effect_pet_allow_distance_3( "pet_allow_distance_3" );
static const efftype_id effect_pet_allow_distance_5( "pet_allow_distance_5" );
static const efftype_id effect_pet_allow_distance_7( "pet_allow_distance_7" );
static const efftype_id effect_pet_allow_distance_10( "pet_allow_distance_10" );
static const efftype_id effect_pet_allow_distance_15( "pet_allow_distance_15" );

// littlemaid auto move things
const efftype_id effect_littlemaid_goodnight( "littlemaid_goodnight" );

static const skill_id skill_survival( "survival" );

static const species_id ZOMBIE( "ZOMBIE" );
static const species_id SPECIES_CUBI( "CUBI" );

bool monexamine::pet_menu( monster &z )
{
    enum choices {
        swap_pos = 0,
        push_zlave,
        rename,
        attach_bag,
        remove_bag,
        drop_all,
        give_items,
        mon_armor_add,
        mon_harness_remove,
        mon_armor_remove,
        play_with_pet,
        pheromone,
        milk,
        pay,
        attach_saddle,
        remove_saddle,
        mount,
        rope,
        remove_bat,
        insert_bat,
        check_bat,
        littlemaid_change_costume,
        littlemaid_itemize,
        littlemaid_toggle_speak,
        littlemaid_stay,
        littlemaid_wipe_floor,
        littlemaid_toggle_pickup,
        shoggothmaid_toggle_cook,
        shoggothmaid_cook_menu,
        shoggothmaid_hug,
        littlemaid_play,
        custom_activity_choice,
        cubi_menu_love_fire,
        cubi_toggle_seduce_friend,
        cubi_toggle_seduce_player,
        pet_stay_here,
        pet_healing,
        revert_to_item,
        pet_allow_distance,
    };

    uilist amenu;
    std::string pet_name = z.get_name();
    bool is_zombie = z.type->in_species( ZOMBIE );
    if( is_zombie ) {
        pet_name = _( "zombie slave" );
    }

    amenu.text = string_format( _( "What to do with your %s?" ), pet_name );

    amenu.addentry( swap_pos, true, 's', _( "Swap positions" ) );
    amenu.addentry( push_zlave, true, 'p', _( "Push %s" ), pet_name );
    amenu.addentry( rename, true, 'e', _( "Rename" ) );
    if( z.has_effect( effect_has_bag ) ) {
        amenu.addentry( give_items, true, 'g', _( "Place items into bag" ) );
        amenu.addentry( remove_bag, true, 'b', _( "Remove bag from %s" ), pet_name );
        if( !z.inv.empty() ) {
            amenu.addentry( drop_all, true, 'd', _( "Remove all items from bag" ) );
        }
    } else if( !z.has_flag( MF_RIDEABLE_MECH ) ) {
        amenu.addentry( attach_bag, true, 'b', _( "Attach bag to %s" ), pet_name );
    }
    if( z.has_effect( effect_harnessed ) ) {
        amenu.addentry( mon_harness_remove, true, 'H', _( "Remove vehicle harness from %s" ), pet_name );
    }
    if( z.has_effect( effect_monster_armor ) ) {
        amenu.addentry( mon_armor_remove, true, 'a', _( "Remove armor from %s" ), pet_name );
    } else if( !z.has_flag( MF_RIDEABLE_MECH ) ) {
        amenu.addentry( mon_armor_add, true, 'a', _( "Equip %s with armor" ), pet_name );
    }
    if( z.has_flag( MF_BIRDFOOD ) || z.has_flag( MF_CATFOOD ) || z.has_flag( MF_DOGFOOD ) ||
        z.has_flag( MF_CANPLAY ) ) {
        amenu.addentry( play_with_pet, true, 'y', _( "Play with %s" ), pet_name );
    }
    if( z.has_effect( effect_tied ) ) {
        amenu.addentry( rope, true, 't', _( "Untie" ) );
    } else if( !z.has_flag( MF_RIDEABLE_MECH ) ) {
        std::vector<item *> rope_inv = g->u.items_with( []( const item & itm ) {
            return itm.has_flag( "TIE_UP" );
        } );
        if( !rope_inv.empty() ) {
            amenu.addentry( rope, true, 't', _( "Tie" ) );
        } else {
            amenu.addentry( rope, false, 't', _( "You need any type of rope to tie %s in place" ),
                            pet_name );
        }
    }
    if( is_zombie ) {
        amenu.addentry( pheromone, true, 'z', _( "Tear out pheromone ball" ) );
    }

    if( z.has_flag( MF_MILKABLE ) ) {
        amenu.addentry( milk, true, 'm', _( "Milk %s" ), pet_name );
    }
    if( z.has_flag( MF_PET_MOUNTABLE ) && !z.has_effect( effect_saddled ) &&
        g->u.has_item_with_flag( "TACK" ) && g->u.get_skill_level( skill_survival ) >= 1 ) {
        amenu.addentry( attach_saddle, true, 'h', _( "Tack up %s" ), pet_name );
    } else if( z.has_flag( MF_PET_MOUNTABLE ) && z.has_effect( effect_saddled ) ) {
        amenu.addentry( remove_saddle, true, 'h', _( "Remove tack from %s" ), pet_name );
    } else if( z.has_flag( MF_PET_MOUNTABLE ) && !z.has_effect( effect_saddled ) &&
               g->u.has_item_with_flag( "TACK" ) && g->u.get_skill_level( skill_survival ) < 1 ) {
        amenu.addentry( remove_saddle, false, 'h', _( "You don't know how to saddle %s" ), pet_name );
    }
    if( z.has_flag( MF_PAY_BOT ) ) {
        amenu.addentry( pay, true, 'f', _( "Manage your friendship with %s" ), pet_name );
    }
    if( !z.has_flag( MF_RIDEABLE_MECH ) ) {
        if( z.has_flag( MF_PET_MOUNTABLE ) && g->u.can_mount( z ) ) {
            amenu.addentry( mount, true, 'r', _( "Mount %s" ), pet_name );
        } else if( !z.has_flag( MF_PET_MOUNTABLE ) ) {
            amenu.addentry( mount, false, 'r', _( "%s cannot be mounted" ), pet_name );
        } else if( z.get_size() <= g->u.get_size() ) {
            amenu.addentry( mount, false, 'r', _( "%s is too small to carry your weight" ), pet_name );
        } else if( g->u.get_skill_level( skill_survival ) < 1 ) {
            amenu.addentry( mount, false, 'r', _( "You have no knowledge of riding at all" ) );
        } else if( g->u.get_weight() >= z.get_weight() * z.get_mountable_weight_ratio() ) {
            amenu.addentry( mount, false, 'r', _( "You are too heavy to mount %s" ), pet_name );
        } else if( !z.has_effect( effect_saddled ) && g->u.get_skill_level( skill_survival ) < 4 ) {
            amenu.addentry( mount, false, 'r', _( "You are not skilled enough to ride without a saddle" ) );
        }
    } else {
        const itype &type = *item::find_type( z.type->mech_battery );
        int max_charge = type.magazine->capacity;
        float charge_percent;
        if( z.battery_item ) {
            charge_percent = static_cast<float>( z.battery_item->ammo_remaining() ) / max_charge * 100;
        } else {
            charge_percent = 0.0;
        }
        amenu.addentry( check_bat, false, 'c', _( "%s battery level is %d%%" ), z.get_name(),
                        static_cast<int>( charge_percent ) );
        if( g->u.weapon.is_null() && z.battery_item ) {
            amenu.addentry( mount, true, 'r', _( "Climb into the mech and take control" ) );
        } else if( !g->u.weapon.is_null() ) {
            amenu.addentry( mount, false, 'r', _( "You cannot pilot the mech whilst wielding something" ) );
        } else if( !z.battery_item ) {
            amenu.addentry( mount, false, 'r', _( "This mech has a dead battery and won't turn on" ) );
        }
        if( z.battery_item ) {
            amenu.addentry( remove_bat, true, 'x', _( "Remove the mech's battery pack" ) );
        } else if( g->u.has_amount( z.type->mech_battery, 1 ) ) {
            amenu.addentry( insert_bat, true, 'x', _( "Insert a new battery pack" ) );
        } else {
            amenu.addentry( insert_bat, false, 'x', _( "You need a %s to power this mech" ), type.nname( 1 ) );
        }
    }
    // variant botsu
//
//    if( z.has_effect( effect_pet_stay_here ) ){
//        amenu.addentry( pet_stay_here, true, 'f', _( "Follow me" ));
//    } else {
//        amenu.addentry( pet_stay_here, true, 'f', _( "Stay here" ));
//    }

    if( z.has_effect( effect_pet_allow_distance_1 ) ){
        amenu.addentry( pet_allow_distance, true, 'D', _( "Distance from master up to: %d" ), 1);
    } else if( z.has_effect( effect_pet_allow_distance_2 ) ){
        amenu.addentry( pet_allow_distance, true, 'D', _( "Distance from master up to: %d" ), 2);
    } else if( z.has_effect( effect_pet_allow_distance_3 ) ){
        amenu.addentry( pet_allow_distance, true, 'D', _( "Distance from master up to: %d" ), 3);
    } else if( z.has_effect( effect_pet_allow_distance_5 ) ){
        amenu.addentry( pet_allow_distance, true, 'D', _( "Distance from master up to: %d" ), 5);
    } else if( z.has_effect( effect_pet_allow_distance_7 ) ){
        amenu.addentry( pet_allow_distance, true, 'D', _( "Distance from master up to: %d" ), 7);
    } else if( z.has_effect( effect_pet_allow_distance_10 ) ){
        amenu.addentry( pet_allow_distance, true, 'D', _( "Distance from master up to: %d" ), 10);
    } else if( z.has_effect( effect_pet_allow_distance_15 ) ){
        amenu.addentry( pet_allow_distance, true, 'D', _( "Distance from master up to: %d" ), 15);
    } else {
        amenu.addentry( pet_allow_distance, true, 'D', _( "Distance from master up to: no limit" ));
    }

    amenu.addentry( pet_healing, true, 'h', _( "Heal pet" ));

    const auto mon_item_id = z.type->revert_to_itype;
    if( !mon_item_id.empty() ) {
        amenu.addentry( revert_to_item, true, 'R', _( "Revert to Item" ));

    }
    if( z.has_flag( MF_LITTLE_MAID ) ) {
        amenu.addentry( littlemaid_itemize, true, 'i', _( "Itemize littlemaid" ));

        if( !z.has_effect( effect_has_bag ) ) {
            if( !z.inv.empty() ) {
                amenu.addentry( drop_all, true, 'd', _( "Remove all items from bag" ) );
            }
        }

        amenu.addentry( littlemaid_change_costume, true, 'C', _( "Change costume" ));
        if( z.has_effect( effect_littlemaid_speak_off ) ){
            amenu.addentry( littlemaid_toggle_speak, true, 's', _( "Allow speak" ));
        } else {
            amenu.addentry( littlemaid_toggle_speak, true, 's', _( "Stop speak" ));
        }
        if( z.has_effect( effect_littlemaid_stay ) ){
            amenu.addentry( littlemaid_stay, true, 'f', _( "Follow me" ));
        } else {
            amenu.addentry( littlemaid_stay, true, 'f', _( "Stay here" ));
        }
        if( z.has_effect( effect_littlemaid_wipe_liquid ) ){
            amenu.addentry( littlemaid_wipe_floor, true, 'w', _( "Stop wipe floor" ));
        } else {
            amenu.addentry( littlemaid_wipe_floor, true, 'w', _( "Wipe floor" ));
        }
        if( z.has_effect( effect_littlemaid_allow_pickup_item ) ){
            amenu.addentry( littlemaid_toggle_pickup, true, 'w', _( "Stop pickup item" ));
        } else {
            amenu.addentry( littlemaid_toggle_pickup, true, 'w', _( "Pickup item" ));
        }
        amenu.addentry( littlemaid_play, true, 'l', _( "Lovely activity" ));
    }
    if( z.has_flag( MF_SHOGGOTH_MAID ) ) {

        if( !z.has_effect( effect_has_bag ) ) {
            if( !z.inv.empty() ) {
                amenu.addentry( drop_all, true, 'd', _( "Remove all items from bag" ) );
            }
        }

        amenu.addentry( littlemaid_change_costume, true, 'C', _( "Change costume" ));

        amenu.addentry( littlemaid_toggle_speak, false, 's', _( "Shoggoth maid do not stop speak" ));

        if( z.has_effect( effect_littlemaid_stay ) ){
            amenu.addentry( littlemaid_stay, true, 'f', _( "Follow me" ));
        } else {
            amenu.addentry( littlemaid_stay, true, 'f', _( "Stay here" ));
        }
        if( z.has_effect( effect_littlemaid_wipe_liquid ) ){
            amenu.addentry( littlemaid_wipe_floor, true, 'w', _( "Stop wipe floor" ));
        } else {
            amenu.addentry( littlemaid_wipe_floor, true, 'w', _( "Wipe floor" ));
        }

        if( z.has_effect( effect_shoggothmaid_allow_cook ) ){
            amenu.addentry( shoggothmaid_toggle_cook, true, 'c', _( "Stop Cooking" ));
            amenu.addentry( shoggothmaid_cook_menu, true, 'm', _("Cooking recipes" ));
        } else {
            amenu.addentry( shoggothmaid_toggle_cook, true, 'c', _( "Allow Cooking" ));
        }

        amenu.addentry( shoggothmaid_hug, true, 'h', _( "Get hug" ));

        amenu.addentry( littlemaid_play, true, 'l', _( "Lovely activity" ));
    }

    if( get_option<bool>( "HENTAI_EXTEND" ) ) {
        if( z.in_species( SPECIES_CUBI ) ){
            if( z.has_effect( effect_cubi_allow_seduce_friendlyfire ) ){
                amenu.addentry( cubi_toggle_seduce_friend, true, 'S', _( "Stop seduce anyone" ));
            } else {
                amenu.addentry( cubi_toggle_seduce_friend, true, 'S', _( "Seduce anyone" ));
            }

            if( z.has_effect( effect_cubi_allow_seduce_player ) ){
                amenu.addentry( cubi_toggle_seduce_player, true, 's', _( "Stop seduce me" ));
            } else {
                amenu.addentry( cubi_toggle_seduce_player, true, 's', _( "Seduce me" ));
            }
        }
        // XXX hardcoding name of cubi who use LOVE_FRAME.
        // special attacks is in z.special_attacks, have to make to finding LOVE_FLAME or something.
        if( z.type->id == "mon_greater_succubi" || z.type->id == "mon_greater_incubi" ){
            if( z.has_effect( effect_cubi_ban_love_flame ) ){
                amenu.addentry( cubi_menu_love_fire, true, 'F', _( "Allow love fire magic" ));
            } else {
                amenu.addentry( cubi_menu_love_fire, true, 'F', _( "Ban love fire magic" ));
            }
        }
    }

    custom_activity *c_act = custom_activity_manager::find_pet_monster_activity( z.type->id );
    if( c_act != nullptr ){
        amenu.addentry( custom_activity_choice, true, 'C',
                string_format( _( c_act->name_in_select_menu ), z.name() ));
    }

    amenu.query();
    int choice = amenu.ret;

    switch( choice ) {
        case swap_pos:
            swap( z );
            break;
        case push_zlave:
            push( z );
            break;
        case rename:
            rename_pet( z );
            break;
        case attach_bag:
            attach_bag_to( z );
            break;
        case remove_bag:
            remove_bag_from( z );
            break;
        case drop_all:
            dump_items( z );
            break;
        case give_items:
            return give_items_to( z );
        case mon_armor_add:
            return add_armor( z );
        case mon_harness_remove:
            remove_harness( z );
            break;
        case mon_armor_remove:
            remove_armor( z );
            break;
        case play_with_pet:
            if( query_yn( _( "Spend a few minutes to play with your %s?" ), pet_name ) ) {
                play_with( z );
            }
            break;
        case pheromone:
            if( query_yn( _( "Really kill the zombie slave?" ) ) ) {
                kill_zslave( z );
            }
            break;
        case rope:
            tie_or_untie( z );
            break;
        case attach_saddle:
        case remove_saddle:
            attach_or_remove_saddle( z );
            break;
        case mount:
            mount_pet( z );
            break;
        case milk:
            milk_source( z );
            break;
        case pay:
            pay_bot( z );
            break;
        case remove_bat:
            remove_battery( z );
            break;
        case insert_bat:
            insert_battery( z );
            break;
        case check_bat:
            break;
        case littlemaid_play:
            maid_play( z );
            break;
        case littlemaid_stay:
            maid_stay_or_follow( z );
            break;
        case littlemaid_itemize:
            maid_itemize( z );
            break;
        case littlemaid_toggle_speak:
            maid_toggle_speak( z );
            break;
        case littlemaid_wipe_floor:
            maid_toggle_wipe_floor( z );
            break;
        case littlemaid_toggle_pickup:
            maid_toggle_pickup( z );
            break;
        case littlemaid_change_costume:
            maid_change_costume( z );
            break;
        case custom_activity_choice:
            start_custom_activity( z, c_act );
            break;
        case cubi_toggle_seduce_friend:
            cubi_allow_seduce_friendlyfire( z );
            break;
        case cubi_toggle_seduce_player:
            cubi_allow_seduce_player( z );
            break;
        case shoggothmaid_toggle_cook:
            shoggothmaid_toggle_cooking( z );
            break;
        case shoggothmaid_cook_menu:
            shoggothmaid_toggle_cooking_menu( z );
            break;
        case shoggothmaid_hug:
            shoggothmaid_get_hug( z );
            break;
        case cubi_menu_love_fire:
            cubi_toggle_ban_love_flame( z );
            break;
        case pet_stay_here:
            toggle_pet_stay( z );
            break;
        case pet_healing:
            heal_pet( z );
            break;
        case revert_to_item:
            g->disable_robot( z.pos() );
            break;
        case pet_allow_distance:
            change_pet_allow_distance( z );
            break;

        default:
            break;
    }
    return true;
}

static item_location pet_armor_loc( monster &z )
{
    auto filter = [z]( const item & it ) {
        return z.type->bodytype == it.get_pet_armor_bodytype() &&
               z.get_volume() >= it.get_pet_armor_min_vol() &&
               z.get_volume() <= it.get_pet_armor_max_vol();
    };

    return game_menus::inv::titled_filter_menu( filter, g->u, _( "Pet armor" ) );
}

static item_location tack_loc()
{
    auto filter = []( const item & it ) {
        return it.has_flag( "TACK" );
    };

    return game_menus::inv::titled_filter_menu( filter, g->u, _( "Tack" ) );
}

void monexamine::remove_battery( monster &z )
{
    g->m.add_item_or_charges( g->u.pos(), *z.battery_item );
    z.battery_item.reset();
}

void monexamine::insert_battery( monster &z )
{
    if( z.battery_item ) {
        // already has a battery, shouldnt be called with one, but just incase.
        return;
    }
    std::vector<item *> bat_inv = g->u.items_with( []( const item & itm ) {
        return itm.has_flag( "MECH_BAT" );
    } );
    if( bat_inv.empty() ) {
        return;
    }
    int i = 0;
    uilist selection_menu;
    selection_menu.text = string_format( _( "Select an battery to insert into your %s." ),
                                         z.get_name() );
    selection_menu.addentry( i++, true, MENU_AUTOASSIGN, _( "Cancel" ) );
    for( auto iter : bat_inv ) {
        selection_menu.addentry( i++, true, MENU_AUTOASSIGN, _( "Use %s" ), iter->tname() );
    }
    selection_menu.selected = 1;
    selection_menu.query();
    auto index = selection_menu.ret;
    if( index == 0 || index == UILIST_CANCEL || index < 0 ||
        index > static_cast<int>( bat_inv.size() ) ) {
        return;
    }
    item *bat_item = bat_inv[index - 1];
    int item_pos = g->u.get_item_position( bat_item );
    if( item_pos != INT_MIN ) {
        z.battery_item = cata::make_value<item>( *bat_item );
        g->u.i_rem( item_pos );
    }
}

bool monexamine::mech_hack( monster &z )
{
    itype_id card_type = "id_military";
    if( g->u.has_amount( card_type, 1 ) ) {
        if( query_yn( _( "Swipe your ID card into the mech's security port?" ) ) ) {
            g->u.mod_moves( -100 );
            z.add_effect( effect_pet, 1_turns, num_bp, true );
            z.friendly = -1;
            add_msg( m_good, _( "The %s whirs into life and opens its restraints to accept a pilot." ),
                     z.get_name() );
            g->u.use_amount( card_type, 1 );
            return true;
        }
    } else {
        add_msg( m_info, _( "You do not have the required ID card to activate this mech." ) );
    }
    return false;
}

static int prompt_for_amount( const char *const msg, const int max )
{
    const std::string formatted = string_format( msg, max );
    const int amount = string_input_popup()
                       .title( formatted )
                       .width( 20 )
                       .text( to_string( max ) )
                       .only_digits( true )
                       .query_int();

    return clamp( amount, 0, max );
}

bool monexamine::pay_bot( monster &z )
{
    time_duration friend_time = z.get_effect_dur( effect_pet );
    const int charge_count = g->u.charges_of( "cash_card" );

    int amount = 0;
    uilist bot_menu;
    bot_menu.text = string_format(
                        _( "Welcome to the %s Friendship Interface.  What would you like to do?\n"
                           "Your current friendship will last: %s" ), z.get_name(), to_string( friend_time ) );
    if( charge_count > 0 ) {
        bot_menu.addentry( 1, true, 'b', _( "Get more friendship.  10 cents/min" ) );
    } else {
        bot_menu.addentry( 2, true, 'q',
                           _( "Sadly you're not currently able to extend your friendship.  - Quit menu" ) );
    }
    bot_menu.query();
    switch( bot_menu.ret ) {
        case 1:
            amount = prompt_for_amount(
                         ngettext( "How much friendship do you get?  Max: %d minute.  (0 to cancel)",
                                   "How much friendship do you get?  Max: %d minutes.", charge_count / 10 ), charge_count / 10 );
            if( amount > 0 ) {
                time_duration time_bought = time_duration::from_minutes( amount );
                g->u.use_charges( "cash_card", amount * 10 );
                z.add_effect( effect_pet, time_bought );
                z.add_effect( effect_paid, time_bought, num_bp, true );
                z.friendly = -1;
                popup( _( "Your friendship grows stronger!\n This %s will follow you for %s." ), z.get_name(),
                       to_string( z.get_effect_dur( effect_pet ) ) );
                return true;
            }
            break;
        case 2:
            break;
    }

    return false;
}

void monexamine::attach_or_remove_saddle( monster &z )
{
    if( z.has_effect( effect_saddled ) ) {
        z.remove_effect( effect_saddled );
        g->u.i_add( *z.tack_item );
        z.tack_item.reset();
    } else {
        item_location loc = tack_loc();

        if( !loc ) {
            add_msg( _( "Never mind." ) );
            return;
        }
        z.add_effect( effect_saddled, 1_turns, num_bp, true );
        z.tack_item = cata::make_value<item>( *loc.get_item() );
        loc.remove_item();
    }
}

bool Character::can_mount( const monster &critter ) const
{
    const auto &avoid = get_path_avoid();
    auto route = g->m.route( pos(), critter.pos(), get_pathfinding_settings(), avoid );

    if( route.empty() ) {
        return false;
    }
    return ( critter.has_flag( MF_PET_MOUNTABLE ) && critter.friendly == -1 &&
             !critter.has_effect( effect_controlled ) && !critter.has_effect( effect_ridden ) ) &&
           ( ( critter.has_effect( effect_saddled ) && get_skill_level( skill_survival ) >= 1 ) ||
             get_skill_level( skill_survival ) >= 4 ) && ( critter.get_size() >= ( get_size() + 1 ) &&
                     get_weight() <= critter.get_weight() * critter.get_mountable_weight_ratio() );
}

void monexamine::mount_pet( monster &z )
{
    g->u.mount_creature( z );
}

void monexamine::swap( monster &z )
{
    std::string pet_name = z.get_name();
    g->u.moves -= 150;

    ///\EFFECT_STR increases chance to successfully swap positions with your pet
    ///\EFFECT_DEX increases chance to successfully swap positions with your pet
    if( !one_in( ( g->u.str_cur + g->u.dex_cur ) / 6 ) ) {
        bool t = z.has_effect( effect_tied );
        if( t ) {
            z.remove_effect( effect_tied );
        }

        g->swap_critters( g->u, z );

        if( t ) {
            z.add_effect( effect_tied, 1_turns, num_bp, true );
        }
        add_msg( _( "You swap positions with your %s." ), pet_name );
    } else {
        add_msg( _( "You fail to budge your %s!" ), pet_name );
    }
}

void monexamine::push( monster &z )
{
    std::string pet_name = z.get_name();
    g->u.moves -= 30;

    ///\EFFECT_STR increases chance to successfully push your pet
    if( !one_in( g->u.str_cur ) ) {
        add_msg( _( "You pushed the %s." ), pet_name );
    } else {
        add_msg( _( "You pushed the %s, but it resisted." ), pet_name );
        return;
    }

    int deltax = z.posx() - g->u.posx(), deltay = z.posy() - g->u.posy();
    z.move_to( tripoint( z.posx() + deltax, z.posy() + deltay, z.posz() ) );
}

void monexamine::rename_pet( monster &z )
{
    std::string unique_name = string_input_popup()
                              .title( _( "Enter new pet name:" ) )
                              .width( 20 )
                              .query_string();
    if( !unique_name.empty() ) {
        z.unique_name = unique_name;
    }
}

void monexamine::attach_bag_to( monster &z )
{
    std::string pet_name = z.get_name();

    auto filter = []( const item & it ) {
        return it.is_armor() && it.get_storage() > 0_ml;
    };

    item_location loc = game_menus::inv::titled_filter_menu( filter, g->u, _( "Bag item" ) );

    if( !loc ) {
        add_msg( _( "Never mind." ) );
        return;
    }

    item &it = *loc;
    z.storage_item = cata::make_value<item>( it );
    add_msg( _( "You mount the %1$s on your %2$s." ), it.display_name(), pet_name );
    g->u.i_rem( &it );
    z.add_effect( effect_has_bag, 1_turns, num_bp, true );
    // Update encumbrance in case we were wearing it
    g->u.flag_encumbrance();
    g->u.moves -= 200;
}

void monexamine::remove_bag_from( monster &z )
{
    std::string pet_name = z.get_name();
    if( z.storage_item ) {
        if( !z.inv.empty() ) {
            dump_items( z );
        }
        g->m.add_item_or_charges( g->u.pos(), *z.storage_item );
        add_msg( _( "You remove the %1$s from %2$s." ), z.storage_item->display_name(), pet_name );
        z.storage_item.reset();
        g->u.moves -= 200;
    } else {
        add_msg( m_bad, _( "Your %1$s doesn't have a bag!" ), pet_name );
    }
    z.remove_effect( effect_has_bag );
}

void monexamine::dump_items( monster &z )
{
    std::string pet_name = z.get_name();
    for( auto &it : z.inv ) {
        g->m.add_item_or_charges( g->u.pos(), it );
    }
    z.inv.clear();
    add_msg( _( "You dump the contents of the %s's bag on the ground." ), pet_name );
    g->u.moves -= 200;
}

bool monexamine::give_items_to( monster &z )
{
    std::string pet_name = z.get_name();
    if( !z.storage_item ) {
        add_msg( _( "There is no container on your %s to put things in!" ), pet_name );
        return true;
    }

    item &storage = *z.storage_item;
    units::mass max_weight = z.weight_capacity() - z.get_carried_weight();
    units::volume max_volume = storage.get_storage() - z.get_carried_volume();

    drop_locations items = game_menus::inv::multidrop( g->u );
    drop_locations to_move;
    for( const drop_location &itq : items ) {
        const item &it = *itq.first;
        units::volume item_volume = it.volume() * itq.second;
        units::mass item_weight = it.weight() * itq.second;
        if( max_weight < item_weight ) {
            add_msg( _( "The %1$s is too heavy for the %2$s to carry." ), it.tname(), pet_name );
            continue;
        } else if( max_volume < item_volume ) {
            add_msg( _( "The %1$s is too big to fit in the %2$s." ), it.tname(), storage.tname() );
            continue;
        } else {
            max_weight -= item_weight;
            max_volume -= item_volume;
            to_move.insert( to_move.end(), itq );
        }
    }
    z.add_effect( effect_controlled, 5_turns );
    g->u.drop( to_move, z.pos(), true );

    return false;
}

bool monexamine::add_armor( monster &z )
{
    std::string pet_name = z.get_name();
    item_location loc = pet_armor_loc( z );

    if( !loc ) {
        add_msg( _( "Never mind." ) );
        return true;
    }

    item &armor = *loc;
    units::mass max_weight = z.weight_capacity() - z.get_carried_weight();
    if( max_weight <= armor.weight() ) {
        add_msg( pgettext( "pet armor", "Your %1$s is too heavy for your %2$s." ), armor.tname( 1 ),
                 pet_name );
        return true;
    }

    armor.set_var( "pet_armor", "true" );
    z.armor_item = cata::make_value<item>( armor );
    add_msg( pgettext( "pet armor", "You put the %1$s on your %2$s." ), armor.display_name(),
             pet_name );
    loc.remove_item();
    z.add_effect( effect_monster_armor, 1_turns, num_bp, true );
    // TODO: armoring a horse takes a lot longer than 2 seconds. This should be a long action.
    g->u.moves -= 200;
    return true;
}

void monexamine::remove_harness( monster &z )
{
    z.remove_effect( effect_harnessed );
    add_msg( m_info, _( "You unhitch %s from the vehicle." ), z.get_name() );
}

void monexamine::remove_armor( monster &z )
{
    std::string pet_name = z.get_name();
    if( z.armor_item ) {
        z.armor_item->erase_var( "pet_armor" );
        g->m.add_item_or_charges( z.pos(), *z.armor_item );
        add_msg( pgettext( "pet armor", "You remove the %1$s from %2$s." ), z.armor_item->display_name(),
                 pet_name );
        z.armor_item.reset();
        // TODO: removing armor from a horse takes a lot longer than 2 seconds. This should be a long action.
        g->u.moves -= 200;
    } else {
        add_msg( m_bad, _( "Your %1$s isn't wearing armor!" ), pet_name );
    }
    z.remove_effect( effect_monster_armor );
}

void monexamine::play_with( monster &z )
{
    std::string pet_name = z.get_name();
    g->u.assign_activity( ACT_PLAY_WITH_PET, rng( 50, 125 ) * 100 );
    g->u.activity.str_values.push_back( pet_name );
}

void monexamine::kill_zslave( monster &z )
{
    z.apply_damage( &g->u, bp_torso, 100 ); // damage the monster (and its corpse)
    z.die( &g->u ); // and make sure it's really dead

    g->u.moves -= 150;

    if( !one_in( 3 ) ) {
        g->u.add_msg_if_player( _( "You tear out the pheromone ball from the zombie slave." ) );
        item ball( "pheromone", 0 );
        iuse pheromone;
        pheromone.pheromone( &g->u, &ball, true, g->u.pos() );
    }
}

void monexamine::tie_or_untie( monster &z )
{
    if( z.has_effect( effect_tied ) ) {
        z.remove_effect( effect_tied );
        if( z.tied_item ) {
            g->u.i_add( *z.tied_item );
            z.tied_item.reset();
        }
    } else {
        std::vector<item *> rope_inv = g->u.items_with( []( const item & itm ) {
            return itm.has_flag( "TIE_UP" );
        } );
        if( rope_inv.empty() ) {
            return;
        }
        int i = 0;
        uilist selection_menu;
        selection_menu.text = string_format( _( "Select an item to tie your %s with." ), z.get_name() );
        selection_menu.addentry( i++, true, MENU_AUTOASSIGN, _( "Cancel" ) );
        for( auto iter : rope_inv ) {
            selection_menu.addentry( i++, true, MENU_AUTOASSIGN, _( "Use %s" ), iter->tname() );
        }
        selection_menu.selected = 1;
        selection_menu.query();
        auto index = selection_menu.ret;
        if( index == 0 || index == UILIST_CANCEL || index < 0 ||
            index > static_cast<int>( rope_inv.size() ) ) {
            return;
        }
        item *rope_item = rope_inv[index - 1];
        int item_pos = g->u.get_item_position( rope_item );
        if( item_pos != INT_MIN ) {
            z.tied_item = cata::make_value<item>( *rope_item );
            g->u.i_rem( item_pos );
            z.add_effect( effect_tied, 1_turns, num_bp, true );
        }
    }
}

void monexamine::milk_source( monster &source_mon )
{
    std::string milked_item = source_mon.type->starting_ammo.begin()->first;
    auto milkable_ammo = source_mon.ammo.find( milked_item );
    if( milkable_ammo == source_mon.ammo.end() ) {
        debugmsg( "The %s has no milkable %s.", source_mon.get_name(), milked_item );
        return;
    }
    if( milkable_ammo->second > 0 ) {
        const int moves = to_moves<int>( time_duration::from_minutes( milkable_ammo->second / 2 ) );
        g->u.assign_activity( ACT_MILK, moves, -1 );
        g->u.activity.coords.push_back( g->m.getabs( source_mon.pos() ) );
        // pin the cow in place if it isn't already
        bool temp_tie = !source_mon.has_effect( effect_tied );
        if( temp_tie ) {
            source_mon.add_effect( effect_tied, 1_turns, num_bp, true );
            g->u.activity.str_values.push_back( "temp_tie" );
        }
        add_msg( _( "You milk the %s." ), source_mon.get_name() );
    } else {
        add_msg( _( "The %s has no more milk." ), source_mon.get_name() );
    }
}

void monexamine::maid_stay_or_follow( monster &z )
{
    if( z.has_effect( effect_littlemaid_stay ) ) {
        add_msg( _("order %s to follow."), z.name() );
        z.remove_effect( effect_littlemaid_stay );
    } else {
        add_msg( _("order %s to stay."), z.name() );
        z.add_effect( effect_littlemaid_stay, 1_turns, num_bp, true );
    }
}

void monexamine::maid_itemize( monster &z )
{
    if( z.storage_item || z.armor_item || !z.inv.empty() || z.tied_item ){
        add_msg( m_info, _("remove all equipment from littlemaid before itemize.") );
    } else if( query_yn( _( "Itemize the %s?" ), z.name() ) ) {
        g->u.moves -= 100;
        g->u.i_add( item( "little_maid_R18_milk_sanpo" ) );
        g->remove_zombie( z );
    }
}

void monexamine::maid_change_costume( monster &z )
{
    if( query_yn( _( "Change %s's costume?" ), z.name() ) ) {
        g->u.moves -= 100;
        // little maid
        if (z.type->id == mtype_id("mon_little_maid_R18_milk_sanpo")) {
            z.poly( mtype_id("mon_little_maid_R18_milk_sanpo_altanate"));
        } else if (z.type->id == mtype_id("mon_little_maid_R18_milk_sanpo_altanate") && one_in(10) ){
            z.poly( mtype_id("mon_little_maid_R18_milk_sanpo_classic"));
        } else if (z.type->id == mtype_id("mon_little_maid_R18_milk_sanpo_altanate")  ){
            z.poly( mtype_id("mon_little_maid_R18_milk_sanpo"));
        } else if (z.type->id == mtype_id("mon_little_maid_R18_milk_sanpo_classic")  ){
            z.poly( mtype_id("mon_little_maid_R18_milk_sanpo"));

        // shoggoth maid
        } else if (z.type->id == mtype_id("mon_shoggoth_maid")  ){
            z.poly( mtype_id("mon_shoggoth_maid_altanate"));
        } else if (z.type->id == mtype_id("mon_shoggoth_maid_altanate") && one_in(10) ){
            z.poly( mtype_id("mon_shoggoth_maid_classic"));
        } else if (z.type->id == mtype_id("mon_shoggoth_maid_altanate")  ){
            z.poly( mtype_id("mon_shoggoth_maid"));
        } else if (z.type->id == mtype_id("mon_shoggoth_maid_classic")  ){
            z.poly( mtype_id("mon_shoggoth_maid"));
        }
    }
}

void monexamine::maid_toggle_speak( monster &z )
{
    if( z.has_effect( effect_littlemaid_speak_off ) ) {
        add_msg( _("allow littlemaid speak.") );
        z.remove_effect( effect_littlemaid_speak_off );
    } else {
        add_msg( _("stop littlemaid speak.") );
        z.add_effect( effect_littlemaid_speak_off, 1_turns, num_bp, true );
    }
}

void monexamine::maid_toggle_wipe_floor( monster &z )
{
    if( z.has_effect( effect_littlemaid_wipe_liquid) ) {
        add_msg( _("Ordered to stop wipe floor to maid.") );
        z.remove_effect( effect_littlemaid_wipe_liquid );
    } else {
        add_msg( _("Ordered to wipe floor to maid.") );
        z.add_effect( effect_littlemaid_wipe_liquid, 1_turns, num_bp, true );
    }
}

void monexamine::maid_toggle_pickup( monster &z )
{
    if( z.has_effect( effect_littlemaid_allow_pickup_item) ) {
        add_msg( _("Ordered to stop pickup item to maid.") );
        z.remove_effect( effect_littlemaid_allow_pickup_item );
    } else {
        add_msg( _("Ordered to pickup item to maid.") );
        z.add_effect( effect_littlemaid_allow_pickup_item, 1_turns, num_bp, true );
    }
}

void monexamine::shoggothmaid_toggle_cooking( monster &z )
{
    if( z.has_effect( effect_shoggothmaid_allow_cook) ) {
        add_msg( _("Ordered to stop cooking to maid.") );
        z.remove_effect( effect_shoggothmaid_allow_cook );
    } else {
        add_msg( _("Ordered to cooking to maid. mount food storage tag to food cargo on your vehicle, and put wheat or egg (or sugar is optional) there. maid will pickup ingredents and cook it, then store meal to food storage.") );
        z.add_effect( effect_shoggothmaid_allow_cook, 1_turns, num_bp, true );
    }
}

void monexamine::shoggothmaid_toggle_cooking_menu( monster &z )
{
    enum recipe_choices {
        shoggothmaid_recipe_scramble_on = 0,
        shoggothmaid_recipe_scramble_off,
        shoggothmaid_recipe_bread_on,
        shoggothmaid_recipe_bread_off,
        shoggothmaid_recipe_deluxe_on,
        shoggothmaid_recipe_deluxe_off,
        shoggothmaid_recipe_pancake_on,
        shoggothmaid_recipe_pancake_off,
        shoggothmaid_recipe_donuts_on,
        shoggothmaid_recipe_donuts_off,
        shoggothmaid_recipe_cake_on,
        shoggothmaid_recipe_cake_off,
        shoggothmaid_recipe_menu_quit,
    };

    bool is_loop = true;
    while( is_loop ) {
        uilist amenu;
        amenu.text = string_format( _( "Shoggoth maid cooking menu" ) );

        if( z.has_effect( effect_shoggothmaid_ban_recipe_scramble ) ) {
            amenu.addentry( shoggothmaid_recipe_scramble_on, true, 's',
                    _( "Scrambled eggs(         1 Powder egg          ): OFF" ) );
        } else {
            amenu.addentry( shoggothmaid_recipe_scramble_off, true, 's',
                    _( "Scrambled eggs(         1 Powder egg          ): ON" ) );
        }
        if( z.has_effect( effect_shoggothmaid_ban_recipe_bread ) ) {
            amenu.addentry( shoggothmaid_recipe_bread_on, true, 'b',
                    _( "Bread         (1 Wheat                        ): OFF" ) );
        } else {
            amenu.addentry( shoggothmaid_recipe_bread_off, true, 'b',
                    _( "Bread         (1 Wheat                        ): ON" ) );
        }
        if( z.has_effect( effect_shoggothmaid_ban_recipe_deluxe ) ) {
            amenu.addentry( shoggothmaid_recipe_deluxe_on, true, 'd',
                    _( "DX eggs       (         1 Powder egg, 10 Sugar): OFF" ) );
        } else {
            amenu.addentry( shoggothmaid_recipe_deluxe_off, true, 'd',
                    _( "DX eggs       (         1 Powder egg, 10 Sugar): ON" ) );
        }
        if( z.has_effect( effect_shoggothmaid_ban_recipe_pancake ) ) {
            amenu.addentry( shoggothmaid_recipe_pancake_on, true, 'p',
                    _( "Pancakes      (1 Wheat, 1 Powder egg          ): OFF" ) );
        } else {
            amenu.addentry( shoggothmaid_recipe_pancake_off, true, 'p',
                    _( "Pankaces      (1 Wheat, 1 Powder egg          ): ON" ) );
        }
        if( z.has_effect( effect_shoggothmaid_ban_recipe_donuts ) ) {
            amenu.addentry( shoggothmaid_recipe_donuts_on, true, 'D',
                    _( "Donuts        (1 Wheat,               10 Sugar): OFF" ) );
        } else {
            amenu.addentry( shoggothmaid_recipe_donuts_off, true, 'D',
                    _( "Donuts        (1 Wheat,               10 Sugar): ON" ) );
        }
        if( z.has_effect( effect_shoggothmaid_ban_recipe_cake ) ) {
            amenu.addentry( shoggothmaid_recipe_cake_on, true, 'c',
                    _( "CAKE          (1 Wheat, 1 Powder egg, 10 Sugar): OFF" ) );
        } else {
            amenu.addentry( shoggothmaid_recipe_cake_off, true, 'c',
                    _( "CAKE          (1 Wheat, 1 Powder egg, 10 Sugar): ON" ) );
        }
        amenu.addentry( shoggothmaid_recipe_menu_quit, true, 'q',
                            _( "Quit recipes setting" ) );
        amenu.query();
        int choice = amenu.ret;

        switch( choice ) {
            case shoggothmaid_recipe_bread_on:
                z.remove_effect( effect_shoggothmaid_ban_recipe_bread );
                break;
            case shoggothmaid_recipe_bread_off:
                z.add_effect( effect_shoggothmaid_ban_recipe_bread, 1_turns, num_bp, true );
                break;
            case shoggothmaid_recipe_scramble_on:
                z.remove_effect( effect_shoggothmaid_ban_recipe_scramble );
                break;
            case shoggothmaid_recipe_scramble_off:
                z.add_effect( effect_shoggothmaid_ban_recipe_scramble, 1_turns, num_bp, true );
                break;
            case shoggothmaid_recipe_pancake_on:
                z.remove_effect( effect_shoggothmaid_ban_recipe_pancake );
                break;
            case shoggothmaid_recipe_pancake_off:
                z.add_effect( effect_shoggothmaid_ban_recipe_pancake, 1_turns, num_bp, true );
                break;
            case shoggothmaid_recipe_donuts_on:
                z.remove_effect( effect_shoggothmaid_ban_recipe_donuts );
                break;
            case shoggothmaid_recipe_donuts_off:
                z.add_effect( effect_shoggothmaid_ban_recipe_donuts, 1_turns, num_bp, true );
                break;
            case shoggothmaid_recipe_deluxe_on:
                z.remove_effect( effect_shoggothmaid_ban_recipe_deluxe );
                break;
            case shoggothmaid_recipe_deluxe_off:
                z.add_effect( effect_shoggothmaid_ban_recipe_deluxe, 1_turns, num_bp, true );
                break;
            case shoggothmaid_recipe_cake_on:
                z.remove_effect( effect_shoggothmaid_ban_recipe_cake );
                break;
            case shoggothmaid_recipe_cake_off:
                z.add_effect( effect_shoggothmaid_ban_recipe_cake, 1_turns, num_bp, true );
                break;
            case UILIST_CANCEL:
                is_loop = false;
                break;
            case shoggothmaid_recipe_menu_quit:
                is_loop = false;
                break;
        }
    }
}


void monexamine::maid_play( monster &z )
{

    enum maid_choices {
        littlemaid_kiss = 0,
        littlemaid_petting,
        littlemaid_service,
        littlemaid_special
    };

    uilist amenu;
    std::string pet_name = z.get_name();
    amenu.text = string_format( _( "What to do with your %s?" ), pet_name );

    amenu.addentry( littlemaid_kiss, true, 'k', _( "Kiss" ) );
    amenu.addentry( littlemaid_petting, true, 'p', _( "Petting" ) );
    amenu.addentry( littlemaid_service, true, 's', _( "Get service" ) );
    amenu.addentry( littlemaid_special, true, 'x', _( "Special thing" ) );

    amenu.query();
    int choice = amenu.ret;

    string_id<activity_type> act_id;
    switch( choice ) {
    case littlemaid_kiss:
        act_id = activity_id( "ACT_LITTLEMAID_KISS" );
        break;
    case littlemaid_petting:
        act_id = activity_id( "ACT_LITTLEMAID_PETTING" );
        break;
    case littlemaid_service:
        act_id = activity_id( "ACT_LITTLEMAID_SERVICE" );
        break;
    case littlemaid_special:
        act_id = activity_id( "ACT_LITTLEMAID_SPECIAL" );
        break;
    default:
        return;
    }
    player_activity act = player_activity( act_id,
                        to_moves<int>( 10_minutes ),-1,0,"having fun with maid" );
    act.monsters.emplace_back( g->shared_from( z ) );
    g->u.assign_activity(act);
}

void monexamine::shoggothmaid_get_hug( monster &maid ){
    const SpeechBubble &speech = get_speech( "shoggoth_maid_hug_start" );
    sounds::sound( maid.pos(), speech.volume, sounds::sound_t::speech, speech.text.translated(),
                   false, "speech", maid.type->id.str() );
    player_activity act = player_activity( activity_id( "ACT_SHOGGOTHMAID_GET_HUG" ),
                        to_moves<int>( 15_minutes ),-1,0,"getting hug from shoggoth maid" );
    act.monsters.emplace_back( g->shared_from( maid ) );
    g->u.assign_activity(act);
}

void monexamine::start_custom_activity( monster &z, custom_activity *c_act){


    if( z.has_effect(effect_cooldown_of_custom_activity) ) {
        add_msg( m_bad, _(c_act->not_ready_message), z.name() );
        return;
    }

    if( c_act->query_yn_message != "" ) {
        const std::string msg = c_act->query_yn_message;
        if( !query_yn( string_format( _( c_act->query_yn_message ), z.name() )  ) ) {
            return;
        }
    }

    activity_id act_id = activity_id( "ACT_CUSTOM_ACTIVITY" );

    player_activity act = player_activity( act_id,
                        to_moves<int>( time_duration::from_turns( c_act->to_finish_turns ) ),-1,0, c_act->name );
    act.monsters.emplace_back( g->shared_from( z ) );
    act.custom_activity_data = c_act;
    g->u.assign_activity(act);
    if( c_act->start_message != "" ){
        add_msg( _(c_act->start_message), z.name() );
    }
}

void monexamine::cubi_allow_seduce_friendlyfire( monster &z )
{
    if( z.has_effect( effect_cubi_allow_seduce_friendlyfire ) ) {
        add_msg( _("Stop make %s to seduce anyone."), z.name() );
        z.remove_effect( effect_cubi_allow_seduce_friendlyfire );
    } else {
        add_msg( _("Make %s to seduce anyone."), z.name() );
        z.add_effect( effect_cubi_allow_seduce_friendlyfire, 1_turns, num_bp, true );
    }
}

void monexamine::cubi_allow_seduce_player( monster &z )
{
    if( z.has_effect( effect_cubi_allow_seduce_player ) ) {
        add_msg( _("Stop make %s to seduce you."), z.name() );
        z.remove_effect( effect_cubi_allow_seduce_player );
    } else {
        add_msg( _("Make %s to seduce you."), z.name() );
        z.add_effect( effect_cubi_allow_seduce_player, 1_turns, num_bp, true );
    }
}

void monexamine::cubi_toggle_ban_love_flame( monster &z )
{
    if( z.has_effect( effect_cubi_ban_love_flame ) ) {
        add_msg( _("You told %s to do burn everything by love."), z.name() );
        z.enable_special( "LOVE_FLAME" );
        z.remove_effect( effect_cubi_ban_love_flame );
    } else {
        add_msg( _("You told %s to do not burn everything by love."), z.name() );
        z.disable_special( "LOVE_FLAME" );
        z.add_effect( effect_cubi_ban_love_flame, 1_turns, num_bp, true );
    }
}

void monexamine::toggle_pet_stay( monster &z )
{
    if( z.has_effect( effect_pet_stay_here ) ) {
        add_msg( _("order %s to follow."), z.name() );
        z.set_stay_place_to_here();
        z.remove_effect( effect_pet_stay_here );
    } else {
        add_msg( _("order %s to stay."), z.name() );
        z.set_stay_place_to_here();
        z.add_effect( effect_pet_stay_here, 1_turns, num_bp, true );
    }
}

void monexamine::heal_pet( monster &z )
{
    // FIXME code is copy pesta from using toilet paper code

    // search paper
    std::vector<const std::list<item> *> med_item_list;
    // std::vector<const std::list<item> *>
    const_invslice inventory = g->u.inv.const_slice();

    for( const std::list<item> *itemstack : inventory ){
        auto item = itemstack->front();
        const cata::value_ptr<islot_comestible> &med_com = item.get_comestible();

        if(med_com) {
            if( med_com->comesttype == "MED" ) {
                std::string med_item_id = item.typeId();
                if( med_item_id.find( "bandage" ) != std::string::npos ) {
                    med_item_list.push_back( itemstack );
                }
            }
        }
    }

    if( med_item_list.size() == 0) {
        // dont have paper
        popup( _("you have not any heal item.") );
        return;
    }
    // paper select menu
    uilist amenu;
    const int SELECT_NOT_USE_PAPER = -99;
    amenu.text = string_format( _( "healing pet" ) );
    amenu.addentry( SELECT_NOT_USE_PAPER , true, '0', _( "cancel" ));

    for(long long unsigned int i = 0 ; i < med_item_list.size() ; i++) {
        item item = med_item_list.at(i)->front();
        amenu.addentry( i , true, -1 , item.tname());
    }
    amenu.query();
    int choice = amenu.ret;

    if( 0 <= choice ){
        const item *using_paper = &(med_item_list.at(choice)->front());
        using_paper->typeId();
        // fuel consume
        std::vector<item_comp> consume_item;
        consume_item.push_back( item_comp( using_paper->typeId() , 1 ) );
        g->u.consume_items( consume_item, 1, is_crafting_component );

        // FIXME HARDCORED MAGIC NUMBER!!
        z.heal(5, false);
        g->u.mod_moves( -999 );
    }
}

static void remove_all_allow_distance_effect( monster &z ) {
    z.remove_effect(effect_pet_allow_distance_1);
    z.remove_effect(effect_pet_allow_distance_2);
    z.remove_effect(effect_pet_allow_distance_3);
    z.remove_effect(effect_pet_allow_distance_5);
    z.remove_effect(effect_pet_allow_distance_7);
    z.remove_effect(effect_pet_allow_distance_10);
    z.remove_effect(effect_pet_allow_distance_15);
}
void monexamine::change_pet_allow_distance( monster &z )
{

    enum choices {
        distance_no_limit = 0,
        distance_1,
        distance_2,
        distance_3,
        distance_5,
        distance_7,
        distance_10,
        distance_15
    };

    uilist amenu;
    std::string pet_name = z.get_name();
    amenu.text = string_format( _( "How much allow distance from master to %s?" ), pet_name );

    amenu.addentry( distance_no_limit, true, '0', _( "no limit" ) );
    amenu.addentry( distance_1, true, '1', _( "1" ) );
    amenu.addentry( distance_2, true, '2', _( "2" ) );
    amenu.addentry( distance_3, true, '3', _( "3" ) );
    amenu.addentry( distance_5, true, '5', _( "5" ) );
    amenu.addentry( distance_7, true, '7', _( "7" ) );
    amenu.addentry( distance_10, true, 't', _( "10" ) );
    amenu.addentry( distance_15, true, 'f', _( "15" ) );
    amenu.query();

    int choice = amenu.ret;
    switch( choice ) {
    case distance_no_limit:
        remove_all_allow_distance_effect( z );
        break;
    case distance_1:
        remove_all_allow_distance_effect( z );
        z.add_effect( effect_pet_allow_distance_1, 1_turns, num_bp, true );
        if( z.type->vision_day <= 1 ){
            popup(_("Your pet seems has not enough vision range, maybe this order does not work."));
        }
        break;
    case distance_2:
        remove_all_allow_distance_effect( z );
        z.add_effect( effect_pet_allow_distance_2, 1_turns, num_bp, true );
        if( z.type->vision_day <= 2 ){
            popup(_("Your pet seems has not enough vision range, maybe this order does not work."));
        }
        break;
    case distance_3:
        remove_all_allow_distance_effect( z );
        z.add_effect( effect_pet_allow_distance_3, 1_turns, num_bp, true );
        if( z.type->vision_day <= 3 ){
            popup(_("Your pet seems has not enough vision range, maybe this order does not work."));
        }
        break;
    case distance_5:
        remove_all_allow_distance_effect( z );
        z.add_effect( effect_pet_allow_distance_5, 1_turns, num_bp, true );
        if( z.type->vision_day <= 5 ){
            popup(_("Your pet seems has not enough vision range, maybe this order does not work."));
        }
        break;
    case distance_7:
        remove_all_allow_distance_effect( z );
        z.add_effect( effect_pet_allow_distance_7, 1_turns, num_bp, true );
        if( z.type->vision_day <= 7 ){
            popup(_("Your pet seems has not enough vision range, maybe this order does not work."));
        }
        break;
    case distance_10:
        remove_all_allow_distance_effect( z );
        if( z.type->vision_day <= 10 ){
            popup(_("Your pet seems has not enough vision range, maybe this order does not work."));
        }
        z.add_effect( effect_pet_allow_distance_10, 1_turns, num_bp, true );
        break;
    case distance_15:
        remove_all_allow_distance_effect( z );
        if( z.type->vision_day <= 15 ){
            popup(_("Your pet seems has not enough vision range, maybe this order does not work."));
        }
        z.add_effect( effect_pet_allow_distance_15, 1_turns, num_bp, true );
        break;
    default:
        return;
    }

}

