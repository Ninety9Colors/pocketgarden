#include <cassert>
#include <functional>

#include "object/procedural/fern_frond.hpp"

#include "raymath.h"

#include "logging.hpp"

constexpr int DEFAULT_SLICES_X = 40;
constexpr int DEFAULT_SLICES_Y = 20;

FernFrond::FernFrond() : FernFrond(std::random_device{}()) {}
FernFrond::FernFrond(uint32_t seed) : ParameterObject(seed), rng_(seed), slices_(DEFAULT_SLICES_X, DEFAULT_SLICES_Y) {
    leaf_ = std::make_unique<TaperedLeaf>(seed);
    leaf_->set_slices(slices_);
    initialize_parameters();
}
FernFrond::FernFrond(Quaternion quaternion, Vector3 position, float scale) : FernFrond(quaternion,position,scale,std::random_device{}()) {}
FernFrond::FernFrond(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : ParameterObject(quaternion, position, scale,seed), rng_(seed), slices_(DEFAULT_SLICES_X, DEFAULT_SLICES_Y) {
    leaf_ = std::make_unique<TaperedLeaf>(seed);
    leaf_->set_slices(slices_);
    initialize_parameters();
}
FernFrond::FernFrond(const json& j) {
    from_json(j);
    initialize_parameters();
}

json FernFrond::to_json() const {
    json j = {
        {"type","FernFrond"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"scale",scale_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}},
        {"seed",seed_},
        {"parameter_map",parameter_map_.to_json()},
        {"slices",{{"first",slices_.first},{"second",slices_.second}}},
        {"leaf",leaf_->to_json()},
        {"lsystem",lsystem_.to_json()}
    };
    return j;
}

void FernFrond::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    scale_ = j.at("scale");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
    seed_ = j.at("seed");
    parameter_map_ = ParameterMap{j.at("parameter_map")};
    slices_ = {j.at("slices")["first"],j.at("slices")["second"]};
    leaf_ = std::make_unique<TaperedLeaf>(j.at("leaf"));
    leaf_->set_slices(slices_);
    rng_ = std::mt19937_64(seed_);
    lsystem_ = LSystem(j.at("lsystem"));
}

void FernFrond::draw(Game& game, Material material) const {
    
}
void FernFrond::draw(Game& game,Matrix transform, Material material) const {
    
}

void FernFrond::draw_instanced(Game& game, Material material, const Matrix* transforms, int matrix_count) const {
    
}

void FernFrond::grow() {
    std::string initial = lsystem_.to_string();
    lsystem_.apply_ruleset(productions_,rng_);
    DEBUG("Grew fern frond l system from " + initial + " to " + lsystem_.to_string());
}

