//
// Created by white on 25. 5. 12.
//

#pragma once
#include <string>
enum MapEnum
{
    test,
    haven,
};

static std::string GetMapInfoPath(MapEnum v)
{
    switch (v)
    {
        case test:
            return "test.mapVerticesInfo";
            break;
        case haven:
            return "haven";
            break;
    }
    return "";
}
