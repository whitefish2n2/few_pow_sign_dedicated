//
// Created by white on 25. 5. 16.
//
#include "Map.h"




void Map::Init() {
}

Map::Map(MapEnum type) {
    this->type = type;
}

Map::~Map() {
    for (auto o :objects) {
        o.second.vertices.clear();
        o.second.triangles.clear();
    }
}

Map::Map(const Map & a) {
    this->objects = a.objects;
    this->type = a.type;

}