void FernFrond::initialize() {
    // TODO: make leaf shape correct & generate its mesh
    if (lsystem_.get_base() == nullptr)
        lsystem_ = LSystem(std::make_shared<LNode>("B",position_,Vector3{1,0,0},std::vector<std::shared_ptr<LNode>>{},nullptr));
        
        // base rule
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> base = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::shared_ptr<LNode> stipe = std::make_shared<LNode>("S",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},nullptr);
            std::shared_ptr<LNode> rachi = std::make_shared<LNode>("R",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},stipe);
            std::shared_ptr<LNode> head = std::make_shared<LNode>("H0",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},rachi);
            stipe->children.push_back(rachi);
            rachi->children.push_back(head);
            return stipe;
        };
        productions_.add_rule("B",Rule(base));

        // branching rule one
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> branching_one = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::shared_ptr<LNode> one = std::make_shared<LNode>("R",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},nullptr);
            std::shared_ptr<LNode> two = std::make_shared<LNode>("[",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},one);
            std::shared_ptr<LNode> three = std::make_shared<LNode>("-",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},two);
            std::shared_ptr<LNode> four = std::make_shared<LNode>("H1",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},three);
            std::shared_ptr<LNode> five = std::make_shared<LNode>("]",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},four);
            std::shared_ptr<LNode> six = std::make_shared<LNode>("[",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},five);
            std::shared_ptr<LNode> seven = std::make_shared<LNode>("+",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},six);
            std::shared_ptr<LNode> eight = std::make_shared<LNode>("H1",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},seven);
            std::shared_ptr<LNode> nine = std::make_shared<LNode>("]",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},eight);
            std::shared_ptr<LNode> ten = std::make_shared<LNode>("H0",node->position,node->direction,node->children,nine);
            one->children.push_back(two);
            two->children.push_back(three);
            three->children.push_back(four);
            four->children.push_back(five);
            five->children.push_back(six);
            six->children.push_back(seven);
            seven->children.push_back(eight);
            eight->children.push_back(nine);
            nine->children.push_back(ten);
            for (auto child : ten->children)
                child->parent = ten;
            return one;
        };
        productions_.add_rule("H0",Rule(branching_one));

        // branching rule two
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> branching_two = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::shared_ptr<LNode> one = std::make_shared<LNode>("R",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},nullptr);
            std::shared_ptr<LNode> two = std::make_shared<LNode>("[",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},one);
            std::shared_ptr<LNode> three = std::make_shared<LNode>("-",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},two);
            std::shared_ptr<LNode> four = std::make_shared<LNode>("H2",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},three);
            std::shared_ptr<LNode> five = std::make_shared<LNode>("]",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},four);
            std::shared_ptr<LNode> six = std::make_shared<LNode>("[",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},five);
            std::shared_ptr<LNode> seven = std::make_shared<LNode>("+",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},six);
            std::shared_ptr<LNode> eight = std::make_shared<LNode>("H2",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},seven);
            std::shared_ptr<LNode> nine = std::make_shared<LNode>("]",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},eight);
            std::shared_ptr<LNode> ten = std::make_shared<LNode>("H1",node->position,node->direction,node->children,nine);
            one->children.push_back(two);
            two->children.push_back(three);
            three->children.push_back(four);
            four->children.push_back(five);
            five->children.push_back(six);
            six->children.push_back(seven);
            seven->children.push_back(eight);
            eight->children.push_back(nine);
            nine->children.push_back(ten);
            for (auto child : ten->children)
                child->parent = ten;
            return one;
        };
        productions_.add_rule("H1",Rule(branching_two));
        // growth rule
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> growth = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::shared_ptr<LNode> stem = std::make_shared<LNode>("S",node->position,node->direction,node->children,node);
            node->children.clear();
            node->children.push_back(stem);
            for (auto child : stem->children)
                child->parent = stem;
            return node;
        };
        productions_.add_rule("S",Rule(growth));
}

void FernFrond::update_matrix() {
    // TODO: update matrix
}

BoundingBox FernFrond::get_bounding_box() const {
    // TODO: get bounding box
    return BoundingBox{};
}
BoundingBox FernFrond::get_bounding_box(Matrix transform) const {
    Matrix m = MatrixMultiply(transform_, transform);
    // TODO: get bounding box
    return BoundingBox{};
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
        Vector3 normal;
        Vector3 dir;
        std::shared_ptr<LNode> node;
        int id;
        int prev_id;
    };
}

