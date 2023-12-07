// room_graph.hpp
#ifndef ROOMGRAPH_HPP
#define ROOMGRAPH_HPP

#include "room.hpp"
#include <vector>
#include <map>

class RoomGraph {
public:
    void addRoom(const Room& room);
    void addConnection(int roomId1, int roomId2, double connectionStrength);

    // Можете добавить другие методы и свойства

private:
    std::map<int, Room> rooms;
    std::map<int, std::vector<std::pair<int, double>>> adjacencyList;
};

#endif // ROOMGRAPH_HPP