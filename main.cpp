#include <iostream>
#include <vector>

using namespace std;

const int MAX_CHILDREN = 5;  // 最大子节点数量
const int MAX_KEYS = MAX_CHILDREN - 1;  // 最大关键字数量

// B+ 树节点结构体
struct BPlusNode {
    int num_keys;  // 关键字数量
    int keys[MAX_KEYS];  // 关键字数组
    BPlusNode* children[MAX_CHILDREN];  // 子节点指针数组
    bool is_leaf;  // 是否为叶子节点
    BPlusNode* next;  // 下一个叶子节点指针

    // 构造函数
    BPlusNode() {
        num_keys = 0;
        for (int i = 0; i < MAX_CHILDREN; i++) {
            children[i] = NULL;
        }
        is_leaf = false;
        next = NULL;
    }
};

// B+ 树类
class BPlusTree {
public:
    BPlusTree() {
        root = NULL;
    }

    // 插入操作
    void Insert(int key) {
        if (root == NULL) {
            root = new BPlusNode;
            root->is_leaf = true;
            root->keys[0] = key;
            root->num_keys = 1;
        } else {
            BPlusNode* node = FindLeafNode(key);
            InsertIntoLeaf(node, key);
            if (node->num_keys == MAX_KEYS) {
                SplitLeafNode(node);
            }
        }
    }

    // 查找操作
    bool Search(int key) {
        BPlusNode* node = FindLeafNode(key);
        for (int i = 0; i < node->num_keys; i++) {
            if (node->keys[i] == key) {
                return true;
            }
        }
        return false;
    }

    // 删除操作
    void Delete(int key) {
        if (root == NULL) {
            return;
        }
        BPlusNode* node = FindLeafNode(key);
        for (int i = 0; i < node->num_keys; i++) {
            if (node->keys[i] == key) {
                for (int j = i; j < node->num_keys - 1; j++) {
                    node->keys[j] = node->keys[j + 1];
                }
                node->num_keys--;
                break;
            }
        }
        if (node->num_keys < (MAX_KEYS + 1) / 2) {
            MergeLeafNode(node);
        }
    }
private:
    // 根节点指针
    BPlusNode* root;

// 查找叶子节点
    BPlusNode* FindLeafNode(int key) {
        BPlusNode* node = root;
        while (!node->is_leaf) {
            int i;
            for (i = 0; i < node->num_keys; i++) {
                if (key < node->keys[i]) {
                    break;
                }
            }
            node = node->children[i];
        }
        return node;
    }

// 插入关键字到叶子节点
    void InsertIntoLeaf(BPlusNode* node, int key) {
        int i;
        for (i = 0; i < node->num_keys; i++) {
            if (key < node->keys[i]) {
                break;
            }
        }
        for (int j = node->num_keys; j > i; j--) {
            node->keys[j] = node->keys[j - 1];
        }
        node->keys[i] = key;
        node->num_keys++;
    }

// 分裂叶子节点
    void SplitLeafNode(BPlusNode* node) {
        BPlusNode* new_node = new BPlusNode;
        new_node->is_leaf = true;
        new_node->next = node->next;
        node->next = new_node;
        for (int i = (MAX_KEYS + 1) / 2; i < MAX_KEYS; i++) {
            new_node->keys[i - (MAX_KEYS + 1) / 2] = node->keys[i];
            new_node->num_keys++;
            node->num_keys--;
        }
        if (node == root) {
            root = new BPlusNode;
            root->keys[0] = new_node->keys[0];
            root->num_keys = 1;
            root->children[0] = node;
            root->children[1] = new_node;
        } else {
            BPlusNode* parent = FindParentNode(node);
            int index = FindChildIndex(parent, node);
            for (int i = parent->num_keys; i > index; i--) {
                parent->keys[i] = parent->keys[i - 1];
                parent->children[i + 1] = parent->children[i];
            }
            parent->keys[index] = new_node->keys[0];
            parent->num_keys++;
            parent->children[index + 1] = new_node;
        }
    }

// 查找父节点
    BPlusNode* FindParentNode(BPlusNode* node) {
        BPlusNode* parent = root;
        while (parent->children[0] != node) {
            int i;
            for (i = 0; i < parent->num_keys; i++) {
                if (node->keys[0] < parent->keys[i]) {
                    break;
                }
            }
            parent = parent->children[i];
        }
        return parent;
    }

// 查找子节点索引
    int FindChildIndex(BPlusNode* parent, BPlusNode* child) {
        int i;
        for (i = 0; i <= parent->num_keys; i++) {
            if (parent->children[i] == child) {
                break;
            }
        }
        return i;
    }

// 合并叶子节点
    void MergeLeafNode(BPlusNode* node) {
        if (node == root) {
            if (node->num_keys == 0) {
                root = NULL;
            }
            return;
        }
        BPlusNode* parent = FindParentNode(node);
        int index = FindChildIndex(parent, node);
        if (index > 0 && parent->children[index - 1]->num_keys > (MAX_KEYS + 1) / 2) {
// 从左兄弟借关键字
            BPlusNode* sibling = parent->children[index - 1];
            for (int i = node->num_keys; i > 0; i--) {
                node->keys[i] = node->keys[i - 1];
            }
            node->keys[0] = sibling->keys[sibling->num_keys - 1];
            node->num_keys++;
            sibling->num_keys--;
        } else if (index < parent->num_keys && parent->children[index + 1]->num_keys > (MAX_KEYS + 1) / 2) {
// 从右兄弟借关键字
            BPlusNode* sibling = parent->children[index + 1];
            node->keys[node->num_keys] = sibling->keys[0];
            node->num_keys++;
            for (int i = 0; i < sibling->num_keys - 1; i++) {
                sibling->keys[i] = sibling->keys[i + 1];
            }
            sibling->num_keys--;
        } else if (index > 0) {
            // 与左兄弟合并
            BPlusNode* sibling = parent->children[index - 1];
            for (int i = 0; i < node->num_keys; i++) {
                sibling->keys[sibling->num_keys + i] = node->keys[i];
            }
            sibling->num_keys += node->num_keys;
            sibling->next = node->next;
            if (parent->num_keys == 1) {
                root = sibling;
                root->next = NULL;
            } else {
                for (int i = index - 1; i < parent->num_keys - 1; i++) {
                    parent->keys[i] = parent->keys[i + 1];
                    parent->children[i + 1] = parent->children[i + 2];
                }
                parent->num_keys--;
            }
            delete node;
        } else {
            // 与右兄弟合并
            BPlusNode* sibling = parent->children[index + 1];
            for (int i = 0; i < sibling->num_keys; i++) {
                node->keys[node->num_keys + i] = sibling->keys[i];
            }
            node->num_keys += sibling->num_keys;
            node->next = sibling->next;
            if (parent->num_keys == 1) {
                root = node;
                root->next = NULL;
            } else {
                for (int i = index; i < parent->num_keys - 1; i++) {
                    parent->keys[i] = parent->keys[i + 1];
                    parent->children[i + 1] = parent->children[i + 2];
                }
                parent->num_keys--;
            }
            delete sibling;
        }
    }

};
