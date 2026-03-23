#include "lily.hpp"

#include "raymath.h"
#include "rlgl.h"

#include <random>
#include <memory>

#include "logging.hpp"

Lily::Lily() : Lily(std::random_device{}()) {}
Lily::Lily(uint32_t seed) : Object3d(), seed_(seed), stage_(0), flower_(std::make_unique<LilyFlower>(seed)), leaf_(std::make_unique<TaperedLeaf>(std::mt19937_64{seed}())) {}
Lily::Lily(Quaternion quaternion, Vector3 position, float scale) : Lily(quaternion,position,scale,std::random_device{}()) {}
Lily::Lily(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : Object3d(quaternion,position,scale), seed_(seed), stage_(0), flower_(std::make_unique<LilyFlower>(seed)), leaf_(std::make_unique<TaperedLeaf>(std::mt19937_64{seed}())) {}
Lily::Lily(const json& j) {from_json(j);}

// Should regenerate mesh if needed
void Lily::advance_stage() {
    if (stage_ == 1) {
        stage_++;
        std::mt19937_64 rng (seed_);
        lsystem_.apply_ruleset(stage_transitions_[std::pair<int,int>{1,2}],rng);
        generate_mesh();
    } else if (stage_ == 2) {
        stage_++;
        std::mt19937_64 rng (std::mt19937_64{seed_}());
        for (int i = 0; i < 5; i++)
            lsystem_.apply_ruleset(stage_transitions_[std::pair<int,int>{2,3}],rng);
        generate_mesh();
    } else if (stage_ == 3) {
        stage_++;
        std::mt19937_64 rng (std::mt19937_64{std::mt19937_64{seed_}()}());
        for (int i = 0; i < 3; i++)
            lsystem_.apply_ruleset(stage_transitions_[std::pair<int,int>{3,4}],rng);
        generate_mesh();
    }
}

// Should be called explicitly after the object is created
void Lily::initialize() {
    flower_->generate_mesh();
    leaf_->generate_mesh();

    if (lsystem_.get_base() == nullptr) {
        lsystem_ = LSystem(std::make_shared<LNode>("Seed",position_,Vector3{0,1,0},std::vector<std::shared_ptr<LNode>>{},nullptr));
        stage_ = 1;
    }

    RuleSet one_to_two {};
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> germinate = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::uniform_real_distribution gen (0.0,1.0);
            float roll = gen(rng);
            float length;
            if (roll < 0.05) {
                length = 0.05f;
            } else if (roll < 0.8) {
                length = 0.15f;
            } else {
                length = 0.3f;
            }
            std::shared_ptr<LNode> stem = std::make_shared<LNode>("Stem",Vector3Scale(node->direction,length),node->direction,node->children,node);
            for (auto child : stem->children)
                child->parent = stem;
            node->children.clear();
            node->children.push_back(stem);
            return node;
        };
        one_to_two.add_rule("Seed",Rule(germinate));
    stage_transitions_[std::pair<int,int>{1,2}] = one_to_two;

    RuleSet two_to_three {};
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> juvenile = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::uniform_real_distribution gen (0.0,1.0);
            int stem_children = 0;
            for (auto c : node->children)
                stem_children += (c->type=="Stem");
            bool is_tip = !stem_children;
            float roll1 = gen(rng);
            std::shared_ptr<LNode> new_stem = nullptr;
            std::shared_ptr<LNode> leaf = nullptr;
            if (roll1 < 0.7f) {
                std::uniform_real_distribution angle (0.0f,2*PI);
                Vector3 perp = Vector3Perpendicular(node->direction);
                perp = Vector3RotateByAxisAngle(perp,node->direction,angle(rng));
                std::uniform_real_distribution tilt (0.0f,15.0f*DEG2RAD);
                Vector3 new_direction = Vector3Normalize(Vector3RotateByAxisAngle(node->direction,perp,tilt(rng)));

                float roll = gen(rng);
                float length;
                if (roll < 0.05) {
                    length = 0.025f;
                } else if (roll < 0.8) {
                    length = 0.075f;
                } else {
                    length = 0.06f;
                }
                new_stem = std::make_shared<LNode>("Stem",Vector3Scale(new_direction,length),new_direction,node->children,node);
                for (auto child : new_stem->children)
                    child->parent = new_stem;
                node->children.clear();
                node->children.push_back(new_stem);
                is_tip = false;
            }

            if (is_tip)
                return node;
            float roll = gen(rng);
            if (roll < 0.8) { // Branch and leaf
                std::uniform_real_distribution angle (0.0f,2*PI);
                Vector3 perp = Vector3Perpendicular(node->direction);
                perp = Vector3RotateByAxisAngle(perp,node->direction,angle(rng));
                std::uniform_real_distribution tilt (45.0f*DEG2RAD,90.0f*DEG2RAD);
                Vector3 leaf_dir = Vector3RotateByAxisAngle(node->direction,perp,tilt(rng));
                float length = 0.05f;

                leaf = std::make_shared<LNode>("Leaf",Vector3Scale(leaf_dir,length),leaf_dir,std::vector<std::shared_ptr<LNode>>{},node);
                node->children.push_back(leaf);
            }
            return node;
        };
        two_to_three.add_rule("Stem",Rule(juvenile));
    stage_transitions_[std::pair<int,int>{2,3}] = two_to_three;

    RuleSet three_to_four {};
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> mature = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            int stem_children = 0;
            for (auto c : node->children)
                stem_children += (c->type=="Stem") || (c->type=="Flower" && c->direction == node->direction);
            if (stem_children)
                return node;
            std::uniform_real_distribution gen (0.0,1.0);
            float roll = gen(rng);
            std::shared_ptr<LNode> flower = nullptr;
            if (roll < 0.8) { // Branch and bloom
                std::uniform_real_distribution angle (0.0f,2*PI);
                Vector3 perp = Vector3Perpendicular(node->direction);
                perp = Vector3RotateByAxisAngle(perp,node->direction,angle(rng));
                std::uniform_real_distribution tilt (45.0f*DEG2RAD,90.0f*DEG2RAD);
                Vector3 flower_dir = Vector3RotateByAxisAngle(node->direction,perp,tilt(rng));
                float length = 0.25f;

                flower = std::make_shared<LNode>("Flower",Vector3Scale(flower_dir,length),flower_dir,std::vector<std::shared_ptr<LNode>>{},node);
            } else {
                float length = 0.25f;
                flower = std::make_shared<LNode>("Flower",Vector3Scale(node->direction,length),node->direction,std::vector<std::shared_ptr<LNode>>{},node);
            }
            node->children.push_back(flower);
            return node;
        };
        three_to_four.add_rule("Stem",Rule(mature));
    stage_transitions_[std::pair<int,int>{3,4}] = three_to_four;
    INFO("Initialized Lily with Seed LNode and stage: " + std::to_string(stage_));
}

