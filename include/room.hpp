// room.hpp

/**
 * @file room.hpp
 * @brief Класс Room.
 *
 * Этот класс представляет собой модель комнаты с уникальным идентификатором.
 * Он может быть расширен для включения других свойств и методов, связанных с комнатой.
 */
#ifndef ROOM_HPP
#define ROOM_HPP

#include <vector>

/**
 * @struct InitialParameters
 * @brief Содержит начальные параметры комнаты, связанные с пожарной безопасностью.
 */
struct InitialParameters
{
    double heat_of_combustion;                  // Теплота сгорания.
    double linear_flame_speed_rate;             // Линейная скорость распространения пожара.
    double specific_fuel_burn_rate;             // Удельная скорость выгорания горючей нагрузки.
    double smoke_forming_ability;               // Дымообразующая способность горящего материала.
    double combustion_completeness_coefficient; // Коэффициент полноты сгорания.
    double heat_absorption_coefficient;         // Коэффициент теплопоглощения.
    double start_temperature;                   // Начальная температура.
    double initial_gas_density;                 // Начальная плотность газовой среды в помещении.
    double c_p;                                 // Удельная теплоемкость при постоянном давлении. // TODO: Переименовать для ясности.
    double room_volume;                         // Объем помещения.
};

/**
 * @struct CalculatedParameters
 * @brief Содержит рассчитанные параметры, основанные на начальных параметрах.
 */
struct CalculatedParameters
{
    double A;                                  // Характеристика схемы развития пожара
    double n;                                  // Характеристика схемы развития пожара.
    double gas_release_per_meter_burn;         // Объем газа, выделяющегося при сгорании на единицу площади.
    double limit_gas_density;                  // Предел плотности газовой среды.
    double limit_gas_temperature;              // Предел температуры газовой среды.
    double limit_smoke_extinction_coefficient; // TODO: Переименовать. Предел коэффициента затухания дыма.
    double limit_visibility;                   // TODO: Переименовать. Предел видимости в условиях пожара.
};

/**
 * @struct FireDynamicsParameters
 * @brief Хранит параметры, связанные с динамикой пожара в помещении.
 */
struct FireDynamicsParameters
{
    double burned_mass;                  // Количество выгоревшей массы.
    double gas_density;                  // Плотность газовой среды в помещении.
    double gas_temperature;              // Температура газовой среды в помещении.
    double smoke_extinction_coefficient; // Коэффициент затухания дыма.
    double visibility;                   // Видимость в помещении.
};

/**
 * @class Room
 * @brief Класс, представляющий комнату с параметрами, связанными с пожарной безопасностью.
 *
 * Этот класс моделирует комнату, включая ее идентификацию и различные параметры,
 * связанные с пожарной безопасностью и динамикой пожара.
 */
class Room
{
public:
    Room() = default;
    /**
     * @brief Конструктор класса Room.
     * @param id Уникальный идентификатор комнаты.
     * @param initParams Начальные параметры комнаты.
     * @param isGasSource Флаг, указывающий, является ли комната источником пожара.
     */
    Room(int id, InitialParameters initParams, bool isGasSource = false);

    /**
     * @brief Возвращает уникальный идентификатор комнаты.
     */
    int getId() const;
    /**
     * @brief Устанавливает флаг источника пожара.
     * @param source Флаг, указывающий, является ли комната источником пожара.
     */
    void setIsGasSource(bool source);
    /**
     * @brief Возвращает флаг источника пожара.
     */
    bool getIsGasSource() const;

    /**
     * @brief Возвращает начальные параметры комнаты.
     */
    InitialParameters getInitialParameters() const;

    /**
     * @brief Возвращает рассчитанные параметры комнаты.
     */
    CalculatedParameters getCalculatedParameters() const;

    /**
     * @brief Обновляет параметры динамики пожара.
     * @param params Параметры динамики пожара.
     */
    void addToFireDynamicsHistory(const FireDynamicsParameters &params);

    /**
     * @brief Возвращает историю параметров динамики пожара.
     */
    std::vector<FireDynamicsParameters> getFireDynamicsHistory() const;

private:
    int id_;                                                    // Уникальный идентификатор комнаты. Используется для идентификации объекта Room.
    bool is_gas_source_;                                        // Флаг, указывающий, является ли комната источником газа.
    InitialParameters initial_params_;                          // Начальные параметры комнаты, влияющие на развитие пожара.
    CalculatedParameters calculated_params_;                    // Рассчитанные параметры, основанные на начальных параметрах.
    std::vector<FireDynamicsParameters> fire_dynamics_history_; // История изменений динамических параметров пожара.

    /**
     * Инициализирует рассчитанные параметры на основе начальных.
     */
    CalculatedParameters initializeCalculatedParams();
};

#endif // ROOM_HPP