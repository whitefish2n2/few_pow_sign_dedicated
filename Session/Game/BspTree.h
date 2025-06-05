//
// Created by white on 25. 5. 20.
//

#ifndef BSPTREE_H
#define BSPTREE_H
#include "../FhishiX/GameObject.h"
struct BSPNode {
    AABB bounds;
    Vector3 mid;
    std::vector<GameObject*> objects;

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

private:
    void InsertRecursive(BSPNode*& node, GameObject* obj, int depth);

    static void SplitNode(BSPNode*& node, int depth);

    static bool ShouldGoMultipleInsert(GameObject *obj, BSPNode *node, int depth);

    static bool IsInFront(GameObject *obj, BSPNode *node, int depth);
    BSPTree::~BSPTree() {
        DeleteNode(root);
    }
};
#endif //BSPTREE_H
