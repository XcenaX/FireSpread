// room.cpp
#include "room.hpp"

Room::Room(int id, InitialParameters initParams, bool isGasSource)
    : id_(id), initial_params_(initParams), is_gas_source_(isGasSource), calculated_params_(initializeCalculatedParams()) {}

int Room::getId() const
{
    return id_;
}

void Room::setIsGasSource(bool isGasSource)
{
    is_gas_source_ = isGasSource;
}

bool Room::getIsGasSource() const
{
    return is_gas_source_;
}

InitialParameters Room::getInitialParameters() const
{
    return initial_params_;
}

CalculatedParameters Room::getCalculatedParameters() const
{
    return calculated_params_;
}

void Room::addToFireDynamicsHistory(const FireDynamicsParameters &params)
{
    fire_dynamics_history_.push_back(params);
}

std::vector<FireDynamicsParameters> Room::getFireDynamicsHistory() const
{
    return fire_dynamics_history_;
}

CalculatedParameters Room::initializeCalculatedParams()
{
    CalculatedParameters params;
    params.A = 1.05 * initial_params_.specific_fuel_burn_rate * pow(initial_params_.linear_flame_speed_rate, 2);
    params.n = 3;
    params.gas_release_per_meter_burn = (initial_params_.combustion_completeness_coefficient * initial_params_.heat_of_combustion * (1 - initial_params_.heat_absorption_coefficient)) / (initial_params_.start_temperature * initial_params_.initial_gas_density * initial_params_.c_p);
    params.limit_gas_density = 1 / params.gas_release_per_meter_burn;
    params.limit_gas_temperature = initial_params_.start_temperature * initial_params_.initial_gas_density / params.limit_gas_density;
    params.limit_smoke_extinction_coefficient = initial_params_.smoke_forming_ability / params.gas_release_per_meter_burn;
    params.limit_visibility = 2.38;
    return params;
}