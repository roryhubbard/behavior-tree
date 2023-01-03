#include <functional>
#include <iostream>
#include <memory>

#include "behavior_tree.h"

namespace bt = behavior_tree;

class Agent {
 public:
  Agent(const bool val) : val_(val) { }

  bt::Status flip_val() {
    std::cout << "Agent::flip_val" << std::endl;
    val_ = !val_;
    return bt::SUCCESS;
  }

  [[nodiscard]] const bool& val() const { return val_; }

 private:
  bool val_;
};

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
   *                    root(SEQ)
   *             /            \     \  \
   *       a(SEL)            b(SEQ)  c  d
   *        / \                |
   * leaf_fail leaf_success  leaf_success
   */
  const auto root = std::make_shared<bt::Sequence>("root");
  const auto a = std::make_shared<bt::Selector>("a");
  const auto b = std::make_shared<bt::Sequence>("b");

  const auto leaf_success = std::make_shared<bt::Leaf>("leaf_success");
  leaf_success->action() = action_success;
  const auto leaf_fail = std::make_shared<bt::Leaf>("leaf_fail");
  leaf_fail->action() = action_fail;

  // Actions can be set to class member functions via lambdas and std::bind.
  Agent agent(false);
  const auto c = std::make_shared<bt::Leaf>("c");
  c->action() = [&agent] () { return agent.flip_val(); };

  const auto d = std::make_shared<bt::Leaf>("d");
  d->action() = std::bind(&Agent::flip_val, &agent);

  a->children() = { leaf_fail, leaf_success };
  b->children() = { leaf_success };
  root->children() = { a, b, c, d };

  root->tick();

  std::cout << *root << std::endl;
  return 0;
}
