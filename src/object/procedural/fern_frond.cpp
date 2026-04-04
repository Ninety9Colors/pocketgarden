#include <cassert>
#include <functional>
#include <map>

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
    DrawMesh(mesh_,material_,transform_);
    leaf_->draw_instanced(game,leaf_transforms_.data(),leaf_transforms_.size());
}
// void FernFrond::draw(Game& game,Matrix transform, Material material) const {
    
// }

// void FernFrond::draw_instanced(Game& game, Material material, const Matrix* transforms, int matrix_count) const {
    
// }

void FernFrond::grow() {
    std::string initial = lsystem_.to_string();
    lsystem_.apply_ruleset(productions_,rng_);
    DEBUG("Grew fern frond l system from " + initial + " to " + lsystem_.to_string());
}

void FernFrond::initialize() {
    // TODO: make leaf shape correct & generate its mesh
    leaf_->generate_mesh();
    if (lsystem_.get_base() == nullptr)
        lsystem_ = LSystem(std::make_shared<LNode>("B",position_,Vector3{1,0,0},std::vector<std::shared_ptr<LNode>>{},nullptr));
        
        // base rule
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> base = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::shared_ptr<LNode> stipe_base = std::make_shared<LNode>("S0",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},nullptr);
            std::shared_ptr<LNode> stipe = std::make_shared<LNode>("S",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},stipe_base);
            std::shared_ptr<LNode> rachi = std::make_shared<LNode>("R",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},stipe);
            std::shared_ptr<LNode> head = std::make_shared<LNode>("H0",node->position,node->direction,std::vector<std::shared_ptr<LNode>>{},rachi);
            stipe_base->children.push_back(stipe);
            stipe->children.push_back(rachi);
            rachi->children.push_back(head);
            return stipe_base;
        };
        productions_.add_rule("B",Rule(base));

        // stipe growth rule
        std::function<std::shared_ptr<LNode>(std::shared_ptr<LNode> node, std::mt19937_64& rng)> stipe_growth = [](std::shared_ptr<LNode> node, std::mt19937_64& rng){
            std::shared_ptr<LNode> stipe = std::make_shared<LNode>("S",node->position,node->direction,node->children,node);
            for (auto child : stipe->children)
                child->parent = stipe;
            node->children.clear();
            node->children.push_back(stipe);
            return node;
        };
        productions_.add_rule("S0",Rule(stipe_growth));

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
}

void FernFrond::update_matrix() {
    transform_ = MatrixMultiply(MatrixScale(scale_, scale_, scale_),MatrixMultiply(QuaternionToMatrix(quaternion_), MatrixTranslate(position_.x, position_.y, position_.z)));
    for (int i = 0; i < leaf_transforms_.size(); i++) {
        Matrix base_rotate = leaf_transforms_base_[i][0];
        Matrix base_offset = leaf_transforms_base_[i][1];
        Matrix base_scale = leaf_transforms_base_[i][2]; // scale of the Leaf, not whole object
        Vector3 leaf_offset = Vector3Scale(Vector3{base_offset.m12,base_offset.m13,base_offset.m14},scale_);
        leaf_offset = Vector3RotateByQuaternion(leaf_offset,quaternion_);

        float scale = 1.0f;
        Matrix transform = MatrixMultiply(MatrixMultiply(MatrixMultiply(MatrixScale(scale_,scale_,scale_),base_scale),MatrixScale(scale, scale, scale)),MatrixMultiply(MatrixMultiply(base_rotate,QuaternionToMatrix(quaternion_)), MatrixTranslate(position_.x+leaf_offset.x, position_.y+leaf_offset.y, position_.z+leaf_offset.z)));
        leaf_transforms_[i] = transform;
    }
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
    DEBUG("Generating stem segment");
    DEBUG(std::to_string(start_pos.x) + "," + std::to_string(start_pos.y) + "," + std::to_string(start_pos.z));
    DEBUG(std::to_string(end_pos.x) + "," + std::to_string(end_pos.y) + "," + std::to_string(end_pos.z));
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
        Vector3 prev_dir;
    };
}

