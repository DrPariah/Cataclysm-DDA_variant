#pragma once
#ifndef ITEM_ENCHANT_H
#define ITEM_ENCHANT_H

#include <string>

#include "type_id.h"
#include "itype.h"

enum item_enchant_type{
    enchant_null = 0,
    enchant_effect_to_target,
    enchant_effect_to_self,
    enchant_anti_specie,
    enchant_fire_gun,
    enchant_emit_field,
    enchant_heal_self,
    enchant_teleport_target,

};

struct item_enchant {
    item_enchant_id id;
    std::string name;
    std::string description;

    item_enchant_type enchant_type = enchant_null;

    int weight_in_natural_spawn;
    float effect_chance;
    float effect_chance_min;
    float effect_chance_max;

    efftype_id effect_type_id_to_apply_target;
    efftype_id effect_type_id_to_apply_self;
    int effect_int;
    int effect_duration_turn;

    itype_id gun_type_id_to_fire;
    int gun_target_num;

    species_id specie_id_to_anti;
    float anti_specie_multiplier;

    emit_id emit_id_to_emit;

    std::string apply_message;

};


class enchant_manager {

    public:
        static item_enchant_type item_enchant_type_string_to_enum( const std::string );

        static void load_enchant_balance_options();

        static item_enchant generate_natual_enchant();

        static void invoke_damage_modifier_enchantment( damage_instance dmg, item_enchant&, Creature&, item&, player& );

        static void invoke_enchantment_effect( item_enchant&, Creature&, item&, player& );
        // enchant balance things.
        // load by load_enchant_balance_options()
        // from external options
        // external optionfiles are
        // data/mods/variant_addition/enchant/*.json
};

void load_item_enchant( const JsonObject &jo, const std::string &src );


#endif /* SRC_ITEM_ENCHANT_H_ */
