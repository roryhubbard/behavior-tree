#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace behavior_tree {

enum Status { RUNNING, SUCCESS, FAILURE };

class Node {
 public:
  Node(const std::string& name) : name_(name) { }

  /** Setter for children. */
  std::vector<std::shared_ptr<const Node>>& children() { return children_; }

  /** Execute this node. */
  virtual Status tick() const = 0;

  friend std::ostream& operator<<(std::ostream& os, const Node& node) {
    os << node.str() << " " << node.name_ << " { ";
    std::for_each(node.children_.begin(), node.children_.end(),
        [&os] (const auto& n) { os << *n; });
    os << "} ";
    return os;
  }

 protected:
  std::string name_;

  std::vector<std::shared_ptr<const Node>> children_;

  virtual std::string str() const = 0;
};

class Sequence final : public Node {
 public:
  Sequence(const std::string& name) : Node(name) { }

  Status tick() const override {
    for (const auto& child : children_) {
      const auto status = child->tick();
      if (status == Status::RUNNING || status == Status::FAILURE) {
        return status;
      }
    }
    return Status::SUCCESS;
  }

 private:
  std::string str() const override { return "SEQUENCE"; }
};

class Selector final : public Node {
 public:
  Selector(const std::string& name) : Node(name) { }

  Status tick() const override {
    for (const auto& child : children_) {
      const auto status = child->tick();
      if (status == Status::RUNNING || status == Status::SUCCESS) {
        return status;
      }
    }
    return Status::FAILURE;
  }

 private:
  std::string str() const override { return "SELECTOR"; }
};

class Leaf final : public Node {
 public:
  Leaf(const std::string& name) : Node(name) { }

  /** Setter for action. */
  std::function<Status ()>& action() { return action_; }

  Status tick() const override { return action_(); }

 private:
  std::function<Status ()> action_;

  std::string str() const override { return "LEAF"; }
};

}  // namespace behavior_tree
