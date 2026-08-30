#include <cstdint>
#include <iostream>
#include <simhash/Simhasher.hpp>
#include <string>
#include <utility>
#include <vector>

// 二叉树节点定义
struct TreeNode {
  int val;
  TreeNode *left;
  TreeNode *right;
  TreeNode() : val(0), left(nullptr), right(nullptr) {}
  TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
  TreeNode(int x, TreeNode *left, TreeNode *right)
      : val(x), left(left), right(right) {}
};

// 单链表节点定义
struct ListNode {
  int val;
  ListNode *next;
  ListNode(int x) : val(x), next(nullptr) {}
};

class Solution {
public:
  ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
    if (!headA || !headB)
      return nullptr;

    ListNode *pA = headA;
    ListNode *pB = headB;

    // 当 pA 和 pB 相等时退出循环（可能是交点，也可能都是 nullptr）
    while (pA != pB) {
      // 如果到达自身链表末尾，则切换到另一条链表的头部，否则继续前进
      pA = (pA == nullptr) ? headB : pA->next;
      pB = (pB == nullptr) ? headA : pB->next;
    }

    // pA == pB，可能是交点，也可能是 nullptr（表示没有交点）
    return pA;
  }
};

using namespace simhash;
void test2() {
  Simhasher simhasher("./raw_data/module1/dict/jieba.dict.utf8",
                      "./raw_data/module1/dict/hmm_model.utf8",
                      "./raw_data/module1/dict/idf.utf8",
                      "./raw_data/module1/dict/stop_words.utf8");
  const char *bin1 = "100010110110";
  const char *bin2 = "110001110011";
  uint64_t u1 = Simhasher::binaryStringToUint64(bin1);
  uint64_t u2 = Simhasher::binaryStringToUint64(bin2);
  std::cout << "默认海明距离为3, isEqual = " << (Simhasher::isEqual(u1, u2))
            << "\n";
  std::cout << "默认海明距离为5, isEqual = " << (Simhasher::isEqual(u1, u2, 5))
            << "\n";
}
void test() {
  std::vector<int> vec;
  vec.reserve(5);
  std::cout << "vec.size = " << vec.capacity() << "\n";
}
int main() {
  int id = 0;
  std::string str("drewang");
  auto &&rvaue = std::make_pair(id, str);
  const auto &rvalue = std::make_pair(id + 1, str);
  test();
  test2();
  return 0;
}
