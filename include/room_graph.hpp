// room_graph.hpp

/**
 * @file room_graph.hpp
 * @brief Класс RoomGraph.
 *
 * Этот класс представляет собой граф комнат, где каждая комната является узлом в графе,
 * а связи между комнатами представляют собой ребра.
 */

// room_graph.hpp

#ifndef ROOMGRAPH_HPP
#define ROOMGRAPH_HPP

#include "room.hpp"
#include <vector>
#include <map>
#include <limits>
#include <stdexcept>
#include <stack>
#include <queue>

/**
 * @class RoomGraph
 * @brief Класс, представляющий граф комнат.
 *
 * Этот класс представляет собой граф комнат, где каждая комната является узлом в графе,
 * а связи между комнатами представляют собой ребра, представляющие пути распространения пожара.
 */
class RoomGraph
{
public:
    /**
     * @brief Добавляет комнату в граф.
     * @attention Рекомендуется добавлять комнату-источник (очаг пожара) первой.
     * @param room Объект Room, который нужно добавить в граф.
     */
    void addRoom(const Room &room);

    /**
     * @brief Возвращает карту всех комнат в графе.
     * @return Карта, где ключ - это ID комнаты, а значение - объект Room.
     */
    const std::map<int, Room> &getRooms() const;

    /**
     * @brief Добавляет связь между двумя комнатами в граф.
     * @param fromRoomId Идентификатор первой комнаты.
     * @param toRoomId Идентификатор второй комнаты.
     * @param connectionStrength Сила связи между комнатами.
     */
    void addConnection(int fromRoomId, int toRoomId, double connectionStrength);

    /**
     * @brief Расчет динамических параметров пожара для каждой комнаты до заданного момента времени.
     * @param timeInSeconds Время в секундах, до которого нужно произвести расчет.
     * @param recursionDepthLimit Ограничение глубины рекурсии для предотвращения бесконечных циклов.
     * @param changeThreshold Порог изменения параметров, при котором расчет считается значимым.
     */
    void calculateFireDynamicsUpToTime(double timeInSeconds, int recursionDepthLimit, double changeThreshold, double timeStep);

private:
    std::map<int, Room> rooms;                                                ///< Словарь, отображающий идентификаторы комнат на объекты Room.
    std::map<int, std::vector<std::pair<int, double>>> outgoingAdjacencyList; ///< Словарь для хранения исходящих связей.
    std::map<int, std::vector<std::pair<int, double>>> incomingAdjacencyList; ///< Словарь для хранения входящих связей.

    /**
     * @brief Вспомогательная функция для топологической сортировки графа.
     *
     * Эта функция рекурсивно посещает вершины графа, начиная с заданной вершины,
     * и сохраняет порядок их обработки в стек.
     *
     * @param v Идентификатор текущей комнаты, которая обрабатывается.
     * @param visited Словарь для отслеживания посещенных комнат.
     * @param Stack Стек, в который добавляются вершины в порядке их обработки.
     */
    void topologicalSortUtil(int v, std::map<int, bool> &visited, std::stack<int> &Stack);

    /**
     * @brief Основная функция для топологической сортировки графа комнат.
     *
     * Эта функция использует вспомогательную функцию topologicalSortUtil для создания
     * линейного порядка вершин, что необходимо для правильной обработки комнат.
     * Результат сортировки помещается в очередь, которая затем используется для итеративной обработки комнат.
     *
     * @param processingQueue Очередь, в которую будут помещены идентификаторы комнат в отсортированном порядке.
     */
    void topologicalSort(std::queue<int> &processingQueue);

    int findSourceRoomId() const;

    void initializeFireDynamicsForAllRooms(int sourceRoomId);

    /**
     * @brief Рекурсивно рассчитывает динамические параметры пожара для заданной комнаты и ее смежных комнат.
     *
     * Этот метод используется для расчета и обновления динамических параметров пожара в указанной комнате и ее смежных комнатах.
     * Расчет основан на текущем времени, параметрах комнаты-источника (очага пожара), параметрах смежных комнат и предыдущих значениях параметров.
     * Метод применяет итеративный подход для обхода смежных комнат и расчета их параметров.
     *
     * @param sourceRoomId Идентификатор комнаты-источника (очага пожара).
     * @param roomId Идентификатор комнаты, для которой необходимо рассчитать параметры.
     * @param currentTime Текущее время в секундах, для которого производится расчет.
     * @param recursionDepth Текущая глубина рекурсии.
     * @param recursionDepthLimit Максимально допустимая глубина рекурсии.
     * @param changeThreshold Порог значимых изменений, при превышении которого продолжается расчет.
     * @param lastCalculatedParameters Ссылка на map, хранящий последние рассчитанные параметры для каждой комнаты.
     */
    void calculateRoomFireDynamics(int sourceRoomId, int roomId, double currentTime, int recursionDepth, int recursionDepthLimit, double changeThreshold);

    /**
     * @brief Вычисляет динамические параметры пожара для указанной комнаты, учитывая ее связи с соседними комнатами.
     *
     * Этот метод рассчитывает динамические параметры пожара для указанной комнаты. Вычисления
     * основаны на текущих параметрах комнаты, связанных с пожарной безопасностью, а также на
     * параметрах смежных комнат. Метод может использоваться как для исходной комнаты (очага пожара),
     * так и для смежных комнат.
     *
     * @param sourceRoomId Идентификатор комнаты-источника (очага пожара), параметры которой используются в расчетах.
     * @param room Комната, для которой производится расчет.
     * @param currentTime Текущее время в секундах, для которого производится расчет.
     * @param lastCalculatedParameters Словарь, содержащий последние рассчитанные параметры для каждой комнаты.
     * @param hasConnections Флаг, указывающий, следует ли учитывать связи с соседними комнатами в расчетах.
     * @return Структура FireDynamicsParameters, содержащая рассчитанные динамические параметры пожара для комнаты.
     */
    FireDynamicsParameters calculateFireDynamicsForRoom(int sourceRoomId, Room &room, double currentTime, bool hasConnections);

    /**
     * @brief Определяет, превышают ли изменения между старыми и новыми параметрами динамики пожара заданный порог.
     *
     * Этот метод сравнивает старые и новые параметры динамики пожара в комнате и определяет,
     * являются ли различия между ними достаточно значимыми, исходя из заданного порога изменений.
     * Если различия превышают порог, это указывает на необходимость обновления и дальнейшего расчета параметров.
     *
     * @param oldParams Структура `FireDynamicsParameters`, содержащая старые параметры динамики пожара.
     * @param newParams Структура `FireDynamicsParameters`, содержащая новые параметры динамики пожара.
     * @param changeThreshold Порог значимых изменений, используемый для определения необходимости обновления параметров.
     * @return true, если различия между старыми и новыми параметрами превышают порог изменений, иначе false.
     */
    bool hasSignificantChanges(const FireDynamicsParameters &oldParams, const FireDynamicsParameters &newParams, double changeThreshold) const;
};

#endif // ROOMGRAPH_HPP
