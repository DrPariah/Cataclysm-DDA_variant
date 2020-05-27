
#include "item_enchant.h"
#include "type_id.h"
#include "itype.h"
#include "rng.h"
#include "options.h"
#include "creature.h"
#include "player.h"
#include "item.h"
#include "messages.h"
#include "map.h"
#include "game.h"
#include "avatar.h"
#include "character.h"
#include "calendar.h"
#include "npc.h"

#include <string>
#include <vector>
#include <map>

namespace
{
std::map<item_enchant_id, item_enchant> item_enchant_map;
std::vector<item_enchant> item_enchant_list;
std::map<item_enchant_type, std::vector<item_enchant>> item_enchant_list_map_by_type;
} // namespace

/** @relates string_id */
template<>
const item_enchant &string_id<item_enchant>::obj() const
{
    const auto iter = item_enchant_map.find( *this );
    if( iter == item_enchant_map.end() ) {
        debugmsg( "invalid enchant id id %s", c_str() );
        static const item_enchant dummy{};
        return dummy;
    }
    return iter->second;
}

item_enchant_type enchant_manager::item_enchant_type_string_to_enum( const std::string type_str ){
    if ( type_str == "enchant_effect_to_target" ) {
        return enchant_effect_to_target;
    } else if ( type_str == "enchant_effect_to_self" ) {
        return enchant_effect_to_self;
    } else if ( type_str == "enchant_fire_gun" ) {
        return enchant_fire_gun;
    } else if ( type_str == "enchant_anti_specie" ) {
        return enchant_anti_specie;
    } else if ( type_str == "enchant_heal_self" ) {
        return enchant_heal_self;
    } else if ( type_str == "enchant_emit_field" ) {
        return enchant_emit_field;
    } else if ( type_str == "enchant_teleport_target" ) {
        return enchant_teleport_target;
    } else {
        debugmsg( "unknown enchant type: %s", type_str );
        return enchant_null;
    }
}

heal_type enchant_manager::enchant_heal_type_string_to_enum( const std::string type_str ){
    if ( type_str == "heal_hp" ) {
        return heal_hp;
    } else if ( type_str == "heal_stamina" ) {
        return heal_stamina;
    } else if ( type_str == "heal_fatigue" ) {
        return heal_fatigue;
    } else if ( type_str == "heal_mana" ) {
        return heal_mana;
    } else {
        debugmsg( "unknown enchant heal type: %s", type_str );
        return heal_null;
    }
}

void load_item_enchant( const JsonObject &jo, const std::string & ){

    item_enchant new_item_enchant;
    new_item_enchant.id = item_enchant_id( jo.get_string( "id" ) );
    new_item_enchant.name = jo.get_string( "name" );
    new_item_enchant.description = jo.get_string( "description" );

    new_item_enchant.enchant_type = enchant_manager::item_enchant_type_string_to_enum( jo.get_string( "enchant_type" ) );

    new_item_enchant.weight_in_natural_spawn = jo.get_int( "weight_in_natural_spawn" , 0);

    new_item_enchant.effect_chance = jo.get_float( "effect_chance" , 1.0);
    new_item_enchant.effect_chance_min = jo.get_float( "effect_chance_min" , new_item_enchant.effect_chance);
    new_item_enchant.effect_chance_max = jo.get_float( "effect_chance_max" , new_item_enchant.effect_chance);

    new_item_enchant.message_on_trigger =  jo.get_string( "message_on_trigger" , "");

    if( new_item_enchant.enchant_type == enchant_effect_to_target ){
        new_item_enchant.effect_type_id_to_apply_target = efftype_id( jo.get_string( "effect_type_id_to_apply_target" ) );
        new_item_enchant.effect_int =  jo.get_int( "effect_int" , 1);
        new_item_enchant.effect_duration_turn =  jo.get_int( "effect_duration_turn" , 10);
    } else if( new_item_enchant.enchant_type == enchant_effect_to_self ){
        new_item_enchant.effect_type_id_to_apply_self = efftype_id( jo.get_string( "effect_type_id_to_apply_self" ) );
        new_item_enchant.effect_int =  jo.get_int( "effect_int" , 1);
        new_item_enchant.effect_duration_turn =  jo.get_int( "effect_duration_turn" , 10);

    } else if( new_item_enchant.enchant_type == enchant_fire_gun ){
        new_item_enchant.gun_type_id_to_fire = itype_id( jo.get_string( "gun_type_id_to_fire" ) );
        new_item_enchant.gun_target_num =  jo.get_int( "gun_target_num" , 1);

        new_item_enchant.gun_max_range = jo.get_int( "gun_max_range" , 10);
        new_item_enchant.gun_fire_mode = gun_mode_id( jo.get_string( "gun_fire_mode", "") );
        new_item_enchant.gun_ammo      = itype_id( jo.get_string( "gun_ammo", "") );
        new_item_enchant.message_on_fire_gun = jo.get_string( "message_on_fire_gun", "" );

    } else if( new_item_enchant.enchant_type == enchant_anti_specie ){
        new_item_enchant.specie_id_to_anti = species_id( jo.get_string( "specie_id_to_anti" ) );
        new_item_enchant.anti_specie_multiplier =  jo.get_float( "anti_specie_multiplier" , 2.0);

    } else if( new_item_enchant.enchant_type == enchant_emit_field ){
        new_item_enchant.emit_id_to_emit = emit_id( jo.get_string( "emit_id_to_emit" ) );
    } else if( new_item_enchant.enchant_type == enchant_heal_self ) {
        new_item_enchant.heal_type_enchant =  enchant_manager::enchant_heal_type_string_to_enum( jo.get_string( "heal_type" ) );
        new_item_enchant.heal_amount = jo.get_int( "heal_amount" );
    }

    item_enchant_map[new_item_enchant.id] = new_item_enchant;
    item_enchant_list.push_back( new_item_enchant );
    item_enchant_list_map_by_type[ new_item_enchant.enchant_type ].push_back( new_item_enchant );
}

