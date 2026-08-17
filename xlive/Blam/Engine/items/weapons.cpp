#include "stdafx.h"
#include "weapons.h"

#include "weapon_definitions.h"

#include "cache/cache_files.h"
#include "game/game_time.h"
#include "objects/damage_reporting.h"
#include "shell/shell.h"
#include "units/units.h"

#include "H2MOD/Modules/CustomVariantSettings/CustomVariantSettings.h"

/* constants */

// unit_handle_message_from_weapon arms the timer with game_seconds_to_ticks_round(0.3f).
#define k_dual_wield_fire_timer_seconds 0.3f

/* prototypes */

static void weapon_barrel_idle(int32 weapon_index, int16 barrel_index);

static __declspec(naked) void weapon_barrel_idle_usercall_to_rewritten(void)
{
    __asm
    {
        push ebx

        mov ebx, [esp + 0x4 + 0x4]

        push ebx
        push eax

        call weapon_barrel_idle

        add esp, 2 * 4

        pop ebx

        retn
    }
}

/* public code */

void weapons_apply_patches(void)
{
    if (!shell_is_dedicated_server())
    {
        PatchCall(Memory::GetAddress(0x15C60C), weapon_barrel_idle_usercall_to_rewritten);
        PatchCall(Memory::GetAddress(0x1611AD), weapon_barrel_idle_usercall_to_rewritten);
        PatchCall(Memory::GetAddress(0x162B38), weapon_barrel_idle_usercall_to_rewritten);
    }

    // Replace call to "deterministic" version so we don't desync in synchronous networking
    PatchCall(Memory::GetAddress(0x160534, 0x13BE4E), weapon_send_message_to_unit_deterministic);

    return;
}

void __cdecl weapon_send_message_to_unit(
    int32 weapon_index,
    e_unit_messages unit_message)
{
    INVOKE(0x15A88E, 0x13EB4E, weapon_send_message_to_unit, weapon_index, unit_message);
    return;
}

void __cdecl weapon_send_message_to_unit_deterministic(
    int32 weapon_index,
    e_unit_messages unit_message)
{
    // Messages 1-4 are the "barrel fired" family that retail uses to arm the dual-wield timers.
    if (IN_RANGE(unit_message, _unit_message_weapon_primary_fire, _unit_message_weapon_secondary_misfire))
    {
        weapon_datum const* weapon = weapon_get(weapon_index);
        const datum unit_index = weapon->item.inventory_owner_unit_index;

        if (unit_index != NONE)
        {
            unit_datum* unit = unit_get(unit_index);
            const int8 fire_timer_ticks = (int8)game_seconds_to_ticks_round(k_dual_wield_fire_timer_seconds);

            // Retail selects the byte by testing the firing weapon against the unit's current slot:
            // the held/primary weapon arms primary_fire_timer, the off-hand arms secondary.
            if (unit_inventory_get_weapon(unit_index, unit->unit.weapon_index) != weapon_index)
            {
                unit->unit.secondary_fire_timer = fire_timer_ticks;
            }
            else
            {
                unit->unit.primary_fire_timer = fire_timer_ticks;
            }
        }
    }

    weapon_send_message_to_unit(weapon_index, unit_message);
    return;
}


int32 __cdecl weapon_get_rounds_total(datum object_index, int32 magazine_index, bool a3)
{
	return INVOKE(0x15F313, 0x1435D3, weapon_get_rounds_total, object_index, magazine_index, a3);
}

void __cdecl weapons_fire_barrels(void)
{
	INVOKE(0x160AB7, 0x144D77, weapons_fire_barrels);
	return;
}

/* private code */

static void weapon_barrel_idle(
    int32 weapon_index,
    int16 barrel_index)
{
    weapon_datum* weapon = weapon_get(weapon_index);
    struct weapon_definition const* weapon_definition = (struct weapon_definition const*)tag_get_fast(weapon->definition_index);

    ASSERT(weapon);
    ASSERT(weapon_definition);

    weapon_barrel* weapon_barrel = &weapon->weapon.barrels[barrel_index];
    weapon_barrel_definition const* barrel_definition = TAG_BLOCK_GET_ELEMENT(&weapon_definition->weapon.barrels, barrel_index, weapon_barrel_definition);

    weapon_barrel->firing_idle_ticks = 0;
    weapon_barrel->fire_count = 0;
    weapon_barrel->state = _weapon_barrel_state_idle;

    bool dub_shot_test = barrel_definition->damage_effect_reporting_type == _damage_reporting_type_battle_rifle && currentVariantSettings.disable_dub_shot;

    if (!barrel_definition->flags.test(_weapon_barrel_definition_dont_clear_fire_bit_after_recovering) || dub_shot_test)
    {
        weapon_barrel->flags.set(_weapon_barrel_fire_bit, false);
    }

    return;
}
