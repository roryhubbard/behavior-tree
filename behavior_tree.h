#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace behavior_tree {

// Nodes never return IDLE. IDLE is the initial status for nodes that have
// status as a state.
enum Status { IDLE, RUNNING, SUCCESS, FAILURE };

// Forward declaration for `repr()` method.
class Sequence;
class Selector;
class FreezeStatus;

class Node {
 public:
  /** Setter for children. */
  std::vector<std::function<Status ()>>& children() { return children_; }

  virtual Status operator()() = 0;

  /**
   * Output json-like string. If more child Nodes are create, this function
   * will need to be updated.
   */
  std::string repr() const {
    std::string r;
    r += " { " + str() + ":";
    std::for_each(children_.begin(), children_.end(),
        [&r] (const auto& child) {
      if (const auto sequence = child.template target<Sequence>()) {
        r += sequence->repr();
      } else if (const auto selector = child.template target<Selector>()) {
        r += selector->repr();
      } else if (
          const auto freeze_status = child.template target<FreezeStatus>()) {
        r += freeze_status->repr();
      } else {
        r += " { LEAF },";
      }
    });
    r += " },";
    return r;
  }

  virtual std::string str() const { return name_; };

 protected:
  Node(const std::string& name = "",
       const std::vector<std::function<Status ()>>& children = {})
      : name_(name), children_(children) { }

  std::string name_;

  std::vector<std::function<Status ()>> children_;
};

class Sequence : public Node {
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

  std::string str() const override { return Node::str() + "(SEQUENCE)"; }
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

  std::string str() const override { return Node::str() + "(SELECTOR)"; }
};

/**
 * Once a certain status is achieved, always return that status on subsequent
 * executions without executing any children.
 */
class FreezeStatus final : public Sequence {
 public:
  FreezeStatus(const Status freeze_status, const std::string& name = "",
               const std::vector<std::function<Status ()>>& children = {})
      : Sequence(name, children), freeze_status_(freeze_status) { }

  Status operator()() override {
    if (current_status_ == freeze_status_) {
      return current_status_;
    }
    current_status_ = Sequence::operator()();
    return current_status_;
  }

  std::string str() const override { return Node::str() + "(FREEZE_STATUS)"; }

 private:
  Status current_status_;

  Status freeze_status_;
};

}  // namespace behavior_tree