void Lily::draw(Game& game, Material material) const {
    DrawMesh(mesh_,material_,transform_);
    flower_->draw_instanced(game,flower_transforms_.data(),flower_transforms_.size());
    leaf_->draw_instanced(game,leaf_transforms_.data(),leaf_transforms_.size());
}

json Lily::to_json() const {
    json j = {
        {"type","Lily"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"scale",scale_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}},
        {"seed",seed_},
        {"flower",flower_->to_json()},
        {"leaf",leaf_->to_json()},
        {"stage",stage_},
        {"lsystem",lsystem_.to_json()}
    };
    return j;
}

void Lily::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    scale_ = j.at("scale");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
    seed_ = j.at("seed");
    flower_ = std::make_unique<LilyFlower>(j.at("flower"));
    leaf_ = std::make_unique<TaperedPetal>(j.at("leaf"));
    stage_ = j.at("stage");
    lsystem_ = LSystem(j.at("lsystem"));
}

static void generate_stem_segment(std::vector<float>& vertices,
                                    std::vector<float>& normals,
                                    std::vector<unsigned char>& colors,
                                    std::vector<unsigned short>& indices,
                                    Vector3 start_pos,
                                    Vector3 end_pos,
                                    Vector3 start_dir,
                                    Vector3 end_dir,
                                    int start_id,
                                    int end_id,
                                    std::map<int,std::pair<int,Vector3>>& previous_vertexes,
                                    Color color,
                                    float width = 0.025f) {
    Vector3 direction = Vector3Normalize(Vector3Subtract(end_pos,start_pos));
    Vector3 normal;

    float length = Vector3Length(Vector3Subtract(end_pos,start_pos));
    float step_size = 0.1f;
    int vertexes_per_unit = 8;

    int start_vertex_index = vertices.size()/3;
    if (previous_vertexes.find(start_id) != previous_vertexes.end()) {
        start_vertex_index = previous_vertexes[start_id].first;
        normal = Vector3RotateByQuaternion(previous_vertexes[start_id].second,QuaternionFromVector3ToVector3(start_dir,end_dir));
    } else {
        normal = Vector3Normalize(Vector3Perpendicular(direction));
    }

    int steps = std::max(int(length/step_size),1);
    for (int step = 1; step <= steps; step++) {
        Vector3 stem_pos = Vector3Add(start_pos,Vector3Scale(direction,step*step_size));
        Vector3 stem_pos_prev = Vector3Add(start_pos,Vector3Scale(direction,step_size*(step-1)));
        if (step == steps)
            stem_pos = end_pos;

        // Generate circle of vertices for prev pos
        int bottom_vertex_index = start_vertex_index;
        if (step == 1 && start_vertex_index != vertices.size()/3) {
        } else if (step > 1) {
            bottom_vertex_index = vertices.size()/3-vertexes_per_unit;
        } else {
            for (int rot = 0; rot < vertexes_per_unit; rot++) {
                float angle = float(rot)/float(vertexes_per_unit)*2.0f*PI;
                Vector3 sideways_direction = Vector3Normalize(Vector3RotateByAxisAngle(normal,direction,angle));
                Vector3 vertex_pos = Vector3Add(Vector3Scale(sideways_direction,width),stem_pos_prev);
                vertices.push_back(vertex_pos.x);
                vertices.push_back(vertex_pos.y);
                vertices.push_back(vertex_pos.z);

                normals.push_back(sideways_direction.x);
                normals.push_back(sideways_direction.y);
                normals.push_back(sideways_direction.z);

                colors.push_back((uint8_t)color.r);
                colors.push_back((uint8_t)color.g);
                colors.push_back((uint8_t)color.b);
                colors.push_back((uint8_t)color.a);
            }
            previous_vertexes[start_id].first = bottom_vertex_index;
            previous_vertexes[start_id].second = normal;
        }

        // Generate circle of vertices for curr pos
        int top_vertex_index = vertices.size()/3;
        for (int rot = 0; rot < vertexes_per_unit; rot++) {
            float angle = float(rot)/float(vertexes_per_unit)*2.0f*PI;
            Vector3 sideways_direction = Vector3Normalize(Vector3RotateByAxisAngle(normal,direction,angle));
            Vector3 vertex_pos = Vector3Add(Vector3Scale(sideways_direction,width),stem_pos);
            vertices.push_back(vertex_pos.x);
            vertices.push_back(vertex_pos.y);
            vertices.push_back(vertex_pos.z);

            normals.push_back(sideways_direction.x);
            normals.push_back(sideways_direction.y);
            normals.push_back(sideways_direction.z);

            colors.push_back((uint8_t)color.r);
            colors.push_back((uint8_t)color.g);
            colors.push_back((uint8_t)color.b);
            colors.push_back((uint8_t)color.a);
        }
        if (step == steps) {
            previous_vertexes[end_id].first = top_vertex_index;
            previous_vertexes[end_id].second = normal;
        }
        // Connecting the vertices with triangles
        for (int quad = 0; quad < vertexes_per_unit; quad++) {
            unsigned short bottom_left = bottom_vertex_index+(quad)%vertexes_per_unit;
            unsigned short bottom_right = bottom_vertex_index+(quad+1)%vertexes_per_unit;
            unsigned short top_left = top_vertex_index+quad;
            unsigned short top_right = top_vertex_index+(quad+1)%vertexes_per_unit;
            indices.push_back(bottom_left);
            indices.push_back(bottom_right);
            indices.push_back(top_left);

            indices.push_back(top_left);
            indices.push_back(bottom_right);
            indices.push_back(top_right);
        }
    }
}

