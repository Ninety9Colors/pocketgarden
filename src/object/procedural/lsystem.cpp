#include "object/procedural/lsystem.hpp"

#include <queue>

#include "logging.hpp"

LNode::LNode(const json& j) : parent(nullptr) {from_json(j);}

json LNode::to_json() const {
    json result = {
        {"type",type},
        {"position",{{"x",position.x},{"y",position.y},{"z",position.z}}},
        {"direction",{{"x",direction.x},{"y",direction.y},{"z",direction.z}}},
        {"children",{}}
    };
    for (auto c : children) {
        result["children"].push_back(c->to_json());
    }
    return result;
}
void LNode::from_json(const json& j) {
    type = j.at("type");
    position = Vector3{j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    direction = Vector3{j.at("direction")["x"],j.at("direction")["y"],j.at("direction")["z"]};
    for (json c : j.at("children")) {
        children.push_back(std::make_shared<LNode>());
        children.back()->parent = shared_from_this();
        children.back()->from_json(c);
    }
}

Rule::Rule(std::function<std::vector<std::shared_ptr<LNode>>(std::shared_ptr<LNode>,std::mt19937_64&)> func) : apply_function_(func) {}
void Rule::apply(std::shared_ptr<LNode> node,std::mt19937_64& rng) {
    std::vector<std::shared_ptr<LNode>> new_nodes = apply_function_(node,rng);
    for (auto n : new_nodes) {
        node->children.push_back(n);
    }
}

RuleSet::RuleSet() {}
void RuleSet::apply_rule(std::shared_ptr<LNode> node,std::mt19937_64& rng) {
    std::string type = node->type;
    if (rules_.find(type) == rules_.end())
        return;
    rules_.at(type).apply(node,rng);
}
void RuleSet::add_rule(std::string node_type, Rule rule) {
    if (rules_.find(node_type) != rules_.end()) {
        WARN("Trying to add rule for " + node_type + ", but already exists!");
        return;
    }
    rules_[node_type] = rule;
}

LSystem::LSystem() : base_node_{nullptr} {}
LSystem::LSystem(const json& j) : base_node_{nullptr} {from_json(j);}
LSystem::LSystem(std::shared_ptr<LNode> base) : base_node_{nullptr} {base_node_ = base;}
void LSystem::apply_ruleset(RuleSet rules,std::mt19937_64& rng) {
    std::deque<std::shared_ptr<LNode>> dfs {};
    dfs.push_back(base_node_);
    while (!dfs.empty()) {
        auto top = dfs.back();
        dfs.pop_back();
        for (auto child : top->children)
            dfs.push_back(child);
        rules.apply_rule(top,rng);
    }
}
std::shared_ptr<LNode> LSystem::get_base() {
    return base_node_;
}
json LSystem::to_json() const {
    json j = {
        {"base_node",(base_node_ == nullptr ? "null_LNode" : base_node_->to_json())},
        {"type","LSystem"}
    };
    return j;
}
void LSystem::from_json(const json& j) {
    if (j.at("base_node") == "null_LNode") {
        base_node_ = nullptr;
        return;
    }
    base_node_ = std::make_shared<LNode>();
    base_node_->from_json(j.at("base_node"));
}