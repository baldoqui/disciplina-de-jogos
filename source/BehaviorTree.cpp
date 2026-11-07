#include <algorithm>

#include "BehaviorTree.hpp"

SelectorNode::SelectorNode(std::vector<BTNodePtr> children)
    : m_children(std::move(children)) {}

BTResult SelectorNode::tick(const BTContext &context) const {
  for (const BTNodePtr &child : m_children) {
    BTResult result = child->tick(context);
    if (result.success)
      return result;
  }
  return BTResult{};
}

SequenceNode::SequenceNode(std::vector<BTNodePtr> children)
    : m_children(std::move(children)) {}

BTResult SequenceNode::tick(const BTContext &context) const {
  BTResult out{true, ""};
  for (const BTNodePtr &child : m_children) {
    BTResult result = child->tick(context);
    if (!result.success)
      return BTResult{};
    if (!result.command.empty())
      out.command = std::move(result.command);
  }
  return out;
}

ConditionNode::ConditionNode(std::string name) : m_name(std::move(name)) {}

BTResult ConditionNode::tick(const BTContext &context) const {
  // Missing facts count as false; the first entry with the name decides.
  const auto it =
      std::find_if(context.begin(), context.end(),
                   [this](const auto &fact) { return fact.first == m_name; });
  return BTResult{it != context.end() && it->second != 0, ""};
}

ActionNode::ActionNode(std::string command) : m_command(std::move(command)) {}

BTResult ActionNode::tick(const BTContext &) const {
  return BTResult{true, m_command};
}

BehaviorTree::BehaviorTree(BTNodePtr root) : m_root(std::move(root)) {}

std::string BehaviorTree::tick(const BTContext &context) const {
  if (!m_root)
    return "";
  BTResult result = m_root->tick(context);
  return result.success ? result.command : "";
}