// r = e^(-theta)
static float polar_x_to_y(float x) {
    INFO("Converting float x=" + std::to_string(x) + " to polar..");
    constexpr float epsilon = 0.01f;
    float theta_min = 0;
    float theta_max = PI/2.0f;
    while (theta_max - theta_min > epsilon) {
        float mid = (theta_max + theta_min)/2.0f;
        float r = std::expf(-mid);
        float curr_x = r*std::cosf(mid);
        float curr_y = r*std::sinf(mid);
        if (curr_x > x) {
            theta_min = mid;
        } else if (curr_x < x) {
            theta_max = mid;
        } else {
            INFO("Result y=" + std::to_string(curr_y));
            return curr_y;
        }
    }
    float r = std::expf(-theta_min);
    INFO("Result y=" + std::to_string(r*std::sinf(theta_min)));
    return r*std::sinf(theta_min);
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
                                    ParameterMap& parameters,
                                    const Vector3& leaf_tip_vector,
                                    std::vector<std::array<Matrix,3>>& leaf_transforms_base,
                                    std::vector<Matrix>& leaf_transforms,
                                    std::unordered_map<std::shared_ptr<LNode>,float>& proportions,
                                    float segment_scale = 1.0f,
                                    float child_segment_scale = 1.0f) {
    float mesh_scale = parameters.get_parameter("MeshScale").value;
    float stipe_prop = parameters.get_parameter("StipeProportion").value;
    float rachi_prop = parameters.get_parameter("RachiProportion").value;
    stipe_prop *= mesh_scale;
    rachi_prop *= mesh_scale;
    float branch_angle = parameters.get_parameter("BranchAngle").value*DEG2RAD;
    std::deque<StackFrame> dfs {};
    dfs.push_back(StackFrame{base_frame.prev_pos,base_frame.pos,base_frame.normal,base_frame.dir,base_node,base_frame.id,base_frame.prev_id,base_frame.prev_dir});
    std::shared_ptr<LNode> latest_top = nullptr;
    while (!dfs.empty()) {
        auto top = dfs.back();
        latest_top = top.node;
        dfs.pop_back();
        for (auto child : top.node->children) {
            if (child->type == "B") {
            } else if (child->type == "S") {
                DEBUG("S");
                Vector3 new_pos = Vector3Add(top.pos,Vector3Scale(top.dir,stipe_prop*segment_scale));
                generate_stem_segment(vertices,normals,colors,indices,top.pos,new_pos,top.dir,top.dir,top.id,next_id++,previous_vertexes,color,2.0f*0.025f*mesh_scale);
                dfs.push_back({top.pos,new_pos,top.normal,top.dir,child,next_id-1,top.id,top.prev_dir});
            } else if (child->type == "R") {
                DEBUG("R");
                if (proportions.contains(child))
                    child_segment_scale = polar_x_to_y(proportions[child]);
                Vector3 new_pos = Vector3Add(top.pos,Vector3Scale(top.dir,rachi_prop*segment_scale));
                generate_stem_segment(vertices,normals,colors,indices,top.pos,new_pos,top.dir,top.dir,top.id,next_id++,previous_vertexes,color,2.0f*0.025f*mesh_scale);
                dfs.push_back({top.pos,new_pos,top.normal,top.dir,child,next_id-1,top.id,top.prev_dir});
            } else if (child->type == "H0") {
                DEBUG("H0");
                Vector3 leaf_pos = Vector3Subtract(top.pos,Vector3Scale(top.dir,0.001f));

                // align normals
                Vector3 leaf_tip_normal = Vector3Normalize(Vector3CrossProduct(Vector3CrossProduct(leaf_tip_vector,{0,1,0}),leaf_tip_vector));
                Quaternion normal_align = QuaternionFromVector3ToVector3(leaf_tip_normal,top.normal);
                Vector3 tip_two = Vector3RotateByQuaternion(leaf_tip_vector,normal_align);

                // align tips
                Vector3 cross = Vector3CrossProduct(tip_two,top.dir);
                int sign = (Vector3DotProduct(cross,top.normal) >= 0) ? 1 : -1;
                Quaternion tip_align = QuaternionFromAxisAngle(top.normal,Vector3Angle(top.dir,tip_two)*sign);

                Matrix rot = QuaternionToMatrix(QuaternionMultiply(tip_align,normal_align));
                Matrix translate = MatrixTranslate(leaf_pos.x,leaf_pos.y,leaf_pos.z);
                Matrix scale = MatrixScale(2.0f*child_segment_scale*mesh_scale,2.0f*child_segment_scale*mesh_scale,2.0f*child_segment_scale*mesh_scale);
                leaf_transforms.push_back(MatrixIdentity());
                leaf_transforms_base.push_back({rot,translate,scale});
                dfs.push_back({top.prev_pos,top.pos,top.normal,top.dir,child,top.id,top.prev_id,top.prev_dir});
            } else if (child->type == "H1") {
                DEBUG("H1");
                Vector3 leaf_pos = Vector3Subtract(top.pos,Vector3Scale(top.dir,0.001f));

                // align normals
                Vector3 leaf_tip_normal = Vector3Normalize(Vector3CrossProduct(Vector3CrossProduct(leaf_tip_vector,{0,1,0}),leaf_tip_vector));
                Quaternion normal_align = QuaternionFromVector3ToVector3(leaf_tip_normal,top.normal);
                Vector3 tip_two = Vector3RotateByQuaternion(leaf_tip_vector,normal_align);

                // align tips
                Vector3 cross = Vector3CrossProduct(tip_two,top.dir);
                int sign = (Vector3DotProduct(cross,top.normal) >= 0) ? 1 : -1;
                Quaternion tip_align = QuaternionFromAxisAngle(top.normal,Vector3Angle(top.dir,tip_two)*sign);

                Matrix rot = QuaternionToMatrix(QuaternionMultiply(tip_align,normal_align));
                Matrix translate = MatrixTranslate(leaf_pos.x,leaf_pos.y,leaf_pos.z);
                Matrix scale = MatrixScale(2.0f*segment_scale*mesh_scale,2.0f*segment_scale*mesh_scale,2.0f*segment_scale*mesh_scale);
                leaf_transforms.push_back(MatrixIdentity());
                leaf_transforms_base.push_back({rot,translate,scale});
                dfs.push_back({top.prev_pos,top.pos,top.normal,top.dir,child,top.id,top.prev_id,top.prev_dir});
            } else if (child->type == "H2") {
                DEBUG("H2");
                Vector3 leaf_pos = Vector3Subtract(top.pos,Vector3Scale(top.dir,0.001f));

                // align normals
                Vector3 leaf_tip_normal = Vector3Normalize(Vector3CrossProduct(Vector3CrossProduct(leaf_tip_vector,{0,1,0}),leaf_tip_vector));
                Quaternion normal_align = QuaternionFromVector3ToVector3(leaf_tip_normal,top.normal);
                Vector3 tip_two = Vector3RotateByQuaternion(leaf_tip_vector,normal_align);

                // align tips
                Vector3 cross = Vector3CrossProduct(tip_two,top.dir);
                int sign = (Vector3DotProduct(cross,top.normal) >= 0) ? 1 : -1;
                Quaternion tip_align = QuaternionFromAxisAngle(top.normal,Vector3Angle(top.dir,tip_two)*sign);

                Matrix rot = QuaternionToMatrix(QuaternionMultiply(tip_align,normal_align));
                Matrix translate = MatrixTranslate(leaf_pos.x,leaf_pos.y,leaf_pos.z);
                Matrix scale = MatrixScale(2.0f*segment_scale*mesh_scale,2.0f*segment_scale*mesh_scale,2.0f*segment_scale*mesh_scale);
                leaf_transforms.push_back(MatrixIdentity());
                leaf_transforms_base.push_back({rot,translate,scale});
                dfs.push_back({top.prev_pos,top.pos,top.normal,top.dir,child,top.id,top.prev_id,top.prev_dir});
            } else if (child->type == "+") {
                DEBUG("+");
                Vector3 new_dir = Vector3Normalize(Vector3RotateByAxisAngle(top.dir,top.normal,branch_angle));
                dfs.push_back({top.prev_pos,top.pos,top.normal,new_dir,child,top.id,top.prev_id,top.dir});
            } else if (child->type == "-") {
                DEBUG("-");
                Vector3 new_dir = Vector3Normalize(Vector3RotateByAxisAngle(top.dir,top.normal,-branch_angle));
                dfs.push_back({top.prev_pos,top.pos,top.normal,new_dir,child,top.id,top.prev_id,top.dir});
            } else if (child->type == "[") {
                DEBUG("[");
                std::shared_ptr<LNode> next = generate_rachis(vertices,normals,colors,indices,previous_vertexes,color,next_id,child,
                    {top.prev_pos,top.pos,top.normal,top.dir,child,next_id++,top.id}
                    ,parameters,leaf_tip_vector,leaf_transforms_base,leaf_transforms,proportions,child_segment_scale,child_segment_scale);
                if (next != nullptr)
                    dfs.push_back({top.prev_pos,top.pos,top.normal,top.dir,next,top.id,top.prev_id,top.prev_dir});
            } else if (child->type == "]") {
                DEBUG("]");
                return child;
            }
        }
    }
    return latest_top;
}

