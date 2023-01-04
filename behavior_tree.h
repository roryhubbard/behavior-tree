#pragma once

#include <functional>
#include <string>
#include <vector>

namespace behavior_tree {

enum Status { IDLE, RUNNING, SUCCESS, FAILURE };

class Node {
 public:
  /** Setter for children. */
  std::vector<std::function<Status ()>>& children() { return children_; }

  virtual Status operator()() = 0;

 protected:
  Node(const std::string& name = "",
       const std::vector<std::function<Status ()>>& children = {})
      : name_(name), children_(children) { }

  std::string name_;

  std::vector<std::function<Status ()>> children_;

  std::string str() const { return name_; };
};

class Sequence final : public Node {
 public:
  Sequence(const std::string& name = "",
           const std::vector<std::function<Status ()>>& children = {})
      : Node(name, children) { }

  Status operator()() override {
    for (const auto& child : children_) {
      const auto status = child();
      if (status == Status::RUNNING || status == Status::FAILURE) {
        return status;
      }
    }
    return Status::SUCCESS;
  }

 private:
  std::string str() const { return Node::str() + " (SEQUENCE)"; }
};

class Selector final : public Node {
 public:
  Selector(const std::string& name = "",
           const std::vector<std::function<Status ()>>& children = {})
      : Node(name, children) { }

  Status operator()() override {
    for (const auto& child : children_) {
      const auto status = child();
      if (status == Status::RUNNING || status == Status::SUCCESS) {
        return status;
      }
    }
    return Status::FAILURE;
  }

 private:
  std::string str() const { return Node::str() + " (SELECTOR)"; }
};

class CacheStatus final : public Node {
 public:
  CacheStatus(const Status status_to_cache, const std::string& name = "",
              const std::vector<std::function<Status ()>>& children = {})
      : Node(name, children), status_to_cache_(status_to_cache) { }

  Status operator()() override {
    if (current_status_ == status_to_cache_) {
      return current_status_;
    }
    current_status_ = children_.front()();
    return current_status_;
  }

 private:
  std::string str() const { return Node::str() + " (CACHE_STATUS)"; }

  Status current_status_;

  Status status_to_cache_;
};

}  // namespace behavior_tree