namespace {
    struct StackFrame {
        Vector3 prev_pos;
        Vector3 pos;
        std::shared_ptr<LNode> node;
        int id;
        int prev_id;
    };
}

void Lily::generate_mesh() {
    if (mesh_.vboId != 0)
        UnloadMesh(mesh_);
    mesh_ = Mesh{0};
    std::vector<float> vertices {};
    std::vector<float> normals {};
    std::vector<unsigned char> colors {};
    std::vector<unsigned short> indices {};

    std::deque<StackFrame> dfs {};
    assert(lsystem_.get_base() != nullptr);
    dfs.push_back({Vector3{0,0,0},Vector3{0,0,0},lsystem_.get_base(),0,-1});
    int next_id = 1;
    std::map<int,std::pair<int,Vector3>> previous_vertexes {};

    Color stem_color = ColorFromHSV(leaf_->get_parameter("BaseHue").value,leaf_->get_parameter("BaseSaturation").value,leaf_->get_parameter("BaseValue").value);
    flower_transforms_.clear();
    flower_transforms_base_.clear();
    leaf_transforms_.clear();
    leaf_transforms_base_.clear();
    while (!dfs.empty()) {
        auto top = dfs.back();
        DEBUG(top.node->type);
        if (top.node->type == "Stem") {
            generate_stem_segment(vertices,normals,colors,indices,top.prev_pos,top.pos,top.node->parent->direction,top.node->direction,top.prev_id,top.id,previous_vertexes,stem_color);
        } else if (top.node->type == "Flower") {
            generate_stem_segment(vertices,normals,colors,indices,top.prev_pos,top.pos,top.node->parent->direction,top.node->direction,top.prev_id,top.id,previous_vertexes,stem_color);
            Vector3 flower_pos = Vector3Subtract(top.pos,Vector3Scale(top.node->direction,0.05f));
            Matrix rot = QuaternionToMatrix(QuaternionFromVector3ToVector3(Vector3{0,1,0},top.node->direction));
            Matrix translate = MatrixTranslate(flower_pos.x,flower_pos.y,flower_pos.z);
            flower_transforms_.push_back(MatrixIdentity());
            flower_transforms_base_.push_back({rot,translate});
        } else if (top.node->type == "Leaf") {
            generate_stem_segment(vertices,normals,colors,indices,top.prev_pos,top.pos,top.node->parent->direction,top.node->direction,top.prev_id,top.id,previous_vertexes,stem_color,0.001f);
            Vector3 leaf_pos = Vector3Subtract(top.pos,Vector3Scale(top.node->direction,0.001f));
            Quaternion stem_to_vertical = QuaternionFromVector3ToVector3(top.node->parent->direction,{0,1,0});
            Vector3 tip_vector_hoz = Vector3Normalize({leaf_->tip_vector().x,0,leaf_->tip_vector().z});
            Vector3 leaf_vector_hoz = Vector3RotateByQuaternion(top.node->direction,stem_to_vertical);
            leaf_vector_hoz.y = 0;
            leaf_vector_hoz = Vector3Normalize(leaf_vector_hoz);
            float angle_vertical = atan2f(Vector3CrossProduct(tip_vector_hoz,leaf_vector_hoz).y,Vector3DotProduct(tip_vector_hoz,leaf_vector_hoz));
            Quaternion rot_vertical = QuaternionFromAxisAngle({0,1,0},angle_vertical);

            Matrix rot = QuaternionToMatrix(QuaternionMultiply(QuaternionInvert(stem_to_vertical),rot_vertical));
            Matrix translate = MatrixTranslate(leaf_pos.x,leaf_pos.y,leaf_pos.z);
            leaf_transforms_.push_back(MatrixIdentity());
            leaf_transforms_base_.push_back({rot,translate});
        }
        dfs.pop_back();
        for (auto child : top.node->children) {
            dfs.push_back({top.pos,Vector3Add(top.pos,child->position),child,next_id++,top.id});
        }
    }

    assert(vertices.size()%3 == 0);
    assert(indices.size()%3 == 0);

    mesh_.vertexCount = vertices.size()/3;
    mesh_.triangleCount = indices.size()/3;
    mesh_.vertices = (float*)MemAlloc(mesh_.vertexCount*sizeof(float)*3);
    mesh_.indices = (unsigned short*)MemAlloc(mesh_.triangleCount*sizeof(unsigned short)*3);
    mesh_.colors = (unsigned char*)MemAlloc(mesh_.vertexCount*sizeof(unsigned char)*4);
    mesh_.normals = (float*)MemAlloc(mesh_.vertexCount*sizeof(float)*3);

    std::memcpy(mesh_.vertices,vertices.data(),mesh_.vertexCount*sizeof(float)*3);
    std::memcpy(mesh_.indices,indices.data(),mesh_.triangleCount*sizeof(unsigned short)*3);
    std::memcpy(mesh_.colors,colors.data(),mesh_.vertexCount*sizeof(unsigned char)*4);
    std::memcpy(mesh_.normals,normals.data(),mesh_.vertexCount*sizeof(float)*3);

    UploadMesh(&mesh_,false);
    update_matrix();
    INFO("Generated Lily mesh");
}

