//
// Created by white on 25. 5. 20.
//

#ifndef BSPTREE_H
#define BSPTREE_H
#include <vector>

#include "../FhishiX/AABB.h"
#include "../FhishiX/gameobject/GameObject.h"
#include "../FhishiX/vector/Vector3.h"
#include "../Component/Definition/ComponentHandle.h"
#include "../FhishiX/gameobject/collider/Collider.h"

struct BSPNode {
    AABB bounds;
    Vector3 mid;
    std::vector<GameObject> objects;

    BSPNode* front = nullptr;
    BSPNode* back = nullptr;
    void updateMid() {
        mid.x = (bounds.min.x + bounds.max.x) / 2.0f;
        mid.y = (bounds.min.y + bounds.max.y) / 2.0f;
        mid.z = (bounds.min.z + bounds.max.z) / 2.0f;
    }
    bool isLeaf() const { return front == nullptr && back == nullptr; }
};

class BSPTree {
public:
    BSPNode* root = nullptr;
    int maxObjectsPerNode = 8;
    int maxDepth = 10;

    void Insert(GameObject* obj);

    static void DeleteNode(BSPNode *node);
    ~BSPTree();

private:
    void InsertRecursive(BSPNode *&node, GameObject obj, int depth);

    static void SplitNode(BSPNode*& node, int depth);

    static bool ShouldGoMultipleInsert(ComponentHandle<Collider> collider, BSPNode *node, int depth);

    static bool IsInFront(GameObject gameobject, BSPNode *node, int depth);


};
#endif //BSPTREE_H
