#include "FireSimulation_2D.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

const int START_FIRE_X = 11;
const int START_FIRE_Y = 5;
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

void FireSimulation::initializePixels(const char room[ROOM_HEIGHT][ROOM_WIDTH], Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH]) {
    for (int i = 0; i < ROOM_HEIGHT; i++) {
        for (int j = 0; j < ROOM_WIDTH; j++) {
            char current_char = room[i][j];
            if (current_char == ' ') current_char = 'f';
            const PixelType* type = pixelDataMap[current_char];
            pixels[i][j].state = 0;
            pixels[i][j].fp = 0;
            pixels[i][j].x = i;
            pixels[i][j].y = j;
            pixels[i][j].pixel_type = type;
            pixels[i][j].t = 0;
            double fuel_mass = 0.0;
            switch (room[i][j]) {
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
            pixels[i][j].fuel_mass = fuel_mass;
        }
    }
}

void FireSimulation::displayRoom(Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH], char char_room[ROOM_HEIGHT][ROOM_WIDTH]) {
    for (int i = 0; i < ROOM_HEIGHT; i++) {
        for (int j = 0; j < ROOM_WIDTH; j++) {            
            if(pixels[i][j].state == BURNING){
                std::cout << '*';                
            } else if(pixels[i][j].state == EMPTY){
                std::cout << char_room[i][j];
            } else if(pixels[i][j].state == BURNT){
                std::cout << 'X';
            }   
        }
        std::cout << std::endl;
    }
}

int FireSimulation::calculateFP(Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH], int x, int y) {
    int a = 0;
    int b = 0;
    // TODO
    // Сделать проверку для 3D

    // Проверяем соседние пиксели по горизонтали и вертикали
    if (x > 0 && pixels[x - 1][y].state == BURNING) {
        a++;
    }
    if (x < ROOM_HEIGHT - 1 && pixels[x + 1][y].state == BURNING) {
        a++;
    }
    if (y > 0 && pixels[x][y - 1].state == BURNING) {
        a++;
    }
    if (y < ROOM_WIDTH - 1 && pixels[x][y + 1].state == BURNING) {
        a++;
    }

    // Проверяем соседние пиксели по диагонали
    if (x > 0 && y > 0 && pixels[x - 1][y - 1].state == BURNING) {
        b++;
    }
    if (x > 0 && y < ROOM_WIDTH - 1 && pixels[x - 1][y + 1].state == BURNING) {
        b++;
    }
    if (x < ROOM_HEIGHT - 1 && y > 0 && pixels[x + 1][y - 1].state == BURNING) {
        b++;
    }
    if (x < ROOM_HEIGHT - 1 && y < ROOM_WIDTH - 1 && pixels[x + 1][y + 1].state == BURNING) {
        b++;
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

    char char_room[ROOM_HEIGHT][ROOM_WIDTH];

    for (int i = 0; i < ROOM_HEIGHT; i++) {
        for (int j = 0; j < ROOM_WIDTH; j++) {
            char_room[i][j] = MAP[i * ROOM_WIDTH + j];
        }
    }

    PixelType* pixel_types = loadData();

    pixelDataMap['t'] = &pixel_types[6];
    pixelDataMap['d'] = &pixel_types[3];
    pixelDataMap['m'] = &pixel_types[19];
    pixelDataMap['#'] = &pixel_types[7];
    pixelDataMap['f'] = &pixel_types[15];

    Pixel pixels[ROOM_HEIGHT][ROOM_WIDTH];
    initializePixels(char_room, pixels);
    // initializePixels(char_room, pixels, pixel_types);

    List* CheckList = createList();
    List* NewList = createList();
    List* FireList = createList();

    addToList(NewList, &pixels[START_FIRE_Y][START_FIRE_X]);

    int step = 0;
    while (CheckList->size > 0 || NewList->size > 0 || FireList->size > 0 && step < 100) {
        // Обработка CheckList
        for (int i = 0; i < CheckList->size; i++) {
            Pixel* pixel = CheckList->pixels[i];
            int fp = calculateFP(pixels, pixel->x, pixel->y);
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
            // TODO 
            // Сделать проверку для 3D

            // Проверка соседнего пикселя влево
            if (x > 0 && pixels[x - 1][y].state < BURNING && MAP[(x - 1) * ROOM_WIDTH + y] != '#') {
                addToList(CheckList, &pixels[x-1][y]);
            }

            // Проверка соседнего пикселя вправо
            if (x < ROOM_HEIGHT - 1 && pixels[x + 1][y].state < BURNING && MAP[(x + 1) * ROOM_WIDTH + y] != '#') {
                addToList(CheckList, &pixels[x+1][y]);
            }

            // Проверка соседнего пикселя вверх
            if (y > 0 && pixels[x][y - 1].state < BURNING && MAP[x * ROOM_WIDTH + (y - 1)] != '#') {
                addToList(CheckList, &pixels[x][y-1]);
            }

            // Проверка соседнего пикселя вниз
            if (y < ROOM_WIDTH - 1 && pixels[x][y + 1].state < BURNING && MAP[x * ROOM_WIDTH + (y + 1)] != '#') {
                addToList(CheckList, &pixels[x][y+1]);
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