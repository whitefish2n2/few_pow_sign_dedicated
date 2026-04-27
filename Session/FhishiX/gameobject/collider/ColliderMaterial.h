    //
    // Created by white on 26. 4. 24..
    //

#ifndef FPSPROJECTSERVER_COLLIDERMATERIAL_H
#define FPSPROJECTSERVER_COLLIDERMATERIAL_H
#include <string>

    enum class CombineMode {
    Average = 0,
    Minimum = 1,
    Multiply = 2,
    Maximum = 3
};

struct ColliderMaterial {
    float staticFriction = 0.6f;
    float dynamicFriction = 0.6f;
    float bounciness = 0.0f;

    CombineMode frictionCombine = CombineMode::Average;
    CombineMode bounceCombine = CombineMode::Average;

    // 문자열("Average", "Minimum" 등)을 CombineMode Enum으로 변환하는 도우미 함수
    static CombineMode ParseCombineMode(const std::string& val) {
        if (val == "Minimum") return CombineMode::Minimum;
        if (val == "Multiply") return CombineMode::Multiply;
        if (val == "Maximum") return CombineMode::Maximum;
        return CombineMode::Average; // 기본값
    }
};
#endif //FPSPROJECTSERVER_COLLIDERMATERIAL_H