static void fill_proportion_map(ParameterMap& parameters,
                                    std::shared_ptr<LNode> base_node,
                                    std::unordered_map<std::shared_ptr<LNode>,float>& proportions) {
    std::deque<std::shared_ptr<LNode>> dfs {};
    float rachi_prop = parameters.get_parameter("RachiProportion").value;
    float mesh_scale = parameters.get_parameter("MeshScale").value;
    dfs.push_back(base_node);
    int depth = 0;
    int rachi_count = 1;
    while (!dfs.empty()) {
        auto top = dfs.back();
        dfs.pop_back();
        for (auto child : top->children) {
            if (child->type == "[") {
                depth++;
            } else if (child->type == "]") {
                depth--;
            } else if (child->type == "R") {
                if (depth == 0) rachi_count++;
            }
            dfs.push_back(child);
        }
    }
    float total_length = rachi_prop*mesh_scale*rachi_count;
    float curr_length = 0.0f;
    dfs.push_back(base_node);
    while (!dfs.empty()) {
        auto top = dfs.back();
        dfs.pop_back();
        for (auto child : top->children) {
            if (child->type == "[") {
                depth++;
            } else if (child->type == "]") {
                depth--;
            } else if (child->type == "R") {
                if (depth == 0) {
                    proportions[child] = curr_length/total_length;
                    curr_length += rachi_prop*mesh_scale;
                }
            }
            dfs.push_back(child);
        }
    }
    for (const auto& p : proportions)
        INFO(std::to_string(p.second));
}

