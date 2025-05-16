//
// Created by white on 25. 5. 15.
//

#ifndef MAP_H
#define MAP_H
#include <memory>
#include <vector>

#include "../Dto/MapEnum.h"
#include "../FhishiX/Vector3.h"

//loadmap으로 맵 버텍스,트라이앵글 정보를 불러와요
//init으로 맵의 진행상황, 트리거같은걸 초기화해요
class Map
{
    public:
    MapEnum type;
    std::vector<Vector3> vertices;
    std::vector<int[3]> triangles = std::vector<int[3]>();;

    std::shared_ptr<Map> LoadMap(MapEnum type);

    void Init();

    void GetMap();;
    void ReturnToPool();
};

#endif //MAP_H
