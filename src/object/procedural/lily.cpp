#include "lily.hpp"

#include "raymath.h"
#include "rlgl.h"

#include <random>
#include <memory>

#include "logging.hpp"

Lily::Lily() : Lily(std::random_device{}()) {}
Lily::Lily(uint32_t seed) : LSystemObject(seed), flower_(std::make_unique<LilyFlower>(seed)) {}
Lily::Lily(Quaternion quaternion, Vector3 position, float scale) : Lily(quaternion,position,scale,std::random_device{}()) {}
Lily::Lily(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : LSystemObject(quaternion,position,scale,seed), flower_(std::make_unique<LilyFlower>(seed)) {}
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
        for (int i = 0; i < 3; i++)
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

    if (lsystem_.get_base() == nullptr) {
        lsystem_ = LSystem(std::make_shared<LNode>("Seed",position_,Vector3{0,1,0},std::vector<std::shared_ptr<LNode>>{},nullptr));
        stage_ = 1;
    }

    RuleSet one_to_two {};
        std::function<std::vector<std::shared_ptr<LNode>>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> germinate = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
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
            std::shared_ptr<LNode> stem = std::make_shared<LNode>("Stem",Vector3Scale(node->direction,length),node->direction,std::vector<std::shared_ptr<LNode>>{},node);
            std::vector<std::shared_ptr<LNode>> result {};
            result.push_back(stem);
            return result;
        };
        one_to_two.add_rule("Seed",Rule(germinate));
    stage_transitions_[std::pair<int,int>{1,2}] = one_to_two;

    RuleSet two_to_three {};
        std::function<std::vector<std::shared_ptr<LNode>>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> juvenile = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::uniform_real_distribution gen (0.0,1.0);
            float roll1 = gen(rng);
            if (roll1 < 0.3f)
                return std::vector<std::shared_ptr<LNode>>{};
            std::uniform_real_distribution angle (0.0f,2*PI);
            Vector3 perp = Vector3Perpendicular(node->direction);
            perp = Vector3RotateByAxisAngle(perp,node->direction,angle(rng));
            std::uniform_real_distribution tilt (0.0f,15.0f*DEG2RAD);
            Vector3 new_direction = Vector3Normalize(Vector3RotateByAxisAngle(node->direction,perp,tilt(rng)));

            float roll = gen(rng);
            float length;
            if (roll < 0.05) {
                length = 0.05f;
            } else if (roll < 0.8) {
                length = 0.15f;
            } else {
                length = 0.3f;
            }
            std::shared_ptr<LNode> new_stem = std::make_shared<LNode>("Stem",Vector3Scale(new_direction,length),new_direction,node->children,node);
            node->children.clear();
            std::vector<std::shared_ptr<LNode>> result {};
            result.push_back(new_stem);
            return result;
        };
        two_to_three.add_rule("Stem",Rule(juvenile));
    stage_transitions_[std::pair<int,int>{2,3}] = two_to_three;

    RuleSet three_to_four {};
        std::function<std::vector<std::shared_ptr<LNode>>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> mature = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            int stem_children = 0;
            for (auto c : node->children)
                stem_children += (c->type=="Stem") || (c->type=="Flower" && c->direction == node->direction);
            if (stem_children)
                return std::vector<std::shared_ptr<LNode>>{};
            std::uniform_real_distribution gen (0.0,1.0);
            float roll = gen(rng);
            std::vector<std::shared_ptr<LNode>> result {};
            if (roll < 0.8) { // Branch and bloom
                std::uniform_real_distribution angle (0.0f,2*PI);
                Vector3 perp = Vector3Perpendicular(node->direction);
                perp = Vector3RotateByAxisAngle(perp,node->direction,angle(rng));
                std::uniform_real_distribution tilt (45.0f*DEG2RAD,90.0f*DEG2RAD);
                Vector3 flower_dir = Vector3RotateByAxisAngle(node->direction,perp,tilt(rng));
                float length = 0.25f;

                std::shared_ptr<LNode> flower = std::make_shared<LNode>("Flower",Vector3Scale(flower_dir,length),flower_dir,std::vector<std::shared_ptr<LNode>>{},node);
                result.push_back(flower);
            } else {
                float length = 0.25f;
                std::shared_ptr<LNode> flower = std::make_shared<LNode>("Flower",Vector3Scale(node->direction,length),node->direction,std::vector<std::shared_ptr<LNode>>{},node);
                result.push_back(flower);
            }
            return result;
        };
        three_to_four.add_rule("Stem",Rule(mature));
    stage_transitions_[std::pair<int,int>{3,4}] = three_to_four;
    INFO("Initialized Lily with Seed LNode and stage: " + std::to_string(stage_));
}