void FernFrond::generate_mesh() {
    DEBUG("Generating fern mesh...");
    if (mesh_.vboId != 0)
        UnloadMesh(mesh_);
    rng_ = std::mt19937_64(seed_);
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

    DEBUG("... beginning recursion ...");
    StackFrame base_frame = {{0,0,0},{0,0,0},{0,1,0},{1,0,0},lsystem_.get_base(),0,-1,{1,0,0}};
    std::unordered_map<std::shared_ptr<LNode>,float> proportions {};
    fill_proportion_map(parameter_map_,lsystem_.get_base(),proportions);
    generate_rachis(vertices,normals,colors,indices,previous_vertexes,stem_color,next_id,lsystem_.get_base(),base_frame,parameter_map_,leaf_->tip_vector(),leaf_transforms_base_,leaf_transforms_,proportions);
    DEBUG("... done recursing ...");

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
    parameter_map_.set_parameter("StipeProportion", Parameter(0.2f,0.2f,0.5f));
    parameter_map_.set_parameter("RachiProportion", Parameter(0.2f,0.7f,0.5f));
    parameter_map_.set_parameter("BranchAngle", Parameter(60.0f,75.0f,85.0f));
    parameter_map_.set_parameter("MeshScale", Parameter(0.05f,0.1f,0.25f));
}