//
// Created by white on 25. 5. 23.
//

#pragma once
enum class Layers {
    Default,
    Ground,
    Gun,
};

class Layer {
    public:
    static bool isCollisionable(Layers l1, Layers l2) {
        switch (l1) {
            case Layers::Default:
                switch (l2) {
                    case Layers::Default:
                        return true;
                    case Layers::Ground:
                        return true;
                    case Layers::Gun:
                        return false;
                }
            case Layers::Ground:
                switch (l2) {
                    case Layers::Default:
                        return true;
                    case Layers::Ground:
                        return true;
                    case Layers::Gun:
                        return true;
                }
            case Layers::Gun:
                switch (l2) {
                    case Layers::Default:
                        return false;
                    case Layers::Ground:
                        return true;
                    case Layers::Gun:
                        return false;
                }
        }
        return false;
    }
};
