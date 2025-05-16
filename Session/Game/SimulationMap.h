//
// Created by white on 25. 5. 15.
//

#ifndef SIMULATIONMAP_H
#define SIMULATIONMAP_H
#include <vector>

#include "Player.h"
#include "../Dto/MapEnum.h"


class SimulationMap {
    public:
    SimulationMap();
    void Init(MapEnum map);
    void Init(MapEnum map, std::vector<Player>& players);

private:
    ~SimulationMap();
};



#endif //SIMULATIONMAP_H
