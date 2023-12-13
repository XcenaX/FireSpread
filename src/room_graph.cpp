// room_graph.cpp
#include "room_graph.hpp"
#include <set>

void RoomGraph::addRoom(const Room &room)
{
    int roomId = room.getId();
    if (rooms.find(roomId) != rooms.end())
    {
        throw std::runtime_error("Room with the given ID already exists.");
    }
    rooms.emplace(roomId, room);
}

const std::map<int, Room> &RoomGraph::getRooms() const
{
    return rooms;
}

void RoomGraph::addConnection(int fromRoomId, int toRoomId, double connectionStrength)
{
    if (rooms.find(fromRoomId) == rooms.end() || rooms.find(toRoomId) == rooms.end())
    {
        throw std::runtime_error("One or both rooms do not exist in the graph.");
    }

    // Добавление исходящей связи
    outgoingAdjacencyList[fromRoomId].emplace_back(toRoomId, connectionStrength);

    // Добавление входящей связи
    incomingAdjacencyList[toRoomId].emplace_back(fromRoomId, connectionStrength);
}

void RoomGraph::topologicalSortUtil(int v, std::map<int, bool> &visited, std::stack<int> &Stack)
{
    visited[v] = true;

    for (std::pair<int, double> adjacent : outgoingAdjacencyList[v])
    {
        int adjacentId = adjacent.first;
        if (!visited[adjacentId])
        {
            topologicalSortUtil(adjacentId, visited, Stack);
        }
    }

    Stack.push(v);
}

void RoomGraph::topologicalSort(std::queue<int> &processingQueue)
{
    std::stack<int> Stack;
    std::map<int, bool> visited;

    for (const auto &pair : rooms)
    {
        visited[pair.first] = false;
    }

    for (const auto &pair : rooms)
    {
        if (!visited[pair.first])
        {
            topologicalSortUtil(pair.first, visited, Stack);
        }
    }

    while (!Stack.empty())
    {
        processingQueue.push(Stack.top());
        Stack.pop();
    }
}

int RoomGraph::findSourceRoomId() const
{
    for (const auto &roomPair : rooms)
    {
        if (roomPair.second.getIsGasSource())
        {
            return roomPair.first;
        }
    }
    return -1; // No source room found
}

void RoomGraph::initializeFireDynamicsForAllRooms(int sourceRoomId)
{
    for (auto &roomPair : rooms)
    {
        roomPair.second.addToFireDynamicsHistory(calculateFireDynamicsForRoom(sourceRoomId, roomPair.second, 0, roomPair.first != sourceRoomId));
    }
}

void RoomGraph::calculateFireDynamicsUpToTime(double timeInSeconds, int recursionDepthLimit, double changeThreshold, double timeStep = 1.0)
{
    int sourceRoomId = findSourceRoomId();

    if (sourceRoomId == -1)
    {
        throw std::runtime_error("No source room (fire origin) was found in the graph.");
    }

    initializeFireDynamicsForAllRooms(sourceRoomId);

    std::queue<int> processingQueue;
    topologicalSort(processingQueue);

    for (double currentTime = timeStep; currentTime <= timeInSeconds; currentTime += timeStep)
    {
        std::queue<int> currentProcessingQueue = processingQueue;

        while (!currentProcessingQueue.empty())
        {
            int roomId = currentProcessingQueue.front();
            currentProcessingQueue.pop();

            Room &room = rooms[roomId];
            FireDynamicsParameters currentParams = calculateFireDynamicsForRoom(sourceRoomId, room, currentTime, roomId != sourceRoomId);

            room.addToFireDynamicsHistory(currentParams);
        }
    }
}

FireDynamicsParameters RoomGraph::calculateFireDynamicsForRoom(int sourceRoomId, Room &room, double currentTime, bool hasConnections = true)
{
    InitialParameters initParams = room.getInitialParameters();
    CalculatedParameters calcParams = room.getCalculatedParameters();
    FireDynamicsParameters fireDynamicsParams;
    if (!hasConnections)
    {
        fireDynamicsParams.burned_mass = calcParams.A * pow(currentTime, calcParams.n);
        fireDynamicsParams.gas_density = calcParams.limit_gas_density + (initParams.initial_gas_density - calcParams.limit_gas_density) * exp(-calcParams.gas_release_per_meter_burn * fireDynamicsParams.burned_mass / initParams.room_volume);
        fireDynamicsParams.gas_temperature = initParams.start_temperature * initParams.initial_gas_density / fireDynamicsParams.gas_density;
        fireDynamicsParams.smoke_extinction_coefficient = calcParams.limit_smoke_extinction_coefficient + (0 - calcParams.limit_smoke_extinction_coefficient) * exp(-calcParams.gas_release_per_meter_burn * fireDynamicsParams.burned_mass / initParams.room_volume);
        fireDynamicsParams.visibility = std::min(30.0, calcParams.limit_visibility / fireDynamicsParams.smoke_extinction_coefficient);
        return fireDynamicsParams;
    }
    int count = 0;
    fireDynamicsParams.burned_mass = 0;
    fireDynamicsParams.gas_density = 0;
    fireDynamicsParams.gas_temperature = 0;
    fireDynamicsParams.smoke_extinction_coefficient = 0;
    fireDynamicsParams.visibility = 0;
    for (const auto &connection : incomingAdjacencyList[room.getId()])
    {
        int adjacentRoomId = connection.first;
        Room &adjacentRoom = rooms[adjacentRoomId];
        FireDynamicsParameters adjacentRoomFireDynamicsParams = rooms.find(adjacentRoomId)->second.getFireDynamicsHistory().back();
        double connectionStrength = connection.second;
        count++;
        fireDynamicsParams.burned_mass += calcParams.A * pow(currentTime, calcParams.n);
        fireDynamicsParams.gas_density += adjacentRoomFireDynamicsParams.gas_density + (initParams.initial_gas_density - adjacentRoomFireDynamicsParams.gas_density) * exp(-connectionStrength * calcParams.gas_release_per_meter_burn * rooms.find(sourceRoomId)->second.getFireDynamicsHistory().back().burned_mass / initParams.room_volume);
        fireDynamicsParams.gas_temperature += initParams.start_temperature * initParams.initial_gas_density / fireDynamicsParams.gas_density;
        fireDynamicsParams.smoke_extinction_coefficient += adjacentRoomFireDynamicsParams.smoke_extinction_coefficient + (0 - adjacentRoomFireDynamicsParams.smoke_extinction_coefficient) * exp(-connectionStrength * calcParams.gas_release_per_meter_burn * rooms.find(sourceRoomId)->second.getFireDynamicsHistory().back().burned_mass / initParams.room_volume);
        fireDynamicsParams.visibility += std::min(30.0, calcParams.limit_visibility / fireDynamicsParams.smoke_extinction_coefficient);
    }
    if (count > 0)
    {
        fireDynamicsParams.burned_mass /= count;
        fireDynamicsParams.gas_density /= count;
        fireDynamicsParams.gas_temperature /= count;
        fireDynamicsParams.smoke_extinction_coefficient /= count;
        fireDynamicsParams.visibility /= count;
    }
    return fireDynamicsParams;
}