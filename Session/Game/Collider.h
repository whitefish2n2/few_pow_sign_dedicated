//
// Created by white on 25. 5. 23.
//

#ifndef COLLIDER_H
#define COLLIDER_H

struct Collision {

}

class Collider {
    public:
    bool isColliding(Collider& other);
    bool isColliding(Collider& c1,const Collider& c2);
};



#endif //COLLIDER_H