static std::shared_ptr<LNode> generate_rachis(std::vector<float>& vertices,
                                    std::vector<float>& normals,
                                    std::vector<unsigned char>& colors,
                                    std::vector<unsigned short>& indices,
                                    std::map<int,std::pair<int,Vector3>>& previous_vertexes,
                                    Color color,
                                    int& next_id,
                                    std::shared_ptr<LNode> base_node,
                                    StackFrame base_frame,
                                    ParameterMap& parameters) {
    float stipe_prop = parameters.get_parameter("StipeProportion").value;
    float rachi_prop = 1-stipe_prop;
    float branch_angle = parameters.get_parameter("BranchAngle").value;
    std::deque<StackFrame> dfs {};
    dfs.push_back({base_frame.prev_pos,base_frame.pos,base_frame.normal,base_frame.dir,base_node,base_frame.id,base_frame.prev_id});
    while (!dfs.empty()) {
        auto top = dfs.back();
        dfs.pop_back();
        for (auto child : top.node->children) {
            assert(child->children.size() == 1);
            if (child->type == "B") {
            } else if (child->type == "S") {
                Vector3 new_pos = Vector3Add(top.pos,Vector3Scale(top.dir,stipe_prop));
                generate_stem_segment(vertices,normals,colors,indices,top.pos,new_pos,top.dir,top.dir,top.id,next_id++,previous_vertexes,color);
                dfs.push_back({top.pos,new_pos,top.normal,top.dir,child,next_id-1,top.id});
            } else if (child->type == "R") {
                Vector3 new_pos = Vector3Add(top.pos,Vector3Scale(top.dir,rachi_prop));
                generate_stem_segment(vertices,normals,colors,indices,top.pos,new_pos,top.dir,top.dir,top.id,next_id++,previous_vertexes,color);
                dfs.push_back({top.pos,new_pos,top.normal,top.dir,child,next_id-1,top.id});
            } else if (child->type == "H0") {
                // TODO: add leaf transform
            } else if (child->type == "H1") {
                // TODO: add leaf transform
            } else if (child->type == "H2") {
                // TODO: add leaf transform
            } else if (child->type == "+") {
                Vector3 new_dir = Vector3Normalize(Vector3RotateByAxisAngle(top.dir,top.normal,branch_angle));
                dfs.push_back({top.prev_pos,top.pos,top.normal,new_dir,child,top.id,top.prev_id});
            } else if (child->type == "-") {
                Vector3 new_dir = Vector3Normalize(Vector3RotateByAxisAngle(top.dir,top.normal,-branch_angle));
                dfs.push_back({top.prev_pos,top.pos,top.normal,new_dir,child,top.id,top.prev_id});
            } else if (child->type == "[") {
                std::shared_ptr<LNode> next = generate_rachis(vertices,normals,colors,indices,previous_vertexes,color,next_id,child,
                    {top.prev_pos,top.pos,top.normal,top.dir,child,next_id++,top.id}
                    ,parameters);
                dfs.push_back({top.prev_pos,top.pos,top.normal,top.dir,next,top.id,top.prev_id});
            } else if (child->type == "]") {
                return child;
            }
        }
    }
    return nullptr;
}

void FernFrond::generate_mesh() {
    if (mesh_.vboId != 0)
        UnloadMesh(mesh_);
    mesh_ = Mesh{0};
    std::vector<float> vertices {};
    std::vector<float> normals {};
    std::vector<unsigned char> colors {};
    std::vector<unsigned short> indices {};

    assert(lsystem_.get_base() != nullptr);
    int next_id = 1;
    std::map<int,std::pair<int,Vector3>> previous_vertexes {};

    Color stem_color = ColorFromHSV(leaf_->get_parameter("BaseHue").value,leaf_->get_parameter("BaseSaturation").value,leaf_->get_parameter("BaseValue").value);
    leaf_transforms_.clear();
    leaf_transforms_base_.clear();

    StackFrame base_frame = {{0,0,0},{0,0,0},{0,1,0},{1,0,0},lsystem_.get_base(),0,-1};
    generate_rachis(vertices,normals,colors,indices,previous_vertexes,stem_color,next_id,lsystem_.get_base(),base_frame,parameter_map_);

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
    INFO("Generated Fern mesh");
}

void FernFrond::set_slices(std::pair<int,int> slices) {
    leaf_->set_slices(slices);
}
void FernFrond::initialize_parameters() {
    std::mt19937_64 rng(seed_);
    parameter_map_.set_parameter("StipeProportion", Parameter(0.2f,0.3f,0.5f));
    parameter_map_.set_parameter("BranchAngle", Parameter(60.0f,75.0f,85.0f));
}