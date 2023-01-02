#include <iostream>
#include <memory>

#include "behavior_tree.h"

namespace bt = behavior_tree;

bt::Status action_success() {
  std::cout << "action_success" << std::endl;
  return bt::Status::SUCCESS;
}

bt::Status action_fail() {
  std::cout << "action_fail" << std::endl;
  return bt::Status::FAILURE;
}

int main() {
  /**
   *             root(SEQ)
   *              /    \
   *       a(SEL)        b(SEQ)
   *        / \            |
   * leaf(F)   leaf(S)   leaf(S)
   */
  const auto root = std::make_shared<bt::Sequence>("root");
  const auto a = std::make_shared<bt::Selector>("a");
  const auto b = std::make_shared<bt::Sequence>("b");

  const auto leaf_success = std::make_shared<bt::Leaf>("leaf_success");
  leaf_success->action() = action_success;
  const auto leaf_fail = std::make_shared<bt::Leaf>("leaf_fail");
  leaf_fail->action() = action_fail;

  a->children() = { leaf_fail, leaf_success };
  b->children() = { leaf_success };
  root->children() = { a, b };

  root->tick();

  std::cout << *root << std::endl;
  return 0;
}
