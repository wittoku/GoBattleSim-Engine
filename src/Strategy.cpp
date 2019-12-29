
#include "Strategy.h"

namespace GoBattleSim
{

void PokemonState::init()
{
	heal();
	active = false;
	hp = max_hp;
	energy = 0;
	damage_reduction_expiry = 0;
	tdo = 0;
	tdo_fast = 0;
	duration = 0;
	num_deaths = 0;
	num_fmoves_used = 0;
	num_cmoves_used = 0;
}

void PokemonState::heal()
{
	hp = max_hp;
	energy = 0;
}

bool PokemonState::is_alive() const
{
	return hp > 0 || immortal;
}

void PokemonState::charge(int t_energy_delta)
{
	energy += t_energy_delta;
	if (energy > GameMaster::max_energy)
	{
		energy = GameMaster::max_energy;
	}
}

void PokemonState::hurt(int t_damage)
{
	hp -= t_damage;
}

void PokemonState::attribute_damage(int t_damage, bool t_is_fmove)
{
	tdo += t_damage;
	if (t_is_fmove)
	{
		tdo_fast += t_damage;
	}
}

Strategy::Strategy(EventResponder t_on_free, EventResponder t_on_clear, EventResponder t_on_attack)
	: on_free(t_on_free ? t_on_free : attacker_no_dodge_on_free), on_clear(t_on_clear), on_attack(t_on_attack)
{
}

// Helper function
int get_projected_energy(const StrategyInput &si)
{
	int projected_energy = si.subject_state->energy;
	if (si.subject_action.type == ActionType::Fast)
	{
		projected_energy += si.subject->get_fmove(0)->energy;
	}
	else if (si.subject_action.type == ActionType::Charged)
	{
		projected_energy += si.subject->get_cmove(-1)->energy;
	}
	return projected_energy;
}

void defender_on_clear(const StrategyInput &si, Action *r_action)
{
	r_action->type = ActionType::Fast;
	r_action->value = 0;
	int projected_energy = get_projected_energy(si);
	for (int i = 0; i < si.subject->cmoves_count; ++i)
	{
		auto cmove = si.subject->get_cmove(i);
		if (projected_energy + cmove->energy >= 0 && ((si.random_number >> i) & 1))
		{
			r_action->type = ActionType::Charged;
			r_action->value = i;
			break;
		}
	}
}

void attacker_no_dodge_on_free(const StrategyInput &si, Action *r_action)
{
	if (si.subject_state->energy + si.subject->get_cmove(-1)->energy >= 0)
	{
		r_action->type = ActionType::Charged;
	}
	else
	{
		r_action->type = ActionType::Fast;
	}
}

void attacker_dodge_charged_on_free(const StrategyInput &si, Action *r_action)
{
	bool predicted_attack = false;
	int time_of_damage = -1, time_of_enemy_cooldown = -1;
	if (si.enemy_action.type == ActionType::Fast)
	{
		time_of_enemy_cooldown = si.enemy_action.time + si.enemy->get_fmove(0)->duration;
	}
	else if (si.enemy_action.type == ActionType::Charged)
	{
		time_of_damage = si.enemy_action.time + si.enemy->cmove->dws;
		time_of_enemy_cooldown = si.enemy_action.time + si.enemy->cmove->duration;
	}
	if (time_of_damage < si.time_free || time_of_damage < si.subject_state->damage_reduction_expiry)
	{
		// Predict next time of damage
		time_of_damage = time_of_enemy_cooldown + 1500 + si.enemy->cmove->dws;
		predicted_attack = true;
	}
	int time_till_damage = time_of_damage - si.time_free;
	if (time_till_damage > si.subject->cmove->duration && si.subject_state->energy + si.subject->cmove->energy >= 0) // Can squeeze in one charge
	{
		r_action->type = ActionType::Charged;
		r_action->value = -1;
	}
	else if (time_till_damage > si.subject->get_fmove(0)->duration) // Can squeeze in one fast
	{
		r_action->type = ActionType::Fast;
	}
	else
	{
		if (predicted_attack)
		{
			r_action->type = ActionType::Wait;
		}
		else
		{
			int delay = time_till_damage - GameMaster::dodge_window;
			r_action->type = ActionType::Dodge;
			r_action->delay = delay > 0 ? delay : 0;
		}
	}
}

void attacker_dodge_charged_on_attack(const StrategyInput &si, Action *r_action)
{
	int time_of_damage;
	if (si.enemy_action.type == ActionType::Fast)
	{
		// Ignore the coming enemy fast attack
		attacker_no_dodge_on_free(si, r_action);
		return;
	}
	else
	{
		time_of_damage = si.enemy_action.time + si.enemy->cmove->dws;
	}
	int time_till_damage = time_of_damage - si.time_free;
	if (time_till_damage > si.subject->cmove->duration && si.subject_state->energy + si.subject->cmove->energy >= 0) // Can squeeze in one charge
	{
		r_action->type = ActionType::Charged;
		r_action->value = -1;
	}
	else if (time_till_damage > si.subject->get_fmove(0)->duration) // Can squeeze in one fast
	{
		r_action->type = ActionType::Fast;
	}
	else // Just dodge
	{
		int delay = time_till_damage - GameMaster::dodge_window;
		r_action->type = ActionType::Dodge;
		r_action->delay = delay > 0 ? delay : 0;
	}
}

void attacker_dodge_all_on_free(const StrategyInput &si, Action *r_action)
{
	bool predicted_attack = false;
	int time_of_damage = -1, time_of_enemy_cooldown = -1;
	if (si.enemy_action.type == ActionType::Fast)
	{
		time_of_damage = si.enemy_action.time + si.enemy->get_fmove(0)->dws;
		time_of_enemy_cooldown = si.enemy_action.time + si.enemy->get_fmove(0)->duration;
	}
	else if (si.enemy_action.type == ActionType::Charged)
	{
		time_of_damage = si.enemy_action.time + si.enemy->cmove->dws;
		time_of_enemy_cooldown = si.enemy_action.time + si.enemy->cmove->duration;
	}
	if (time_of_damage < si.time_free || time_of_damage < si.subject_state->damage_reduction_expiry)
	{
		// Predict next time of damage
		time_of_damage = time_of_enemy_cooldown + 1500 + si.enemy->get_fmove(0)->dws;
		predicted_attack = true;
	}
	int time_till_damage = time_of_damage - si.time_free;
	if (time_till_damage > si.subject->cmove->duration && si.subject_state->energy + si.subject->cmove->energy >= 0) // Can squeeze in one charge
	{
		r_action->type = ActionType::Charged;
		r_action->value = -1;
	}
	else if (time_till_damage > si.subject->get_fmove(0)->duration) // Can squeeze in one fast
	{
		r_action->type = ActionType::Fast;
	}
	else
	{
		if (predicted_attack)
		{
			r_action->type = ActionType::Wait;
		}
		else
		{
			int delay = time_till_damage - GameMaster::dodge_window;
			r_action->type = ActionType::Dodge;
			r_action->delay = delay > 0 ? delay : 0;
		}
	}
}

void attacker_dodge_all_on_attack(const StrategyInput &si, Action *r_action)
{
	int time_of_damage;
	if (si.enemy_action.type == ActionType::Fast)
	{
		time_of_damage = si.enemy_action.time + si.enemy->get_fmove(0)->dws;
	}
	else
	{
		time_of_damage = si.enemy_action.time + si.enemy->cmove->dws;
	}
	int time_till_damage = time_of_damage - si.time_free;
	if (time_till_damage > si.subject->cmove->duration && si.subject_state->energy + si.subject->cmove->energy >= 0) // Can squeeze in one charge
	{
		r_action->type = ActionType::Charged;
		r_action->value = -1;
	}
	else if (time_till_damage > si.subject->get_fmove(0)->duration) // Can squeeze in one fast
	{
		r_action->type = ActionType::Fast;
	}
	else // Just dodge
	{
		int delay = time_till_damage - GameMaster::dodge_window;
		r_action->type = ActionType::Dodge;
		r_action->delay = delay > 0 ? delay : 0;
	}
}

// int damage_if_dodged = (1 - GameMaster::dodge_damage_reduction_percent) * calc_damage(si.battle_p, si.enemy, enemy_move, subject);

} // namespace GoBattleSim
