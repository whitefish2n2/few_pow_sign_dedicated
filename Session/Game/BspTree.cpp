//
// Created by white on 25. 5. 20.
//
#include "BspTree.h"

void BSPTree::Insert(GameObject *obj) {
    if (!obj) return;
    InsertRecursive(root, obj, 0);
}
void BSPTree::DeleteNode(BSPNode* node) {
    if (!node) return;

    DeleteNode(node->front);
    DeleteNode(node->back);
    if (node->isLeaf())node->objects.clear();
    delete node;
}

void BSPTree::InsertRecursive(BSPNode*& node, GameObject* obj, int depth) {
    if (!node) {
        node = new BSPNode();
        node->bounds = obj->boundBox;
        node->updateMid();
    }

    if (node->isLeaf() && (node->objects.size() < maxObjectsPerNode || depth >= maxDepth)) {
        node->objects.push_back(obj);
        return;
    }

    if (node->isLeaf()) {
        SplitNode(node, depth);
    }
    if (ShouldGoMultipleInsert(obj,node,depth+1)) {
        InsertRecursive(node->front, obj, depth + 1);
        InsertRecursive(node->back, obj, depth + 1);
    }
    else if (IsInFront(obj, node, depth+1)) InsertRecursive(node->front, obj, depth + 1);
    else InsertRecursive(node->back, obj, depth + 1);
}

void BSPTree::SplitNode(BSPNode*& node, int depth) {
    if (!node) return;

    node->front = new BSPNode();
    node->back = new BSPNode();

    int axis = depth % 3; //x:0, y:1, z=2
    float mid;
    switch (axis) {
        case 0: // X
            mid = node->mid.x;
            break;
        case 1: // Y
            mid = node->mid.y;
            break;
        case 2: // Z
            mid = node->mid.z;
            break;
        default:
            mid = node->mid.x;
            break;
    }

    node->front->bounds = AABB::Empty();
    node->back->bounds = AABB::Empty();
    for (auto* obj : node->objects) {
        float value;

        if (ShouldGoMultipleInsert(obj, node, depth+1)) {
            node->front->objects.push_back(obj);
            node->front->bounds = AABB::ComputeUnion(node->front->bounds, obj->boundBox);
            node->back->objects.push_back(obj);
            node->back->bounds = AABB::ComputeUnion(node->back->bounds, obj->boundBox);
        }
        else {
            switch (axis) {
                case 0: value = obj->boundBox.min.x; break;
                case 1: value = obj->boundBox.min.y; break;
                case 2: value = obj->boundBox.min.z; break;
                default: value = obj->boundBox.max.x; break;
            }
            if (value >= mid) {
                node->front->objects.push_back(obj);
                node->front->bounds = AABB::ComputeUnion(node->front->bounds, obj->boundBox);
            }
            else {
                node->back->objects.push_back(obj);
                node->back->bounds = AABB::ComputeUnion(node->back->bounds, obj->boundBox);
            }
        }
    }
    node->front->updateMid();
    node->back->updateMid();
    node->objects.clear();
}

bool BSPTree::ShouldGoMultipleInsert(GameObject* obj, BSPNode* node, int depth) {
    int axis = depth % 3;
    switch (axis) {
        case 0:
            if (obj->boundBox.min.x <= node->mid.x && obj->boundBox.max.x >= node->mid.x) {
                return true;
            }
            break;
        case 1:
            if (obj->boundBox.min.y <= node->mid.y && obj->boundBox.max.y >= node->mid.y) {
                return true;
            }
            break;
        case 2:
            if (obj->boundBox.min.z <= node->mid.z && obj->boundBox.max.z >= node->mid.z) {
                return true;
            }
            break;
        default:break;
    }
    return false;
}
bool BSPTree::IsInFront(GameObject* obj, BSPNode* node, int depth) {
    int axis = depth % 3;

    switch (axis) {
        case 0:
            return obj->boundBox.min.x>node->mid.x;
            break;
        case 1:
            return obj->boundBox.min.y>node->mid.y;
            break;
        case 2:
            return obj->boundBox.min.z>node->mid.z;
            break;
        default: return obj->boundBox.min.x>node->mid.x;
    }
}

