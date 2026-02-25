//
// Created by white on 25. 5. 20.
//

#ifndef KDTREE_H
#define KDTREE_H
#include <stack>
#include <unordered_set>
#include <vector>

#include "../FhishiX/AABB.h"
#include "../FhishiX/vector/Vector3.h"
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
class Iterator {
    private:
        std::stack<KDNode*> nodeStack;
        KDNode* currentNode = nullptr;
        size_t currentObjectIndex = 0;

        // 중복 방지를 위한 방문 기록 (Iterator 복사 시 상태를 공유하기 위해 shared_ptr 사용)
        std::shared_ptr<std::unordered_set<Collider*>> visited;
        Collider* currentCollider = nullptr;

        void advance() {
            currentCollider = nullptr;

            while (currentNode != nullptr || !nodeStack.empty()) {
                if (currentNode == nullptr) {
                    currentNode = nodeStack.top();
                    nodeStack.pop();
                    currentObjectIndex = 0;
                }

                while (currentObjectIndex < currentNode->objects.size()) {
                    Collider* col = currentNode->objects[currentObjectIndex++];
                    if (visited->find(col) == visited->end()) {
                        visited->insert(col);
                        currentCollider = col;
                        return;
                    }
                }


                if (currentNode->front) nodeStack.push(currentNode->front);
                if (currentNode->back) nodeStack.push(currentNode->back);

                currentNode = nullptr;
            }
        }

    public:
        Iterator() = default;

        Iterator(KDNode* root) : visited(std::make_shared<std::unordered_set<Collider*>>()) {
            if (root) {
                currentNode = root;
                advance();
            }
        }

        Collider* operator*() const {
            return currentCollider;
        }

        Iterator& operator++() {
            advance();
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return currentCollider != other.currentCollider;
        }
    };

    Iterator begin() const {
        return Iterator(root);
    }

    Iterator end() const {
        return Iterator();
    }

    KDNode* root = nullptr;
    int objectCount = 0;
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