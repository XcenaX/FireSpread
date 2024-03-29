#ifndef FIRESIMULATION_H
#define FIRESIMULATION_H

#include <unordered_map>

#define ROOM_WIDTH 100
#define ROOM_HEIGHT 34
#define ROOM_DEPTH 50
#define EMPTY 0
#define BURNING 1   
#define BURNT 2

const double V = 0.08; // Скорость линейного распространения пожара
const double FIRE_SPREAD_PROB_DIVISOR = 4;
const double MAX_LOWEST_HEAT_OF_COMBUSTION = 45000;

extern const int START_FIRE_X;
extern const int START_FIRE_Y;
extern const int START_FIRE_Z;

extern const char * MAP;

struct PixelType {
    const char* Name;
    double LowestHeatOfCombustion_kJ_per_kg;
    double LinearFlameSpeed;
    double BurningRate;
    double SmokeGeneration;
    double OxygenConsumption_kg_per_kg;
    struct GasEmission {
        double CarbonDioxide_kg_per_kg;
        double CarbonMonoxide_kg_per_kg;
        double HydrogenChloride_kg_per_kg;
    } GasEmission;
};

struct Pixel {
    int state;
    int fp;
    int x;
    int y;
    int z;
    double fuel_mass; 
    int t; // Текущее время горения
    const PixelType* pixel_type;
};

struct List {
    Pixel** pixels;
    int size;
};

class FireSimulation {
public:
    FireSimulation();
    ~FireSimulation();

    void runSimulation();

private:
    std::unordered_map<char, const PixelType*> pixelDataMap;
    const char* JSON_FILE_PATH = "G:/VKR/Automates3/fire.json";

    PixelType* loadData();
    void initializePixels(const char room[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH], Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH]);
    void displayRoom(Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH], char char_room[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH]);
    int calculateFP(Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH], int x, int y, int z);
    List* createList();
    void addToList(List* list, Pixel* pixel);
    void removeFromList(List* list, Pixel* pixel);
};

#endif // FIRESIMULATION_H