void Lily::update_matrix() {
    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixMultiply(QuaternionToMatrix(quaternion_), MatrixTranslate(position_.x, position_.y, position_.z)));
    assert(flower_transforms_.size() == flower_transforms_base_.size());
    std::mt19937_64 rng (seed_);
    std::uniform_real_distribution roll (0.4f,0.6f);
    for (int i = 0; i < flower_transforms_.size(); i++) {
        Matrix base_rotate = flower_transforms_base_[i].first;
        Matrix base_offset = flower_transforms_base_[i].second;
        Vector3 flower_offset = Vector3Scale(Vector3{base_offset.m12,base_offset.m13,base_offset.m14},scale_);
        flower_offset = Vector3RotateByQuaternion(flower_offset,quaternion_);

        float scale = roll(rng);
        Matrix transform = MatrixMultiply(MatrixMultiply(MatrixScale(scale_,scale_,scale_),MatrixScale(scale, scale, scale)),MatrixMultiply(MatrixMultiply(base_rotate,QuaternionToMatrix(quaternion_)), MatrixTranslate(position_.x+flower_offset.x, position_.y+flower_offset.y, position_.z+flower_offset.z)));
        flower_transforms_[i] = transform;
    }
    for (int i = 0; i < leaf_transforms_.size(); i++) {
        Matrix base_rotate = leaf_transforms_base_[i].first;
        Matrix base_offset = leaf_transforms_base_[i].second;
        Vector3 leaf_offset = Vector3Scale(Vector3{base_offset.m12,base_offset.m13,base_offset.m14},scale_);
        leaf_offset = Vector3RotateByQuaternion(leaf_offset,quaternion_);

        float scale = roll(rng)*1.5f;
        Matrix transform = MatrixMultiply(MatrixMultiply(MatrixScale(scale_,scale_,scale_),MatrixScale(scale, scale, scale)),MatrixMultiply(MatrixMultiply(base_rotate,QuaternionToMatrix(quaternion_)), MatrixTranslate(position_.x+leaf_offset.x, position_.y+leaf_offset.y, position_.z+leaf_offset.z)));
        leaf_transforms_[i] = transform;
    }
}