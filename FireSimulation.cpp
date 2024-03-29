#include "FireSimulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

const int START_FIRE_X = 11;
const int START_FIRE_Y = 5;
const int START_FIRE_Z = 1;
const int TIME_SPEED = 3; // Ускорить вывод

const char * MAP = "\
####################################################################################################\
#    tt   tt   tt     t #    tt   tt   tt   t #                                      #     ttt     #\
#    tt   tt   tt     t #    tt   tt   tt   t #                                      #ttt       ttt#\
#                       #                     #                                      #             #\
#    tt   tt   tt     t #    tt   tt   tt   t #                                      #ttt       ttt#\
#    tt   tt   tt     t #    tt   tt   tt   t #                                      #             #\
#                       #                     #              mmm   mmm   mmm         #ttt       ttt#\
####################dd#####dd##################       ########################       # t    mm   t #\
#mm                                                   #       tt   tt   tt   #       #####dd########\
#mm                                                   #m t                   #                     #\
####################dd#####dd##################       #m t    tt   tt   tt   d                     #\
#    tt   tt   tt     t #    tt   tt   tt   t #       #m t    tt   tt   tt   d                     #\
#    tt   tt   tt     t #    tt   tt   tt   t #       #m t                   d                     #\
#                       #                     #       #       tt   tt   tt   #                     #\
#    tt   tt   tt     t #    tt   tt   tt   t #       #m t    tt   tt   tt   d                     #\
#    tt   tt   tt     t #    tt   tt   tt   t #       #m t                   d      mmm   mmm   mmm#\
#                       #                     #       #       tt   tt   tt   #      mmm   mmm   mmm#\
#    tt   tt   tt     t #    tt   tt   tt   t #                                      #     ttt     #\
#    tt   tt   tt     t #    tt   tt   tt   t #                                      #ttt       ttt#\
#                       #                     #                                      #             #\
#    tt   tt   tt     t #    tt   tt   tt   t #                                      #ttt       ttt#\
#    tt   tt   tt     t #    tt   tt   tt   t #                                      #             #\
#                       #                     #              mmm   mmm   mmm         #ttt       ttt#\
####################dd#####dd##################       ########################       # t    mm   t #\
#mm                                                   #       tt   tt   tt   #       #####dd########\
#mm                                                   #m t                   #                     #\
####################dd#####dd##################       #m t    tt   tt   tt   d                     #\
#    tt   tt   tt     t #    tt   tt   tt   t #       #m t    tt   tt   tt   d                     #\
#    tt   tt   tt     t #    tt   tt   tt   t #       #m t                   d                     #\
#                       #                     #       #       tt   tt   tt   #                     #\
#    tt   tt   tt     t #    tt   tt   tt   t #       #m t    tt   tt   tt   d                     #\
#    tt   tt   tt     t #    tt   tt   tt   t #       #m t                   d      mmm   mmm   mmm#\
#                       #                     #       #       tt   tt   tt   #      mmm   mmm   mmm#\
####################################################################################################";


FireSimulation::FireSimulation() {
    srand(time(NULL));
}

FireSimulation::~FireSimulation() {
    // Освобождение ресурсов, если надо будет
}

PixelType* FireSimulation::loadData() {
    FILE* fp = fopen(JSON_FILE_PATH, "r");
    if (!fp) {
        printf("Ошибка при считывании файла: %s\n", strerror(errno));
        return nullptr;
    }

    char readBuffer[40000];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    rapidjson::Document d;
    d.ParseStream(is);

    fclose(fp);

    int numElements = d.Size();
    PixelType* data = new PixelType[numElements];

    for (int i = 0; i < numElements; i++) {
        const rapidjson::Value& obj = d[i];
        data[i].Name = obj["Name"].GetString();
        data[i].LowestHeatOfCombustion_kJ_per_kg = obj["LowestHeatOfCombustion_kJ_per_kg"].GetDouble();
        data[i].LinearFlameSpeed = obj["LinearFlameSpeed"].GetDouble();
        data[i].BurningRate = obj["BurningRate"].GetDouble();
        data[i].SmokeGeneration = obj["SmokeGeneration"].GetDouble();
        data[i].OxygenConsumption_kg_per_kg = obj["OxygenConsumption_kg_per_kg"].GetDouble();

        const rapidjson::Value& gasEmission = obj["GasEmission"];
        data[i].GasEmission.CarbonDioxide_kg_per_kg = gasEmission["CarbonDioxide_kg_per_kg"].GetDouble();
        data[i].GasEmission.CarbonMonoxide_kg_per_kg = gasEmission["CarbonMonoxide_kg_per_kg"].GetDouble();
        data[i].GasEmission.HydrogenChloride_kg_per_kg = gasEmission["HydrogenChloride_kg_per_kg"].GetDouble();
    }

    return data;
}

