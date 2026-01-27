//
// Created by white on 25. 5. 20.
//

#ifndef KDTREE_H
#define KDTREE_H
#include <vector>

#include "../FhishiX/AABB.h"
#include "../FhishiX/gameobject/GameObject.h"
#include "../FhishiX/vector/Vector3.h"
#include "../Component/Definition/ComponentHandle.h"
#include "../FhishiX/gameobject/collider/Collider.h"

struct KDNode {
    AABB bounds;
    Vector3 mid;
    std::vector<Collider*> objects;
    KDNode* front = nullptr;
    KDNode* back = nullptr;
    void updateMid() {
        mid.x = (bounds.min.x + bounds.max.x) / 2.0f;
        mid.y = (bounds.min.y + bounds.max.y) / 2.0f;
        mid.z = (bounds.min.z + bounds.max.z) / 2.0f;
    }
    bool isLeaf() const { return front == nullptr && back == nullptr; }
};

class KDTree {
public:
    KDNode* root = nullptr;
    KDTree()=default;
    KDTree(AABB worldSize) {
        root = new KDNode();
        root->bounds = worldSize;
        root->updateMid();
    }
    int maxObjectsPerNode = 8;
    int maxDepth = 10;

    void Insert(Collider *collider);

    static void DeleteNode(KDNode *node);
    ~KDTree();

private:
    void InsertRecursive(KDNode *&node, Collider *collider, int depth);

    static void SplitNode(KDNode*& node, int depth);

    static bool ShouldGoMultipleInsert(Collider *collider, KDNode *node, int depth);

    static bool IsInFront(Collider *collider, KDNode *node, int depth);


};
#endif