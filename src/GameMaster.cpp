
#include "GameMaster.h"

#include <string.h>
#include <stdexcept>

namespace GoBattleSim
{
char err_msg[256];

GameMaster GameMaster::instance;

GameMaster &GameMaster::get()
{
	return instance;
}

GameMaster::GameMaster()
{
	for (int i = 0; i < MAX_NUM_STAGES; ++i)
	{
		m_stage_multipliers[i] = 1.0;
	}
	for (int i = 0; i < MAX_NUM_TYPES; ++i)
	{
		for (int j = 0; j < MAX_NUM_TYPES; ++j)
		{
			m_type_effectiveness[i][j] = 1.0;
		}
	}
}

unsigned GameMaster::num_types(unsigned t_num_types)
{
	if (t_num_types >= MAX_NUM_TYPES)
	{
		sprintf(err_msg, "too many types (%d, max &d)", t_num_types, MAX_NUM_TYPES);
		throw std::runtime_error(err_msg);
	}
	return m_num_types = t_num_types;
}

double GameMaster::effectiveness(unsigned t_type_i, unsigned t_type_j, double t_multiplier)
{
	if (t_type_i < m_num_types && t_type_j < m_num_types)
	{
		m_type_effectiveness[t_type_i][t_type_j] = t_multiplier;
	}
	else if (t_type_i >= m_num_types)
	{
		sprintf(err_msg, "invalid first type index (%d)", t_type_i);
		throw std::runtime_error(err_msg);
	}
	else if (t_type_j >= m_num_types)
	{
		sprintf(err_msg, "invalid second type index (%d)", t_type_j);
		throw std::runtime_error(err_msg);
	}
	return t_multiplier;
}

unsigned GameMaster::num_types() const
{
	return m_num_types;
}

double GameMaster::effectiveness(int t_type_i, int t_type_j) const
{
	if (t_type_i < 0 || t_type_j < 0 || t_type_i > m_num_types || t_type_j > m_num_types)
	{
		return 1;
	}
	else
	{
		return m_type_effectiveness[t_type_i][t_type_j];
	}
}

int GameMaster::boosted_weather(unsigned t_type, int t_weather)
{
	if (0 <= t_type && t_type < m_num_types)
	{
		m_type_boosted_weathers[t_type] = t_weather;
	}
	else
	{
		sprintf(err_msg, "invalid type index (%d)", t_type);
		throw std::runtime_error(err_msg);
	}
	return t_weather;
}

int GameMaster::boosted_weather(int t_type) const
{
	if (0 <= t_type && t_type < m_num_types)
	{
		return m_type_boosted_weathers[t_type];
	}
	else
	{
		return 1;
	}
}

void GameMaster::set_stage_bounds(int t_min_stage, int t_max_stage)
{
	if (t_min_stage > t_max_stage)
	{
		sprintf(err_msg, "min_stage (%d) > max_stage (%d)", t_min_stage, t_max_stage);
		throw std::runtime_error(err_msg);
	}
	auto num_stages = t_max_stage - t_min_stage + 1;
	if (m_num_types > MAX_NUM_STAGES)
	{
		sprintf(err_msg, "too many stages (%d, max %d)", num_stages, MAX_NUM_STAGES);
		throw std::runtime_error(err_msg);
	}
	min_stage = t_min_stage;
	max_stage = t_max_stage;
}

double GameMaster::stage_multiplier(int t_stage, double t_multiplier)
{
	if (min_stage <= t_stage && t_stage <= max_stage)
	{
		m_stage_multipliers[t_stage - min_stage] = t_multiplier;
	}
	else
	{
		sprintf(err_msg, "invalid stage (%d, min %d, max %d)", t_stage, min_stage, max_stage);
		throw std::runtime_error(err_msg);
	}
	return t_multiplier;
}

double GameMaster::stage_multiplier(int t_stage) const
{
	if (t_stage < min_stage || t_stage > max_stage)
	{
		sprintf(err_msg, "invalid stage (%d, min %d, max %d)", t_stage, min_stage, max_stage);
		throw std::runtime_error(err_msg);
	}
	return m_stage_multipliers[t_stage - min_stage];
}

void GameMaster::set_parameter(char *t_name, double t_value)
{
	if (strcmp(t_name, "max_energy") == 0)
		max_energy = t_value;
	else if (strcmp(t_name, "min_stage") == 0)
		min_stage = t_value;
	else if (strcmp(t_name, "max_stage") == 0)
		max_stage = t_value;
	else if (strcmp(t_name, "dodge_duration") == 0)
		dodge_duration = t_value;
	else if (strcmp(t_name, "dodge_window") == 0)
		dodge_window = t_value;
	else if (strcmp(t_name, "swap_duration") == 0)
		swap_duration = t_value;
	else if (strcmp(t_name, "switching_cooldown") == 0)
		switching_cooldown = t_value;
	else if (strcmp(t_name, "rejoin_duration") == 0)
		rejoin_duration = t_value;
	else if (strcmp(t_name, "item_menu_animation_time") == 0)
		item_menu_animation_time = t_value;
	else if (strcmp(t_name, "max_revive_time_per_pokemon") == 0)
		max_revive_time_per_pokemon = t_value;
	else if (strcmp(t_name, "same_type_attack_bonus_multiplier") == 0)
		same_type_attack_bonus_multiplier = t_value;
	else if (strcmp(t_name, "weather_attack_bonus_multiplier") == 0)
		weather_attack_bonus_multiplier = t_value;
	else if (strcmp(t_name, "pvp_fast_attack_bonus_multiplier") == 0)
		pvp_fast_attack_bonus_multiplier = t_value;
	else if (strcmp(t_name, "pvp_charged_attack_bonus_multiplier") == 0)
		pvp_charged_attack_bonus_multiplier = t_value;
	else if (strcmp(t_name, "dodge_damage_reduction_percent") == 0)
		dodge_damage_reduction_percent = t_value;
	else if (strcmp(t_name, "energy_delta_per_health_lost") == 0)
		energy_delta_per_health_lost = t_value;
}

} // namespace GoBattleSim
