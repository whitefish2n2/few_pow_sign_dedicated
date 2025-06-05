//
// Created by white on 25. 5. 23.
//

#pragma once
enum Layers {
    Default,
    Ground,
    Gun,
};

class Layer {
    public:
    static bool isCollisionable(Layers l1, Layers l2) {
        switch (l1) {
            case Default:
                switch (l2) {
                    case Default:
                        return true;
                    case Ground:
                        return true;
                    case Gun:
                        return false;
                }
            case Ground:
                switch (l2) {
                    case Default:
                        return true;
                    case Ground:
                        return true;
                    case Gun:
                        return true;
                }
            case Gun:
                switch (l2) {
                    case Default:
                        return false;
                    case Ground:
                        return true;
                    case Gun:
                        return false;
                }
        }
        return false;
    }
}