static item_enchant get_random_enchant_with_weight( const std::vector<item_enchant> &enchant_list) {

    int weight_sum = 0;

    for( item_enchant enchant : enchant_list ){
        weight_sum += enchant.weight_in_natural_spawn;
    }

    int weight_iterating_sum = rng( 0, weight_sum );
    for( item_enchant enchant : enchant_list ){
        weight_iterating_sum -= enchant.weight_in_natural_spawn;
        if( weight_iterating_sum <= 0 ) {
            add_msg( m_debug, "choosen enchant: %s", enchant.name );
            return enchant;
        }
    }
    debugmsg( "choosen enchant weight is out bound! check weight_in_natural_spawn at item_enchant json" );
    return item_enchant();
}

/**
 * return random whole enchant data at natural generation
 */
item_enchant enchant_manager::generate_natual_enchant() {

    item_enchant new_item_enchant;

    int weight_effect_self     = get_option<int>("ENCHANT_WEIGHT_EFFECT_SELF");
    int weight_effect_target   = get_option<int>("ENCHANT_WEIGHT_EFFECT_TARGET");
    int weight_anti_specie     = get_option<int>("ENCHANT_WEIGHT_EFFECT_ANTI_SPECIE");
    int weight_fire_gun        = get_option<int>("ENCHANT_WEIGHT_EFFECT_FIRE_GUN");
    int weight_emit_field      = get_option<int>("ENCHANT_WEIGHT_EFFECT_EMIT_FIELD");
    int weight_heal_self       = get_option<int>("ENCHANT_WEIGHT_EFFECT_HEAL_SELF");
    // int weight_teleport_target = get_option<int>("ENCHANT_WEIGHT_EFFECT_TELEPORT_TARGET");

    int weight_sum =
        weight_effect_self      +
        weight_effect_target    +
        weight_anti_specie      +
        weight_fire_gun         +
        weight_emit_field       +
        weight_heal_self        +
    //    weight_teleport_target  +
        0;

    int weight_iterating_sum = rng( 0, weight_sum );

    if( ( weight_iterating_sum -= weight_effect_self) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_effect_to_self ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_effect_target ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_effect_to_target ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_anti_specie ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_anti_specie ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_fire_gun ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_fire_gun ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_emit_field ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_emit_field ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else if( ( weight_iterating_sum -= weight_heal_self ) <= 0 ) {
        new_item_enchant = get_random_enchant_with_weight( item_enchant_list_map_by_type[ enchant_heal_self ] );

        if( new_item_enchant.effect_chance_min != new_item_enchant.effect_chance_max ) {
            new_item_enchant.effect_chance = rng_float( new_item_enchant.effect_chance_min, new_item_enchant.effect_chance_max );
        }

    } else {
        debugmsg( "choosen enchant weight is out bound! check external option named ENCHANT_WEIGHT_EFFECT_xxx" );
    }

    return new_item_enchant;

}

void enchant_manager::invoke_damage_modifier_enchantment(damage_instance &dmg,
        item_enchant &enchant, Creature &target, item &weapon, player& user) {
    if( enchant.enchant_type == enchant_anti_specie ) {
        if ( rng_float( 0.0, 1.0 ) < enchant.effect_chance ) {
            if( target.in_species( enchant.specie_id_to_anti ) ) {
                add_msg( m_debug, _("Bash: %.2f, Cut: %.2f, Stub: %.2f"),
                        dmg.type_damage( DT_BASH ), dmg.type_damage( DT_CUT ), dmg.type_damage( DT_STAB ));
                add_msg( m_debug, _("anti_specie_multiplier: %f"), enchant.anti_specie_multiplier );

                dmg.mult_damage( enchant.anti_specie_multiplier );

                add_msg( m_debug, _("Bash: %.2f, Cut: %.2f, Stub: %.2f"),
                        dmg.type_damage( DT_BASH ), dmg.type_damage( DT_CUT ), dmg.type_damage( DT_STAB ));

                if( enchant.message_on_trigger == ""){
                    if ( 2.0 < enchant.anti_specie_multiplier ) {
                        add_msg(_("%s is super effective to %s!"), weapon.tname(), target.get_name());
                    } else if ( 1.0 < enchant.anti_specie_multiplier ) {
                        add_msg(_("%s is very effective to %s!"), weapon.tname(), target.get_name());
                    } else {
                        add_msg(_("%s is not effective to %s..."), weapon.tname(), target.get_name());
                    }

                } else {
                    add_msg( enchant.message_on_trigger, weapon.tname(), target.get_name(), user.get_name() );
                }
            }
        }
    }
}

static void effect_fire_gun( item_enchant& enchant, Creature& target,
        item &weapon, player& user )
{
    int max_range = enchant.gun_max_range;
    int NO_USE_VARIABLE = 0;

    if( !user.auto_find_hostile_target( max_range, NO_USE_VARIABLE) ) {
        return;
    }

    item gun( enchant.gun_type_id_to_fire );
    if( enchant.gun_fire_mode == gun_mode_id("") ){
        gun.gun_set_mode( enchant.gun_fire_mode );
    }
    if( enchant.gun_ammo == "" && enchant.gun_ammo == "NULL" && enchant.gun_ammo == "null" ){
        gun.ammo_set( enchant.gun_ammo, 101 );
    } else if ( gun.ammo_default() != "" && gun.ammo_default() != "NULL" && gun.ammo_default() != "null" ) {
        gun.ammo_set( gun.ammo_default(), 101 );
    }

    if( !gun.ammo_sufficient() ) {
        add_msg( "%s is out of ammo, but this message is not show at normally. It's probably bug.", weapon.tname() );
        return;
    }

    standard_npc tmp( _( "The " ) + weapon.tname(), user.pos(), {}, 8,
                      12, 12, 12, 12 );
    tmp.set_fake( true );
    tmp.set_attitude( NPCATT_FOLLOW );
    tmp.recoil = 0; // no need to aim

    tmp.weapon = gun;
    tmp.i_add( item( "UPS_off", calendar::turn, 1000 ) );

    tmp.fire_gun( target.pos(), gun.gun_current_mode().qty );

    add_msg( _( enchant.message_on_fire_gun ), weapon.tname(), target.get_name(), user.get_name() );

}

void enchant_manager::invoke_enchantment_effect(item_enchant& enchant, Creature& target,
        item &weapon, player& user) {

    if ( rng_float( 0.0, 1.0 ) < enchant.effect_chance ) {

        if( enchant.enchant_type == enchant_effect_to_target ) {
            efftype_id efid = efftype_id( enchant.effect_type_id_to_apply_target);
            time_duration timed = time_duration::from_turns(enchant.effect_duration_turn) ;
            int intencity = enchant.effect_int;
            target.add_effect( efid, timed , num_bp, false, intencity, false, false);
            add_msg( enchant.message_on_trigger,  weapon.tname(), target.get_name(), user.get_name());

        } else if ( enchant.enchant_type == enchant_effect_to_self ) {
            efftype_id efid = efftype_id( enchant.effect_type_id_to_apply_self);
            time_duration timed = time_duration::from_turns(enchant.effect_duration_turn) ;
            int intencity = enchant.effect_int;
//            user.add_effect( efid, timed , num_bp, false, intencity, false, true);
            user.add_effect( efid, timed , num_bp, false, intencity, false, false);
            add_msg( enchant.message_on_trigger,  weapon.tname(), target.get_name(), user.get_name());

        } else if ( enchant.enchant_type == enchant_anti_specie ) {
            // anti specie is in damage modifire enchantment.

        } else if ( enchant.enchant_type == enchant_fire_gun ) {
            effect_fire_gun(enchant, target, weapon, user );
            for ( int i=0 ; enchant.gun_target_num < i ; i++) {
                add_msg( enchant.message_on_trigger,  weapon.tname(), target.get_name(), user.get_name());
            }
        } else if ( enchant.enchant_type == enchant_emit_field ) {
            g->m.emit_field( target.pos(), emit_id( enchant.emit_id_to_emit ) , 1.0f );
            add_msg( enchant.message_on_trigger,  weapon.tname(), target.get_name(), user.get_name());

        } else if ( enchant.enchant_type == enchant_heal_self ) {
            if( enchant.heal_type_enchant == heal_hp ) {
                user.healall( enchant.heal_amount );
            } else if( enchant.heal_type_enchant == heal_stamina ) {
                user.mod_stamina( enchant.heal_amount );
            } else if( enchant.heal_type_enchant == heal_fatigue ) {
                user.mod_fatigue( enchant.heal_amount );
            } else if( enchant.heal_type_enchant == heal_mana ) {
                user.magic.mod_mana( user, enchant.heal_amount );
            } else {
                debugmsg("unknown heal type: %d %s", enchant.heal_type_enchant,  weapon.tname());
            }
            add_msg( enchant.message_on_trigger,  weapon.tname(), target.get_name(), user.get_name());
        } else if ( enchant.enchant_type == enchant_teleport_target ) {
            add_msg("teleport is not inplemented yet, sorry!");
            add_msg( enchant.message_on_trigger,  weapon.tname(), target.get_name(), user.get_name());

        }
    }

}


void enchant_manager::add_random_enchant_to_item( item& it ){

    // make enchant

    if( 0 < get_option<int>( "ENCHANT_RATE_TO_NATURAL_ITEM_SPAWN" ) ) {
        add_msg( m_debug, "try enchant to %s", it.tname());

        if ( it.is_melee( DT_CUT ) || it.is_melee( DT_STAB )  || it.is_melee( DT_BASH ) ){
            if ( rng(0, 100) <= get_option<int>( "ENCHANT_RATE_TO_NATURAL_ITEM_SPAWN" ) ) {
                // apply enchant
                it.item_enchant_list.push_back( enchant_manager::generate_natual_enchant() );
                // FUCKIN HARDCODED!!!
                if( rng(0, 100) <= 50 ) {
                    it.item_enchant_list.push_back( enchant_manager::generate_natual_enchant() );
                }
                if( rng(0, 100) <= 25 ) {
                    it.item_enchant_list.push_back( enchant_manager::generate_natual_enchant() );
                }
                if( rng(0, 100) <= 10 ) {
                    it.item_enchant_list.push_back( enchant_manager::generate_natual_enchant() );
                }
                if( rng(0, 100) <= 5 ) {
                    it.item_enchant_list.push_back( enchant_manager::generate_natual_enchant() );
                }
            } else {

            }
        } else {
            add_msg( m_debug, "%s is not have enough melee damage.", it.tname());
        }
        add_msg( m_debug, "current enchant list");
        for(item_enchant enc : it.item_enchant_list){
            add_msg( m_debug,  enc.name );
        }
    }
}

void enchant_manager::add_random_enchant_to_item( std::vector<item> &it_list ){
    add_msg( m_debug,  "called item vector");

    for( item &it : it_list ) {
        enchant_manager::add_random_enchant_to_item( it );
    }
}
void enchant_manager::add_random_enchant_to_item( std::list<item> &it_list ){
    add_msg( m_debug, "called item list");

    for( item &it : it_list ) {
        enchant_manager::add_random_enchant_to_item( it );
    }
}

void item_enchant::serialize(JsonOut &json) const {
    json.start_object();

    json.member("id" , id );
    json.member("effect_chance" , effect_chance );

    json.end_object();
}
void item_enchant::deserialize(const JsonObject &jo) {

    id = item_enchant_id( jo.get_string( "id" ) );

    item_enchant enchant_definition = id.obj();

    if( enchant_definition.enchant_type == enchant_null ) {
        // no enchant is not allowed
        debugmsg( "enchant type is enchant_null :" , id.str() );
        return;
    }

    name = enchant_definition.name;
    description = enchant_definition.description;

    enchant_type = enchant_definition.enchant_type;

    // effect_chance = enchant_definition.effect_chance;

    effect_type_id_to_apply_target = enchant_definition.effect_type_id_to_apply_target;
    effect_type_id_to_apply_self = enchant_definition.effect_type_id_to_apply_self;
    effect_int = enchant_definition.effect_int;
    effect_duration_turn = enchant_definition.effect_duration_turn;

    gun_type_id_to_fire = enchant_definition.gun_type_id_to_fire;
    gun_target_num = enchant_definition.gun_target_num;
    gun_max_range = enchant_definition.gun_max_range;
    gun_fire_mode = enchant_definition.gun_fire_mode;
    gun_ammo = enchant_definition.gun_ammo;
    message_on_fire_gun = enchant_definition.message_on_fire_gun;

    specie_id_to_anti = enchant_definition.specie_id_to_anti;
    anti_specie_multiplier = enchant_definition.anti_specie_multiplier;

    emit_id_to_emit = enchant_definition.emit_id_to_emit;


    message_on_trigger = enchant_definition.message_on_trigger;

    // saved datas
    effect_chance = jo.get_float( "effect_chance" );

}
