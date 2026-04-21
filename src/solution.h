#ifndef SPEEDCIRCULARLIST_H
#define SPEEDCIRCULARLIST_H
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

constexpr int s_prime = 31;

inline int log2(int x) {
    int ans = 0;
    while (x > 1) {
        x /= 2;
        ++ans;
    }
    return ans;
}

template<typename T, int b_prime>
class SpeedCircularLinkedList {
public:
    struct Node {
        std::map<std::string, T> kv_map;
        int bound = 0;
        Node* next = nullptr;
        Node** fast_search_list = nullptr;

        Node(int bound_in, int fast_search_list_size) : bound(bound_in) {
            if (fast_search_list_size > 0) {
                fast_search_list = new Node*[fast_search_list_size];
                for (int i = 0; i < fast_search_list_size; ++i) fast_search_list[i] = nullptr;
            } else {
                fast_search_list = nullptr;
            }
        }

        ~Node() {
            if (fast_search_list) {
                delete[] fast_search_list;
                fast_search_list = nullptr;
            }
        }
    };

private:
    Node* head = nullptr;
    int fast_search_list_size = 0;
    int list_size = 0;

    static int GetHashCode(std::string str) {
        long long ans = 0;
        for (auto& ch : str) {
            ans = (ans * s_prime + ch) % b_prime;
        }
        return static_cast<int>((ans + b_prime) % b_prime);
    }

    void BuildFastSearchList() {
        if (list_size <= 0 || !head) return;
        // Simple jump table: fast_search_list[k] = 2^k successor
        std::vector<Node*> nodes;
        nodes.reserve(list_size);
        Node* cur = head;
        for (int i = 0; i < list_size; ++i) {
            nodes.push_back(cur);
            cur = cur->next;
        }
        for (int i = 0; i < list_size; ++i) {
            for (int k = 0; k < fast_search_list_size; ++k) {
                int idx = (i + (1 << k)) % list_size;
                nodes[i]->fast_search_list[k] = nodes[idx];
            }
        }
    }

public:
    explicit SpeedCircularLinkedList(std::vector<int> node_bounds) {
        list_size = static_cast<int>(node_bounds.size());
        if (list_size == 0) {
            head = nullptr;
            fast_search_list_size = 0;
            return;
        }
        fast_search_list_size = std::max(0, log2(std::max(1, list_size)));
        // Create nodes in increasing bound order
        head = new Node(node_bounds[0], fast_search_list_size);
        Node* prev = head;
        for (int i = 1; i < list_size; ++i) {
            Node* node = new Node(node_bounds[i], fast_search_list_size);
            prev->next = node;
            prev = node;
        }
        // make circular
        prev->next = head;
        BuildFastSearchList();
    }

    ~SpeedCircularLinkedList() {
        if (!head || list_size == 0) return;
        // Break the cycle to safely delete
        Node* cur = head->next;
        head->next = nullptr;
        while (cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        delete head;
        head = nullptr;
        list_size = 0;
    }

    void put(std::string str, T value) {
        int code = GetHashCode(str);
        if (!head) return;
        Node* cur = head;
        // If code <= head bound, store at head
        if (code <= head->bound) {
            cur->kv_map[str] = value;
            return;
        }
        // Traverse until find node with bound >= code and previous bound < code
        while (!(code <= cur->bound && code > cur->next->bound ? false : true)) {
            // The above condition is tricky; better do simple monotonic traversal
            if (code <= cur->bound) break;
            cur = cur->next;
            if (cur == head) break; // safety
        }
        // Simple linear traversal: find first bound >= code
        cur = head->next;
        while (cur != head && cur->bound < code) cur = cur->next;
        if (cur->bound >= code) {
            cur->kv_map[str] = value;
        } else {
            // Should not happen since code < b_prime == max bound, but fallback to head
            head->kv_map[str] = value;
        }
    }

    T get(std::string str) {
        int code = GetHashCode(str);
        if (!head) return T();
        if (code <= head->bound) {
            auto it = head->kv_map.find(str);
            if (it != head->kv_map.end()) return it->second;
            return T();
        }
        Node* cur = head->next;
        while (cur != head && cur->bound < code) cur = cur->next;
        if (cur->bound >= code) {
            auto it = cur->kv_map.find(str);
            if (it != cur->kv_map.end()) return it->second;
            return T();
        }
        return T();
    }

    void print() {
        if (!head) return;
        Node* cur = head;
        for (int i = 0; i < list_size; ++i) {
            std::cout << "[Node] Bound = " << cur->bound << ", kv_map_size = " << cur->kv_map.size() << '\n';
            cur = cur->next;
        }
    }

    int size() const { return list_size; }
};
#endif // SPEEDCIRCULARLIST_H

