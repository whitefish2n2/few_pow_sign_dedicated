//
// Created by white on 25. 5. 20.
//
#include "KDTree.h"
#include "../FhishiX/gameobject/GameObjectArgument.h"
#include "../FhishiX/gameobject/collider/BoxCollider.h"
#include "../FhishiX/gameobject/collider/Collider.h"

void KDTree::Insert(ComponentHandle<Collider> collider) {
    if (collider == ComponentHandle<Collider>::NULLPTR()) return;
    if (nodes.empty()) return; // 배열이 비어있으면 루트가 없는 것
    objectCount++;

    InsertRecursive(0, collider, 0);
}

KDTree::~KDTree() {
    nodes.clear();
    objectCount = 0;
}

void KDTree::InsertRecursive(int nodeIndex, ComponentHandle<Collider> collider, int depth) {

    if (nodes[nodeIndex].isLeaf()) {
        if (nodes[nodeIndex].objects.size() < maxObjectsPerNode || depth >= maxDepth) {
            nodes[nodeIndex].objects.push_back(collider);
            return;
        }
        SplitNode(nodeIndex, depth);
    }

    if (ShouldGoMultipleInsert(collider, nodeIndex, depth)) {
        InsertRecursive(nodes[nodeIndex].front, collider, depth + 1);
        InsertRecursive(nodes[nodeIndex].back, collider, depth + 1);
    }
    else if (IsInFront(collider, nodeIndex, depth)) {
        InsertRecursive(nodes[nodeIndex].front, collider, depth + 1);
    }
    else {
        InsertRecursive(nodes[nodeIndex].back, collider, depth + 1);
    }
}

void KDTree::SplitNode(int nodeIndex, int depth) {
    // 자식 노드 공간 할당 (이때 nodes 벡터가 이사 갈 수도 있음!)
    int fIdx = AllocateNode();
    int bIdx = AllocateNode();

    // 이사 갔을 수도 있으므로, 인덱스를 통해 부모 노드에 다시 접근
    nodes[nodeIndex].front = fIdx;
    nodes[nodeIndex].back = bIdx;

    AABB original = nodes[nodeIndex].bounds;
    AABB FrontBound = original;
    AABB BackBound = original;

    int axis = depth % 3; // x:0, y:1, z:2
    float mid;

    switch (axis) {
        case 0: // X
            FrontBound.min.x = nodes[nodeIndex].mid.x;
            BackBound.max.x  = nodes[nodeIndex].mid.x;
            mid = nodes[nodeIndex].mid.x;
            break;
        case 1: // Y
            FrontBound.min.y = nodes[nodeIndex].mid.y;
            BackBound.max.y  = nodes[nodeIndex].mid.y;
            mid = nodes[nodeIndex].mid.y;
            break;
        case 2: // Z
            FrontBound.min.z = nodes[nodeIndex].mid.z;
            BackBound.max.z  = nodes[nodeIndex].mid.z;
            mid = nodes[nodeIndex].mid.z;
            break;
        default:
            FrontBound.min.z = nodes[nodeIndex].mid.z;
            BackBound.max.z  = nodes[nodeIndex].mid.z;
            mid = nodes[nodeIndex].mid.z;
            break;
    }

    nodes[fIdx].bounds = FrontBound;
    nodes[fIdx].updateMid();

    nodes[bIdx].bounds = BackBound;
    nodes[bIdx].updateMid();

    // 기존 객체들을 자식들에게 분배
    for (ComponentHandle<Collider>& collider : nodes[nodeIndex].objects) {
        float value;

        if (ShouldGoMultipleInsert(collider, nodeIndex, depth)) {
            nodes[fIdx].objects.push_back(collider);
            nodes[bIdx].objects.push_back(collider);
        }
        else {
            switch (axis) {
                case 0: value = collider->boundBox.min.x; break;
                case 1: value = collider->boundBox.min.y; break;
                case 2: value = collider->boundBox.min.z; break;
                default: value = collider->boundBox.max.x; break;
            }
            if (value >= mid) {
                nodes[fIdx].objects.push_back(collider);
            }
            else {
                nodes[bIdx].objects.push_back(collider);
            }
        }
    }

    nodes[nodeIndex].objects.clear();
}

bool KDTree::ShouldGoMultipleInsert(ComponentHandle<Collider> collider, int nodeIndex, int depth) {
    int axis = depth % 3;
    const KDNode& node = nodes[nodeIndex];

    switch (axis) {
        case 0:
            if (collider->boundBox.min.x <= node.mid.x && collider->boundBox.max.x >= node.mid.x) return true;
            break;
        case 1:
            if (collider->boundBox.min.y <= node.mid.y && collider->boundBox.max.y >= node.mid.y) return true;
            break;
        case 2:
            if (collider->boundBox.min.z <= node.mid.z && collider->boundBox.max.z >= node.mid.z) return true;
            break;
        default:
            break;
    }
    return false;
}

bool KDTree::IsInFront(ComponentHandle<Collider> collider, int nodeIndex, int depth) {
    int axis = depth % 3;
    const KDNode& node = nodes[nodeIndex];

    switch (axis) {
        case 0: return collider->boundBox.min.x >= node.mid.x;
        case 1: return collider->boundBox.min.y >= node.mid.y;
        case 2: return collider->boundBox.min.z >= node.mid.z;
        default: return collider->boundBox.min.x >= node.mid.x;
    }
}