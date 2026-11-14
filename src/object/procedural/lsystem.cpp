#include "object/procedural/lsystem.hpp"

#include <queue>

#include "logging.hpp"
#include "json.hpp"
using json = nlohmann::json;

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
    std::shared_ptr<LNode> base = shared_from_this();
    std::deque<std::pair<std::shared_ptr<LNode>,std::shared_ptr<json>>> bfs {};
    bfs.push_back({base,std::make_shared<json>(j)});
    while (!bfs.empty()) {
        const auto& front = bfs.front();
        auto node = front.first;
        auto j2 = front.second;
        node->type = j2->at("type");
        node->position = Vector3{j2->at("position")["x"],j2->at("position")["y"],j2->at("position")["z"]};
        node->direction = Vector3{j2->at("direction")["x"],j2->at("direction")["y"],j2->at("direction")["z"]};
        for (const json& c : j2->at("children")) {
            node->children.push_back(std::make_shared<LNode>());
            node->children.back()->parent = node;
            bfs.push_back({node->children.back(),std::make_shared<json>(c)});
        }
        bfs.pop_front();
    }
}
std::string LNode::to_string() const {
    std::string result;
    std::deque<std::shared_ptr<const LNode>> s;
    s.push_back(shared_from_this());
    while (!s.empty()) {
        auto n = s.back(); 
        s.pop_back();
        result += n->type;
        for (auto child : n->children)
            s.push_back(child);
    }
    return result;
}

Rule::Rule(std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode>,std::mt19937_64&)> func) : apply_function_(func) {}
std::shared_ptr<LNode> Rule::apply(std::shared_ptr<LNode> node,std::mt19937_64& rng) {
    return apply_function_(node,rng);
}

RuleSet::RuleSet() {}
std::shared_ptr<LNode> RuleSet::apply_rule(std::shared_ptr<LNode> node,std::mt19937_64& rng) {
    std::string type = node->type;
    if (rules_.find(type) == rules_.end())
        return node;
    return rules_.at(type).apply(node,rng);
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
    DEBUG("Applying ruleset to L system: " + base_node_->to_string());
    std::deque<std::shared_ptr<LNode>> bfs {};
    std::deque<std::pair<int,std::shared_ptr<LNode>>> reverse_bfs {};
    bfs.push_back(base_node_);
    reverse_bfs.push_back({-1,base_node_});
    while (!bfs.empty()) {
        auto top = bfs.front();
        for (int i = 0; i < top->children.size(); i++) {
            bfs.push_back(top->children[i]);
            reverse_bfs.push_front({i,top->children[i]});
        }
        bfs.pop_front();
    }
    while (!reverse_bfs.empty()) {
        auto front = reverse_bfs.front();
        auto replacement = rules.apply_rule(front.second,rng);
        // Apply rule should automatically relink the node's children, we just have to link the parent

        if (front.first != -1 && replacement != nullptr)
            front.second->parent->children[front.first] = replacement;
        replacement->parent = front.second->parent;
        if (front.second == base_node_)
            base_node_ = replacement;
        
        reverse_bfs.pop_front();
    }
    DEBUG("New L system: " + to_string());
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

std::string LSystem::to_string() const {
    return base_node_->to_string();
}