#include <functional>
#include <iostream>
#include <memory>

#include "behavior_tree.h"

namespace bt = behavior_tree;

class Agent {
 public:
  Agent(const int val) : val_(val) { }

  bt::Status increment() {
    std::cout << "Agent::increment" << std::endl;
    ++val_;
    return val_ > 2 ? bt::SUCCESS : bt::FAILURE;
  }

  [[nodiscard]] const int& val() const { return val_; }

 private:
  int val_;
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
   *             /            \     \  \  \  \
   *       a(SEL)            b(SEQ)  c  d  e  e
   *        / \                |
   * leaf_fail leaf_success  leaf_success
   */
  auto root = bt::Sequence("root");
  auto a = bt::Selector("a");
  auto b = bt::Sequence("b");

  // Children can be set via constructor or children() setter.
  auto leaf_success = bt::Sequence("leaf_success", { action_success });
  auto leaf_fail = bt::Sequence("leaf_fail");
  leaf_fail.children() = { action_fail };

  // Actions can be set to class member functions via lambdas and std::bind.
  Agent agent(false);
  auto c = bt::Sequence("c", { [&agent] () { return agent.increment(); } });

  auto d = bt::Sequence("d", { std::bind(&Agent::increment, &agent) });

  a.children() = { leaf_fail, leaf_success };
  b.children() = { leaf_success };

  auto e = bt::FreezeStatus(bt::Status::SUCCESS, "e");
  e.children() = { [&agent] () { return agent.increment(); } };

  // References to the same node can be used to update the same node from
  // different children. This is only useful for nodes that have state.
  root.children() = { a, b, c, d, std::ref(e), std::ref(e) };

  for (int i = 0; i < 5; ++i) {
    std::cout << "root tick" << std::endl;
    const auto status = root();
    std::cout << status << std::endl;
  }

  std::cout << root.repr() << std::endl;

  return 0;
}
