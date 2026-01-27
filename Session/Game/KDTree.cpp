//
// Created by white on 25. 5. 20.
//
#include "KDTree.h"

#include "../FhishiX/gameobject/GameObjectArgument.h"
#include "../FhishiX/gameobject/collider/BoxCollider.h"
#include "../FhishiX/gameobject/collider/Collider.h"

void KDTree::Insert(Collider* collider) {
    if (collider == nullptr) return;
    if (!root) return;
    InsertRecursive(root,collider , 0);
}
void KDTree::DeleteNode(KDNode* node) {
    if (!node) return;

    DeleteNode(node->front);
    DeleteNode(node->back);
    delete node;
}

void KDTree::InsertRecursive(KDNode*& node, Collider* collider, int depth) {

    if (node->isLeaf()) {
        if ((node->objects.size() < maxObjectsPerNode || depth >= maxDepth)) {
            node->objects.push_back(collider);
            return;
        }
        SplitNode(node, depth);
    }

    if (ShouldGoMultipleInsert(collider,node,depth)) {
        InsertRecursive(node->front, collider, depth + 1);
        InsertRecursive(node->back, collider, depth + 1);
    }
    else if (IsInFront(collider, node, depth)) InsertRecursive(node->front, collider, depth + 1);
    else InsertRecursive(node->back, collider, depth + 1);
}

void KDTree::SplitNode(KDNode*& node, int depth) {
    if (!node) return;

    node->front = new KDNode();
    node->back = new KDNode();
    AABB original = node->bounds;
    AABB FrontBound = original;
    AABB BackBound = original;
    int axis = depth % 3; //x:0, y:1, z=2
    float mid;
    switch (axis) {
        case 0: // X
            FrontBound.min.x = node->mid.x;
            BackBound.max.x = node->mid.x;
            mid = node->mid.x;
            break;
        case 1: // Y
            FrontBound.min.y = node->mid.y;
            BackBound.max.y = node->mid.y;
            mid = node->mid.y;
            break;
        case 2: // Z
            FrontBound.min.z = node->mid.z;
            BackBound.max.z = node->mid.z;
            mid = node->mid.z;
            break;
        default:
            FrontBound.min.z = node->mid.z;
            BackBound.max.z = node->mid.z;
            mid = node->mid.z;
            break;
    }
    node->front->bounds = FrontBound;
    node->front->updateMid();

    node->back->bounds = BackBound;
    node->back->updateMid();

    for (Collider* collider : node->objects) {
        float value;

        if (ShouldGoMultipleInsert(collider, node, depth)) {
            node->front->objects.push_back(collider);
            node->back->objects.push_back(collider);
        }
        else {
            switch (axis) {
                case 0: value = collider->boundBox.min.x; break;
                case 1: value = collider->boundBox.min.y; break;
                case 2: value = collider->boundBox.min.z; break;
                default: value = collider->boundBox.max.x; break;
            }
            if (value >= mid) {
                node->front->objects.push_back(collider);
            }
            else {
                node->back->objects.push_back(collider);
            }
        }
    }
    node->front->updateMid();
    node->back->updateMid();
    node->objects.clear();
}

bool KDTree::ShouldGoMultipleInsert(Collider* collider, KDNode* node, int depth) {
    int axis = depth % 3;
    switch (axis) {
        case 0:
            if (collider-> boundBox.min.x <= node->mid.x && collider->boundBox.max.x >= node->mid.x) {
                return true;
            }
            break;
        case 1:
            if (collider->boundBox.min.y <= node->mid.y && collider->boundBox.max.y >= node->mid.y) {
                return true;
            }
            break;
        case 2:
            if (collider->boundBox.min.z <= node->mid.z && collider->boundBox.max.z >= node->mid.z) {
                return true;
            }
            break;
        default:break;
    }
    return false;
}
bool KDTree::IsInFront(Collider* collider, KDNode* node, int depth) {
    int axis = depth % 3;
    switch (axis) {
        case 0:
            return collider->boundBox.min.x>=node->mid.x;
            break;
        case 1:
            return collider->boundBox.min.y>=node->mid.y;
            break;
        case 2:
            return collider->boundBox.min.z>=node->mid.z;
            break;
        default: return collider->boundBox.min.x>=node->mid.x;
    }
}

KDTree::~KDTree() {
    DeleteNode(root);
}