void FireSimulation::initializePixels(const char room[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH], Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH]) {
    for (int i = 0; i < ROOM_HEIGHT; i++) {
        for (int j = 0; j < ROOM_WIDTH; j++) {
            for (int k = 0; k < ROOM_DEPTH; k++) {
                char current_char = room[i][j][k];
                if (current_char == ' ') current_char = 'f';
                const PixelType* type = pixelDataMap[current_char];
                pixels[i][j][k].state = 0;
                pixels[i][j][k].fp = 0;
                pixels[i][j][k].x = i;
                pixels[i][j][k].y = j;
                pixels[i][j][k].pixel_type = type;
                pixels[i][j][k].t = 0;
                double fuel_mass = 5.0;
                switch (room[i][j][k]) {
                    case 't':
                        fuel_mass = 5;
                        break;
                    case 'd':
                        fuel_mass = 10;
                        break;
                    case '#':
                        fuel_mass = 200;
                        break;
                    case 'm':
                        fuel_mass = 20;
                        break;
                    case ' ':
                        fuel_mass = 50;
                        break;
                    default:
                        break;
                }
                pixels[i][j][k].fuel_mass = fuel_mass;
            }
        }
    }
}

// TODO когда буду переносить на UE сделать норм вывод
void FireSimulation::displayRoom(Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH], char char_room[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH]) {
    for (int i = 0; i < ROOM_HEIGHT; i++) {
        for (int j = 0; j < ROOM_WIDTH; j++) {            
            if(pixels[i][j][0].state == BURNING){
                std::cout << '*';                
            } else if(pixels[i][j][0].state == EMPTY){
                std::cout << char_room[i][j][0];
            } else if(pixels[i][j][0].state == BURNT){
                std::cout << 'X';
            }   
        }
        std::cout << std::endl;
    }
}

int FireSimulation::calculateFP(Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH], int x, int y, int z) {
    int a = 0;
    int b = 0;

    // Проверяем соседние пиксели по горизонтали, вертикали и в глубину
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                // Пропускаем центральный пиксель
                if (dx == 0 && dy == 0 && dz == 0) continue;

                int newX = x + dx;
                int newY = y + dy;
                int newZ = z + dz;

                // Проверяем, что координаты находятся в пределах комнаты
                if (newX >= 0 && newX < ROOM_HEIGHT &&
                    newY >= 0 && newY < ROOM_WIDTH &&
                    newZ >= 0 && newZ < ROOM_DEPTH) {

                    // Проверяем состояние соседнего пикселя
                    if (pixels[newX][newY][newZ].state == BURNING) {
                        if (dx == 0 || dy == 0 || dz == 0) {
                            a++; // Сосед по горизонтали/вертикали/глубине
                        } else {
                            b++; // Сосед по диагонали
                        }
                    }
                }
            }
        }
    }

    return 2 * a + b;
}


List* FireSimulation::createList() {
    List* list = new List;
    list->pixels = nullptr;
    list->size = 0;
    return list;
}

void FireSimulation::addToList(List* list, Pixel* pixel) {
    list->size++;
    list->pixels = (Pixel**)realloc(list->pixels, sizeof(Pixel*) * list->size);
    list->pixels[list->size - 1] = pixel;
}

void FireSimulation::removeFromList(List* list, Pixel* pixel) {
    for (int i = 0; i < list->size; i++) {
        if (list->pixels[i] == pixel) {
            for (int j = i; j < list->size - 1; j++) {
                list->pixels[j] = list->pixels[j + 1];
            }
            list->size--;
            list->pixels = (Pixel**)realloc(list->pixels, sizeof(Pixel*) * list->size);
            break;
        }
    }
}

