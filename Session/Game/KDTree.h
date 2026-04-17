//
// Created by white on 25. 5. 20.
//

#ifndef KDTREE_H
#define KDTREE_H
#include <stack>
#include <unordered_set>
#include <vector>

#include "../Component/Definition/ComponentHandle.h"
#include "../FhishiX/AABB.h"
#include "../FhishiX/vector/Vector3.h"
#include "../FhishiX/gameobject/collider/Collider.h"

struct KDNode {
    AABB bounds;
    Vector3 mid;
    std::vector<ComponentHandle<Collider>> objects;
    int front = -1;
    int back = -1;
    //-1  = 자식 없음


    void updateMid() {
        mid.x = (bounds.min.x + bounds.max.x) / 2.0f;
        mid.y = (bounds.min.y + bounds.max.y) / 2.0f;
        mid.z = (bounds.min.z + bounds.max.z) / 2.0f;
    }
    bool isLeaf() const { return front == -1 && back == -1; }
};
/*
 *fix
 * 알아보았던 힙 메모리 할당의 위험성...
 * 원래 구조는 KDNode 각 노드가 자식 노드를 들고 있었던 구조
 * 실행해보니 서버를 끌때 지연이 있었는데, 여기서 new로 할당한 KDNode 힙 메모리를 해제하는 과정에서 걸린 것으로 파악
 * 노드를 new로 생성하지 않고 트리를 만드는 방법이 뭐가 있을까?
 * vector<KDNode> 에다가 모든 노드를 생성되는 순서대로 저장하고, 각 노드에선 자식 두 개의 노드의 인덱스만 저장하도록 해보자.
 * 원본이 vector에 저장되기 때문에 탐색 속도도 포인터 체이싱이 아닌 인덱스 탐색으로 빨라지고, new로 할당 및 delete로 삭제할 필요도 없어졌다!
 */
class KDTree {
public:
    KDNode* root = nullptr;
    int objectCount = 0;
    std::vector<KDNode> nodes;
    KDTree()=default;
    KDTree(AABB worldSize) {
        nodes.reserve(10000);

        // 0번 인덱스 = 루트 노드
        KDNode rootNode;
        rootNode.bounds = worldSize;
        rootNode.updateMid();
        nodes.push_back(rootNode);
    }

    int AllocateNode() {
        KDNode newNode;
        nodes.push_back(newNode);
        return nodes.size() - 1;
    }
    int maxObjectsPerNode = 8;
    int maxDepth = 10;

    void Insert(ComponentHandle<Collider> collider);

    ~KDTree();

private:
    void InsertRecursive(int nodeIndex, ComponentHandle<Collider> collider, int depth);

    void SplitNode(int nodeIndex, int depth);

    bool ShouldGoMultipleInsert(ComponentHandle<Collider> collider, int nodeIndex, int depth);

    bool IsInFront(ComponentHandle<Collider> collider, int nodeIndex, int depth);


};
#endif