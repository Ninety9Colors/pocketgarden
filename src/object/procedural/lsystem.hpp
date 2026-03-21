#pragma once

#include "raylib.h"

#include "json.hpp"
using json = nlohmann::json;

#include <functional>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

class LNode : public std::enable_shared_from_this<LNode> {
public:
    LNode() : type("NullType"),position{},direction{},children{},parent{nullptr} {};
    LNode(std::string type, Vector3 position, Vector3 direction, std::vector<std::shared_ptr<LNode>> children, std::shared_ptr<LNode> parent) :
        type(type), position(position), direction(direction), children(children), parent(parent) {}
    LNode(const json& j);

    json to_json() const;
    void from_json(const json& j);
    std::string to_string() const;
    
    std::string type;
    Vector3 position;
    Vector3 direction;
    std::vector<std::shared_ptr<LNode>> children;
    std::shared_ptr<LNode> parent;
};

class Rule {
public:
    Rule() {};
    Rule(std::function<std::vector<std::shared_ptr<LNode>>(std::shared_ptr<LNode>,std::mt19937_64&)> func);
    void apply(std::shared_ptr<LNode> node,std::mt19937_64& rng);
private:
    std::function<std::vector<std::shared_ptr<LNode>>(std::shared_ptr<LNode>,std::mt19937_64&)> apply_function_;
};

class RuleSet {
public:
    RuleSet();
    void apply_rule(std::shared_ptr<LNode> node,std::mt19937_64& rng) ;
    void add_rule(std::string node_type, Rule rule);
private:
    std::map<std::string,Rule> rules_;
};

class LSystem {
public:
    LSystem();
    LSystem(const json& j);
    LSystem(std::shared_ptr<LNode> base);
    void apply_ruleset(RuleSet rules,std::mt19937_64& rng);
    std::shared_ptr<LNode> get_base();

    json to_json() const;
    void from_json(const json& j);
    std::string to_string() const;
private:
    std::shared_ptr<LNode> base_node_;
};