void FireSimulation::runSimulation() {
    srand(time(NULL));

    char char_room[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH];

    for (int i = 0; i < ROOM_HEIGHT; i++) {
        for (int j = 0; j < ROOM_WIDTH; j++) {
            for (int k = 0; k < ROOM_DEPTH; k++) {
                char_room[i][j][k] = MAP[i * ROOM_WIDTH + j];
            }
        }
    }

    PixelType* pixel_types = loadData();

    pixelDataMap['t'] = &pixel_types[6];
    pixelDataMap['d'] = &pixel_types[3];
    pixelDataMap['m'] = &pixel_types[19];
    pixelDataMap['#'] = &pixel_types[7];
    pixelDataMap['f'] = &pixel_types[15];

    Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH][ROOM_DEPTH];
    initializePixels(char_room, pixels);
    // initializePixels(char_room, pixels, pixel_types);

    List* CheckList = createList();
    List* NewList = createList();
    List* FireList = createList();

    addToList(NewList, &pixels[START_FIRE_Y][START_FIRE_X][START_FIRE_Z]);

    int step = 0;
    while (CheckList->size > 0 || NewList->size > 0 || FireList->size > 0 && step < 100) {
        std::cout << "STEP: " << step << "\n";
        // Обработка CheckList
        for (int i = 0; i < CheckList->size; i++) {
            Pixel* pixel = CheckList->pixels[i];
            int fp = calculateFP(pixels, pixel->x, pixel->y, pixel->z);
            double probability = (V * fp) / FIRE_SPREAD_PROB_DIVISOR;
            // probability *= (1.0 - CheckList->pixels[i]->pixel_type->LowestHeatOfCombustion_kJ_per_kg / MAX_LOWEST_HEAT_OF_COMBUSTION); // Уменьшаем P на основе Низшей теплоты сгорания

            if(probability == 0){
                removeFromList(CheckList, pixel);
            }
            else if (rand() / (double)RAND_MAX < probability) {
                pixel->state = BURNING;
                removeFromList(CheckList, pixel);
                addToList(NewList, pixel);
            }
        }

        for (int i = 0; i < NewList->size; i++) {
            Pixel* pixel = NewList->pixels[i];
            int x = pixel->x;
            int y = pixel->y;
            int z = pixel->z;

            // Проверка соседних пикселей во всех направлениях
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        // Пропускаем сам пиксель
                        if (dx == 0 && dy == 0 && dz == 0) continue;

                        int newX = x + dx;
                        int newY = y + dy;
                        int newZ = z + dz;

                        // Проверяем, что координаты находятся в пределах комнаты
                        if (newX >= 0 && newX < ROOM_HEIGHT &&
                            newY >= 0 && newY < ROOM_WIDTH &&
                            newZ >= 0 && newZ < ROOM_DEPTH) {

                            // Проверяем состояние соседнего пикселя и стену на карте
                            if (pixels[newX][newY][newZ].state < BURNING && MAP[newX * ROOM_WIDTH + newY] != '#') {
                                addToList(CheckList, &pixels[newX][newY][newZ]);
                            }
                        }
                    }
                }
            }

            // Перенос пикселя из NewList в FireList
            pixel->state = BURNING;
            addToList(FireList, pixel);
            removeFromList(NewList, pixel);
        }


        // Обработка FireList
        for (int i = 0; i < FireList->size; i++) {
            Pixel* pixel = FireList->pixels[i];

            if (pixel->state == BURNING) {                
                pixel->t += TIME_SPEED * 1; 
                double A = 1.05 * pixel->pixel_type->BurningRate * pow(pixel->pixel_type->LinearFlameSpeed, 2);
                double burntMass = A * pow(pixel->t, 3);

                if (pixel->fuel_mass <= burntMass) {
                    pixel->state = BURNT;
                    removeFromList(FireList, pixel);
                    //delete pixel;
                }
            }
        }

        step++;
        //system("cls");
        printf("Шаг %d:\n", step);
        displayRoom(pixels, char_room);
        Sleep(1000 / TIME_SPEED);
    }

    delete[] pixel_types;
    delete[] CheckList->pixels;
    delete[] NewList->pixels;
    delete[] FireList->pixels;
    delete CheckList;
    delete NewList;
    delete FireList;
}