#ifndef OCTREE_HPP
#define OCTREE_HPP

#include <array>
#include <vector>
#include <assert.h>
#include <math.h>
#include <unordered_map>

template<typename T>
class Octree {
public:
    struct Node {
        T mData;

        Node() : mData(T()) {}
        Node(T d) : mData(d) {}
        
        Node* traverse(size_t index) {
            return &children[index];
        }

        bool hasChild(size_t index) {
            return children.find(index) != children.end();
        }
        
        bool hasChild() {
            for (int i = 0; i < 8; ++i) {
                if (hasChild(i)) {
                    return true;
                }
            }
            return false;
        }

        void insert(T data) {
            mData = data;
        }

        void insert(size_t index, T data) {
            assert(index < 8);
            children[index] = Node(data);
        }
    private:
        std::unordered_map<size_t, Node> children;
        
    };

public:
    Octree() : rootNode(Node(T())) {}

    Node* traverse(size_t index) { return &rootNode.children[index]; }
    Node* traverse() { return &rootNode; }

private:
    Node rootNode;
};

#endif // OCTREE_HPP
