//
// Created by white on 25. 5. 20.
//
#include "KDTree.h"
#include "../FhishiX/gameobject/GameObjectArgument.h"
#include "../FhishiX/gameobject/collider/BoxCollider.h"
#include "../FhishiX/gameobject/collider/Collider.h"

void KDTree::GetOverlaps(const AABB& queryAABB, std::vector<ComponentHandle<Collider>>& outOverlaps) const {
    if (rootIndex == -1 || nodes.empty()) return;

    // 1. 재귀 호출과 std::stack 대신 '고정 크기 원시 배열'을 사용하여 힙 할당 원천 차단
    int stack[64];
    int stackPtr = 0;

    // 루트 노드부터 탐색 시작
    stack[stackPtr++] = rootIndex;

    while (stackPtr > 0) {
        // 스택에서 노드 인덱스 하나 꺼내기 (Pop)
        int currentNodeIdx = stack[--stackPtr];
        const KDNode& node = nodes[currentNodeIdx];

        // 2. 가지치기 (Pruning): 현재 노드의 공간(AABB) 자체가 쿼리 공간과 겹치지 않으면 탐색 포기
        if (!node.bounds.Intersects(queryAABB)) {
            continue;
        }

        // 3. 현재 노드 안에 콜라이더들이 있다면, 실제 콜라이더의 AABB와 정밀 검사
        for (const auto& handle : node.objects) {
            Collider* col = handle.operator->();
            if (!col) continue;

            const AABB& objAABB = col->GetAABB();

            // 겹침 판정 (AABB vs AABB)
            if (objAABB.Intersects(queryAABB)) {
                outOverlaps.push_back(handle);
            }
        }

        // 4. 자식 노드가 있다면 스택에 추가 (Front를 나중에 넣어야 먼저 꺼내어 깊이 탐색함)
        if (node.back != -1) {
            stack[stackPtr++] = node.back;
        }
        if (node.front != -1) {
            stack[stackPtr++] = node.front;
        }
    }

    // ====================================================================
    // 5. 중복 제거 (Deduplication)
    // 걸쳐 있는(Straddling) 객체는 트리의 양쪽 자식 노드에 모두 들어가 있을 수 있습니다.
    // 검색 결과에서 동일한 핸들을 하나로 합칩니다.
    // ====================================================================
    if (outOverlaps.size() > 1) {
        // entityId는 컴포넌트 타입별로 독립된 풀이라, 서로 다른 타입인데 번호만 우연히 같을 수 있음
        // (예: BoxCollider entityId=2, MeshCollider entityId=2). typeId까지 같이 봐야 진짜 동일 객체.
        std::sort(outOverlaps.begin(), outOverlaps.end(), [](const auto& a, const auto& b) {
            if (a.typeId != b.typeId) return a.typeId < b.typeId;
            return a.entityId < b.entityId;
        });

        // 연속된 중복 항목을 뒤로 밀고 버림 (Swap and Pop의 응용)
        outOverlaps.erase(std::unique(outOverlaps.begin(), outOverlaps.end(), [](const auto& a, const auto& b) {
            return a.typeId == b.typeId && a.entityId == b.entityId;
        }), outOverlaps.end());
    }
}

void KDTree::Insert(ComponentHandle<Collider> collider) {
    if (collider.isNull()) return;
    if (nodes.empty()) return; // 배열이 비어있으면 루트가 없는 것
    objectCount++;

    InsertRecursive(0, collider, 0);
}

KDTree::~KDTree() {
    nodes.clear();
    objectCount = 0;
}

int KDTree::BuildRecursive(std::vector<ComponentHandle<Collider>> &colliders, int start, int end, int depth) {
    if (start > end) return -1;

    int nodeIdx = AllocateNode();

    AABB currentAABB = AABB::Empty();
    for (int i = start; i <= end; ++i) {
        currentAABB = AABB::ComputeUnion(currentAABB, colliders[i]->GetAABB());
    }
    nodes[nodeIdx].bounds = currentAABB;

    if (start == end) {
        nodes[nodeIdx].objects.push_back(colliders[start]);
        return nodeIdx;
    }

    int axis = depth % 3;
    int mid = start + (end - start) / 2;

    std::nth_element(colliders.begin() + start,
                     colliders.begin() + mid,
                     colliders.begin() + end + 1,
                     [axis](const ComponentHandle<Collider>& a, const ComponentHandle<Collider>& b) {
                         AABB boxA = a->GetAABB();
                         AABB boxB = b->GetAABB();

                         // AABB의 중심점(Center) 계산 (min + max)
                         float centerA, centerB;
                         if (axis == 0) {
                             centerA = boxA.min.x + boxA.max.x;
                             centerB = boxB.min.x + boxB.max.x;
                         } else if (axis == 1) {
                             centerA = boxA.min.y + boxA.max.y;
                             centerB = boxB.min.y + boxB.max.y;
                         } else {
                             centerA = boxA.min.z + boxA.max.z;
                             centerB = boxB.min.z + boxB.max.z;
                         }
                         return centerA < centerB;
                     });

    nodes[nodeIdx].front = BuildRecursive(colliders, start, mid, depth + 1);
    nodes[nodeIdx].back = BuildRecursive(colliders, mid + 1, end, depth + 1);

    return nodeIdx;
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
            auto boundBox = collider->GetAABB();
            switch (axis) {
                case 0: value = boundBox.min.x; break;
                case 1: value = boundBox.min.y; break;
                case 2: value = boundBox.min.z; break;
                default: value = boundBox.max.x; break;
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
    auto boundBox = collider->GetAABB();
    switch (axis) {
        case 0:
            if (boundBox.min.x <= node.mid.x && boundBox.max.x >= node.mid.x) return true;
            break;
        case 1:
            if (boundBox.min.y <= node.mid.y && boundBox.max.y >= node.mid.y) return true;
            break;
        case 2:
            if (boundBox.min.z <= node.mid.z && boundBox.max.z >= node.mid.z) return true;
            break;
        default:
            break;
    }
    return false;
}

bool KDTree::IsInFront(ComponentHandle<Collider> collider, int nodeIndex, int depth) {
    int axis = depth % 3;
    const KDNode& node = nodes[nodeIndex];
    auto boundBox = collider->GetAABB();
    switch (axis) {
        case 0: return boundBox.min.x >= node.mid.x;
        case 1: return boundBox.min.y >= node.mid.y;
        case 2: return boundBox.min.z >= node.mid.z;
        default: return boundBox.min.x >= node.mid.x;
    }
}