void Lily::draw(Game& game, Material material) const {
    DrawMesh(mesh_,material_,transform_);
    flower_->draw_instanced(game,flower_transforms_.data(),flower_transforms_.size());
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
    stage_ = j.at("stage");
    lsystem_ = LSystem(j.at("lsystem"));
}

static void generate_stem_segment(std::vector<float>& vertices,
                                    std::vector<float>& normals,
                                    std::vector<unsigned char>& colors,
                                    std::vector<unsigned short>& indices,
                                    Vector3 start_pos,
                                    Vector3 end_pos) {
    Vector3 direction = Vector3Normalize(Vector3Subtract(end_pos,start_pos));
    Vector3 normal = Vector3Normalize(Vector3Perpendicular(direction));

    float length = Vector3Length(Vector3Subtract(end_pos,start_pos));
    float width = 0.025f;
    float step_size = 0.1f;
    int vertexes_per_unit = 8;
    for (int step = 1; step < int(length/step_size); step++) {
        Vector3 stem_pos = Vector3Add(start_pos,Vector3Scale(direction,step*step_size));
        Vector3 stem_pos_prev = Vector3Add(start_pos,Vector3Scale(direction,step_size*(step-1)));
        // Generate circle of vertices for prev pos
        int bottom_vertex_index = vertices.size()/3;
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

            Color stem_color = DARKGREEN;
            colors.push_back((uint8_t)stem_color.r);
            colors.push_back((uint8_t)stem_color.g);
            colors.push_back((uint8_t)stem_color.b);
            colors.push_back((uint8_t)stem_color.a);
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

            Color stem_color = DARKGREEN;
            colors.push_back((uint8_t)stem_color.r);
            colors.push_back((uint8_t)stem_color.g);
            colors.push_back((uint8_t)stem_color.b);
            colors.push_back((uint8_t)stem_color.a);
        }

        // Connecting the vertices with triangles
        for (int quad = 0; quad < vertexes_per_unit; quad++) {
            unsigned short bottom_left = bottom_vertex_index+quad;
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
    Vector3 stem_pos = end_pos;
    Vector3 stem_pos_prev;
    if (int(length/step_size))
        stem_pos_prev = Vector3Add(start_pos,Vector3Scale(direction,step_size*(int(length/step_size)-1)));
    else
        stem_pos_prev = start_pos;
    // Generate circle of vertices for prev pos
    int bottom_vertex_index = vertices.size()/3;
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

        Color stem_color = DARKGREEN;
        colors.push_back((uint8_t)stem_color.r);
        colors.push_back((uint8_t)stem_color.g);
        colors.push_back((uint8_t)stem_color.b);
        colors.push_back((uint8_t)stem_color.a);
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

        Color stem_color = DARKGREEN;
        colors.push_back((uint8_t)stem_color.r);
        colors.push_back((uint8_t)stem_color.g);
        colors.push_back((uint8_t)stem_color.b);
        colors.push_back((uint8_t)stem_color.a);
    }

    // Connecting the vertices with triangles
    for (int quad = 0; quad < vertexes_per_unit; quad++) {
        unsigned short bottom_left = bottom_vertex_index+quad;
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

struct StackFrame {
    Vector3 prev_pos;
    Vector3 pos;
    std::shared_ptr<LNode> node;
};

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
    dfs.push_back({Vector3{0,0,0},Vector3{0,0,0},lsystem_.get_base()});
    flower_transforms_.clear();
    flower_transforms_base_.clear();
    while (!dfs.empty()) {
        auto top = dfs.back();
        if (top.node->type == "Stem") {
            generate_stem_segment(vertices,normals,colors,indices,top.prev_pos,top.pos);
        } else if (top.node->type == "Flower") {
            generate_stem_segment(vertices,normals,colors,indices,top.prev_pos,top.pos);
            Matrix rot = QuaternionToMatrix(QuaternionFromVector3ToVector3(Vector3{0,1,0},top.node->direction));
            Matrix translate = MatrixTranslate(top.pos.x,top.pos.y,top.pos.z);
            flower_transforms_.push_back(MatrixIdentity());
            flower_transforms_base_.push_back({rot,translate});
        }

        dfs.pop_back();
        for (auto child : top.node->children) {
            dfs.push_back({top.pos,Vector3Add(top.pos,child->position),child});
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
}