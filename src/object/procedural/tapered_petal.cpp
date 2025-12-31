#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>
#include <iostream>
#include <random>

#include "raylib.h"
#include "raymath.h"

#include "object/procedural/tapered_petal.hpp"

// Returns the true float index offset for a point on the petal, not the triplet offset
static int vertex_index(int i, int j, const std::pair<int,int>& slices, bool bottom) {
    return ((slices.second+1)*i + j)*3 + ((slices.first+1)*(slices.second+1)*3*bottom);
}

constexpr int DEFAULT_SLICES_X = 40;
constexpr int DEFAULT_SLICES_Y = 20;

TaperedPetal::TaperedPetal() : TaperedPetal(std::random_device{}()) {}
TaperedPetal::TaperedPetal(uint32_t seed) : ParameterObject(seed), slices_{40,20} {
    initialize_parameters();
}
TaperedPetal::TaperedPetal(Quaternion quaternion, Vector3 position, float scale) : TaperedPetal(quaternion, position, scale, std::random_device{}()) {}
TaperedPetal::TaperedPetal(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : ParameterObject(quaternion,position,scale,seed), slices_{40,20} {
    initialize_parameters();
}
TaperedPetal::TaperedPetal(const json& j) {
    from_json(j);
    // initialize_parameters();
}

json TaperedPetal::to_json() const {
    json j = {
        {"type","TaperedPetal"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"scale",scale_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}},
        {"seed",seed_},
        {"parameter_map",parameter_map_.to_json()},
        {"slices",{{"first",slices_.first},{"second",slices_.second}}}
    };
    return j;
}

void TaperedPetal::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    scale_ = j.at("scale");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
    seed_ = j.at("seed");
    parameter_map_ = ParameterMap{j.at("parameter_map")};
    slices_ = {j.at("slices")["first"],j.at("slices")["second"]};
}

void TaperedPetal::generate_mesh() {

    float length = parameter_map_.get_parameter("Length").value;
    float width = parameter_map_.get_parameter("Width").value;

    float u_step = length/(1.0f*slices_.first);
    float v_step = 2.0f*width/(1.0f*slices_.second);

    // Generate Freckle Positions
    std::vector<unsigned short> freckle_positions {};
    std::mt19937_64 rng(seed_);
    std::uniform_real_distribution<double> dist(0.0f,1.0f);
    for (int i = 0; i <= slices_.first; i++) {
        for (int j = 0; j <= slices_.second; j++) {
            float u = i*u_step;
            int index_top = vertex_index(i,j,slices_,false);
            float roll = dist(rng);
            float freckle_coverage = parameter_map_.get_parameter("FreckleCoverage").value;
            float freckle_chance = parameter_map_.get_parameter("FreckleAmount").value*std::powf(1.0f-u/(length*freckle_coverage),parameter_map_.get_parameter("FreckleCentrality").value);
            if (i > 1 && j > 1 && j < slices_.second-1 && u/length < freckle_coverage && roll < freckle_chance)
                freckle_positions.emplace_back(index_top);
        }
    }

    int petal_vertex_count = (slices_.first+1)*(slices_.second+1)*2; // Dual Sided
    int petal_triangle_count = (slices_.first*slices_.second)*2*2; // Dual Sided

    int freckle_vertex_count = freckle_positions.size()*9;
    int freckle_triangle_count = freckle_positions.size()*8;

    mesh_ = Mesh{0};
    mesh_.vertexCount = petal_vertex_count + freckle_vertex_count;
    mesh_.triangleCount = petal_triangle_count + freckle_triangle_count;
    mesh_.vertices = (float*)MemAlloc(mesh_.vertexCount*sizeof(float)*3);
    mesh_.indices = (unsigned short*)MemAlloc(mesh_.triangleCount*sizeof(unsigned short)*3);
    mesh_.colors = (unsigned char*)MemAlloc(mesh_.vertexCount*sizeof(unsigned char)*4);
    mesh_.normals = (float*)MemAlloc(mesh_.vertexCount*sizeof(float)*3);
    std::memset(mesh_.normals, 0, mesh_.vertexCount*sizeof(float)*3);

    for (int i = 0; i <= slices_.first; i++) {
        for (int j = 0; j <= slices_.second; j++) {
            float u = i*u_step;
            float v = j*v_step - width;
            int index_top = vertex_index(i,j,slices_,false);
            int index_bottom = vertex_index(i,j,slices_,true);
            float x = X(u,v);
            float y = Y(u,v);
            float z = Z(u,v);
            mesh_.vertices[index_top] = x;
            mesh_.vertices[index_top+1] = y;
            mesh_.vertices[index_top+2] = z;

            mesh_.vertices[index_bottom] = x;
            mesh_.vertices[index_bottom+1] = y;
            mesh_.vertices[index_bottom+2] = z;

            float base_hue = parameter_map_.get_parameter("BaseHue").value;
            float base_saturation = parameter_map_.get_parameter("BaseSaturation").value;
            float base_value = parameter_map_.get_parameter("BaseValue").value;

            float gradient_hue = parameter_map_.get_parameter("GradientHue").value;
            float gradient_saturation = parameter_map_.get_parameter("GradientSaturation").value;
            float gradient_value = parameter_map_.get_parameter("GradientValue").value;

            float border_hue = parameter_map_.get_parameter("BorderHue").value;
            float border_saturation = parameter_map_.get_parameter("BorderSaturation").value;
            float border_value = parameter_map_.get_parameter("BorderValue").value;

            float stripe_hue = parameter_map_.get_parameter("StripeHue").value;
            float stripe_saturation = parameter_map_.get_parameter("StripeSaturation").value;
            float stripe_value = parameter_map_.get_parameter("StripeValue").value;

            float gradient_width = parameter_map_.get_parameter("GradientWidth").value;
            float gradient_amount = std::max<float>(gradient_width-(length-u)/length,0.0f)/gradient_width;
            if (gradient_width == 0.0f) gradient_amount = 0.0f;

            float border_width = parameter_map_.get_parameter("BorderWidth").value;
            float border_amount = std::max<float>(border_width-(width-std::abs(v))/width,0.0f)/border_width;
            if (border_width == 0.0f) border_amount = 0.0f;

            float stripe_width = parameter_map_.get_parameter("StripeWidth").value;
            float stripe_amount = std::max<float>(stripe_width-std::abs(v)/width,0.0f)/stripe_width;
            if (stripe_width == 0.0f) stripe_amount = 0.0f;

            float hue = lerp(base_hue,gradient_hue,gradient_amount);
            float saturation = lerp(base_saturation,gradient_saturation,gradient_amount);
            float value = lerp(base_value,gradient_value,gradient_amount);

            hue = lerp(hue,border_hue,border_amount);
            saturation = lerp(saturation,border_saturation,border_amount);
            value = lerp(value,border_value,border_amount);

            hue = lerp(hue,stripe_hue,stripe_amount);
            saturation = lerp(saturation,stripe_saturation,stripe_amount);
            value = lerp(value,stripe_value,stripe_amount);

            Color rgb = ColorFromHSV(std::fmod(hue,360.0f),saturation,value);

            mesh_.colors[4*index_top/3] = (unsigned short) rgb.r;
            mesh_.colors[4*index_top/3+1] = (unsigned short) rgb.g;
            mesh_.colors[4*index_top/3+2] = (unsigned short) rgb.b;
            mesh_.colors[4*index_top/3+3] = 255;

            mesh_.colors[4*index_bottom/3] = (unsigned short) rgb.r;
            mesh_.colors[4*index_bottom/3+1] = (unsigned short) rgb.g;
            mesh_.colors[4*index_bottom/3+2] = (unsigned short) rgb.b;
            mesh_.colors[4*index_bottom/3+3] = 255;
        }
    }

    int triangle_index = 0;
    for (int i = 0; i <= slices_.first-1; i++) {
        for (int j = 0; j <= slices_.second-1; j++) {
            int index = vertex_index(i,j,slices_,false);
            int index_two = vertex_index(i+1,j,slices_,false);
            int index_three = vertex_index(i,j+1,slices_,false);
            int index_four = vertex_index(i+1,j+1,slices_,false);

            mesh_.indices[triangle_index] = index/3;
            mesh_.indices[triangle_index+1] = index_three/3;
            mesh_.indices[triangle_index+2] = index_four/3;

            mesh_.indices[triangle_index+3] = index_two/3;
            mesh_.indices[triangle_index+4] = index/3;
            mesh_.indices[triangle_index+5] = index_four/3;

            Vector3 a = Vector3Subtract(Vector3{mesh_.vertices[index_three],mesh_.vertices[index_three+1],mesh_.vertices[index_three+2]},
                                        Vector3{mesh_.vertices[index],mesh_.vertices[index+1],mesh_.vertices[index+2]});          
            Vector3 b = Vector3Subtract(Vector3{mesh_.vertices[index_four],mesh_.vertices[index_four+1],mesh_.vertices[index_four+2]},
                                        Vector3{mesh_.vertices[index],mesh_.vertices[index+1],mesh_.vertices[index+2]});
            Vector3 c = Vector3Subtract(Vector3{mesh_.vertices[index],mesh_.vertices[index+1],mesh_.vertices[index+2]},
                                        Vector3{mesh_.vertices[index_two],mesh_.vertices[index_two+1],mesh_.vertices[index_two+2]});          
            Vector3 d = Vector3Subtract(Vector3{mesh_.vertices[index_four],mesh_.vertices[index_four+1],mesh_.vertices[index_four+2]},
                                        Vector3{mesh_.vertices[index_two],mesh_.vertices[index_two+1],mesh_.vertices[index_two+2]});
            Vector3 norm = Vector3Normalize(Vector3CrossProduct(a,b));
            if (norm.x == 0 && norm.y == 0 && norm.z == 0)
                norm = Vector3Normalize(Vector3CrossProduct(c,d));
            mesh_.normals[index] += norm.x;
            mesh_.normals[index+1] += norm.y;
            mesh_.normals[index+2] += norm.z;
            mesh_.normals[index_two] += norm.x;
            mesh_.normals[index_two+1] += norm.y;
            mesh_.normals[index_two+2] += norm.z;
            mesh_.normals[index_three] += norm.x;
            mesh_.normals[index_three+1] += norm.y;
            mesh_.normals[index_three+2] += norm.z;
            mesh_.normals[index_four] += norm.x;
            mesh_.normals[index_four+1] += norm.y;
            mesh_.normals[index_four+2] += norm.z;

            triangle_index += 6;

            // Back side

            index = vertex_index(i,j,slices_,true);
            index_two = vertex_index(i+1,j,slices_,true);
            index_three = vertex_index(i,j+1,slices_,true);
            index_four = vertex_index(i+1,j+1,slices_,true);

            mesh_.indices[triangle_index] = index_three/3;
            mesh_.indices[triangle_index+1] = index/3;
            mesh_.indices[triangle_index+2] = index_four/3;

            mesh_.indices[triangle_index+3] = index/3;
            mesh_.indices[triangle_index+4] = index_two/3;
            mesh_.indices[triangle_index+5] = index_four/3;

            a = Vector3Subtract(Vector3{mesh_.vertices[index],mesh_.vertices[index+1],mesh_.vertices[index+2]},
                                        Vector3{mesh_.vertices[index_three],mesh_.vertices[index_three+1],mesh_.vertices[index_three+2]});          
            b = Vector3Subtract(Vector3{mesh_.vertices[index_four],mesh_.vertices[index_four+1],mesh_.vertices[index_four+2]},
                                        Vector3{mesh_.vertices[index_three],mesh_.vertices[index_three+1],mesh_.vertices[index_three+2]});
            c = Vector3Subtract(Vector3{mesh_.vertices[index_four],mesh_.vertices[index_four+1],mesh_.vertices[index_four+2]},
                                        Vector3{mesh_.vertices[index_two],mesh_.vertices[index_two+1],mesh_.vertices[index_two+2]});          
            d = Vector3Subtract(Vector3{mesh_.vertices[index],mesh_.vertices[index+1],mesh_.vertices[index+2]},
                                        Vector3{mesh_.vertices[index_two],mesh_.vertices[index_two+1],mesh_.vertices[index_two+2]});
            norm = Vector3Normalize(Vector3CrossProduct(a,b));
            if (norm.x == 0 && norm.y == 0 && norm.z == 0)
                norm = Vector3Normalize(Vector3CrossProduct(c,d));
            mesh_.normals[index] += norm.x;
            mesh_.normals[index+1] += norm.y;
            mesh_.normals[index+2] += norm.z;
            mesh_.normals[index_two] += norm.x;
            mesh_.normals[index_two+1] += norm.y;
            mesh_.normals[index_two+2] += norm.z;
            mesh_.normals[index_three] += norm.x;
            mesh_.normals[index_three+1] += norm.y;
            mesh_.normals[index_three+2] += norm.z;
            mesh_.normals[index_four] += norm.x;
            mesh_.normals[index_four+1] += norm.y;
            mesh_.normals[index_four+2] += norm.z;

            triangle_index += 6;
        }
    }

    for (int i = 0; i < mesh_.vertexCount; i++) {
        int normal_index = i*3;
        Vector3 norm = {mesh_.normals[normal_index], mesh_.normals[normal_index+1], mesh_.normals[normal_index+2]};
        norm = Vector3Normalize(norm);
        mesh_.normals[normal_index] = norm.x;
        mesh_.normals[normal_index+1] = norm.y;
        mesh_.normals[normal_index+2] = norm.z;
    }

    int freckle_index = petal_vertex_count*3;
    for (unsigned short index_vertex : freckle_positions) {
        constexpr float ROOT2_2 = 0.7071067811865475244f;
        constexpr float epsilon = 0.0001f; // Prevent Z-fighting
        
        Color freckle_color = ColorFromHSV(parameter_map_.get_parameter("FreckleHue").value,
                                        parameter_map_.get_parameter("FreckleSaturation").value,
                                        parameter_map_.get_parameter("FreckleValue").value);
        Vector3 normal = Vector3{mesh_.normals[index_vertex],mesh_.normals[index_vertex+1],mesh_.normals[index_vertex+2]};
        Vector3 position = Vector3Add(Vector3{mesh_.vertices[index_vertex],mesh_.vertices[index_vertex+1],mesh_.vertices[index_vertex+2]},
                                    normal*epsilon);
        Vector3 other = Vector3{0,0,1};
        Vector3 tangent = Vector3Normalize(Vector3CrossProduct(normal,other));
        assert((tangent.x != 0.0f || tangent.y != 0.0f || tangent.z != 0.0f));
        Vector3 binormal = Vector3Normalize(Vector3CrossProduct(normal,tangent));

        const float ELLIPSE_A = parameter_map_.get_parameter("FreckleSize").value*parameter_map_.get_parameter("Length").value/100.0f;
        const float ELLIPSE_B = parameter_map_.get_parameter("FreckleSize").value*parameter_map_.get_parameter("Width").value/100.0f;

        const float TANGENT_COMP_X = ELLIPSE_A*ROOT2_2*tangent.x;
        const float TANGENT_COMP_Y = ELLIPSE_A*ROOT2_2*tangent.y;
        const float TANGENT_COMP_Z = ELLIPSE_A*ROOT2_2*tangent.z;

        const float BINORMAL_COMP_X = ELLIPSE_B*ROOT2_2*binormal.x;
        const float BINORMAL_COMP_Y = ELLIPSE_B*ROOT2_2*binormal.y;
        const float BINORMAL_COMP_Z = ELLIPSE_B*ROOT2_2*binormal.z;

        mesh_.vertices[freckle_index] = position.x;
        mesh_.vertices[freckle_index+1] = position.y;
        mesh_.vertices[freckle_index+2] = position.z;
        
        mesh_.vertices[freckle_index+3] = position.x + ELLIPSE_A*tangent.x; // 0 Degrees
        mesh_.vertices[freckle_index+4] = position.y + ELLIPSE_A*tangent.y;
        mesh_.vertices[freckle_index+5] = position.z + ELLIPSE_A*tangent.z;

        mesh_.vertices[freckle_index+6] = position.x + TANGENT_COMP_X + BINORMAL_COMP_X; // 45 Degrees
        mesh_.vertices[freckle_index+7] = position.y + TANGENT_COMP_Y + BINORMAL_COMP_Y;
        mesh_.vertices[freckle_index+8] = position.z + TANGENT_COMP_Z + BINORMAL_COMP_Z;

        mesh_.vertices[freckle_index+9] = position.x + ELLIPSE_B*binormal.x; // 90 Degrees
        mesh_.vertices[freckle_index+10] = position.y + ELLIPSE_B*binormal.y;
        mesh_.vertices[freckle_index+11] = position.z + ELLIPSE_B*binormal.z;

        mesh_.vertices[freckle_index+12] = position.x - TANGENT_COMP_X + BINORMAL_COMP_X; // 135 Degrees
        mesh_.vertices[freckle_index+13] = position.y - TANGENT_COMP_Y + BINORMAL_COMP_Y;
        mesh_.vertices[freckle_index+14] = position.z - TANGENT_COMP_Z + BINORMAL_COMP_Z;

        mesh_.vertices[freckle_index+15] = position.x - ELLIPSE_A*tangent.x; // 180 Degrees
        mesh_.vertices[freckle_index+16] = position.y - ELLIPSE_A*tangent.y;
        mesh_.vertices[freckle_index+17] = position.z - ELLIPSE_A*tangent.z;

        mesh_.vertices[freckle_index+18] = position.x - TANGENT_COMP_X - BINORMAL_COMP_X; // 225 Degrees
        mesh_.vertices[freckle_index+19] = position.y - TANGENT_COMP_Y - BINORMAL_COMP_Y;
        mesh_.vertices[freckle_index+20] = position.z - TANGENT_COMP_Z - BINORMAL_COMP_Z;

        mesh_.vertices[freckle_index+21] = position.x - ELLIPSE_B*binormal.x; // 270 Degrees
        mesh_.vertices[freckle_index+22] = position.y - ELLIPSE_B*binormal.y;
        mesh_.vertices[freckle_index+23] = position.z - ELLIPSE_B*binormal.z;

        mesh_.vertices[freckle_index+24] = position.x + TANGENT_COMP_X - BINORMAL_COMP_X; // 315 Degrees
        mesh_.vertices[freckle_index+25] = position.y + TANGENT_COMP_Y - BINORMAL_COMP_Y;
        mesh_.vertices[freckle_index+26] = position.z + TANGENT_COMP_Z - BINORMAL_COMP_Z;

        int freckle_position = freckle_index/3;
        for (int i = freckle_position; i <= freckle_position + 8; i++) {
            mesh_.normals[i*3] = normal.x;
            mesh_.normals[i*3+1] = normal.y;
            mesh_.normals[i*3+2] = normal.z;
            mesh_.colors[i*4] = freckle_color.r;
            mesh_.colors[i*4+1] = freckle_color.g;
            mesh_.colors[i*4+2] = freckle_color.b;
            mesh_.colors[i*4+3] = 255;
        }

        mesh_.indices[triangle_index] = freckle_position; // Quadrant 1
        mesh_.indices[triangle_index+1] = freckle_position + 1;
        mesh_.indices[triangle_index+2] = freckle_position + 2;

        mesh_.indices[triangle_index+3] = freckle_position;
        mesh_.indices[triangle_index+4] = freckle_position + 2;
        mesh_.indices[triangle_index+5] = freckle_position + 3;

        mesh_.indices[triangle_index+6] = freckle_position; // Quadrant 2
        mesh_.indices[triangle_index+7] = freckle_position + 3;
        mesh_.indices[triangle_index+8] = freckle_position + 4;

        mesh_.indices[triangle_index+9] = freckle_position;
        mesh_.indices[triangle_index+10] = freckle_position + 4;
        mesh_.indices[triangle_index+11] = freckle_position + 5;

        mesh_.indices[triangle_index+12] = freckle_position; // Quadrant 3
        mesh_.indices[triangle_index+13] = freckle_position + 5;
        mesh_.indices[triangle_index+14] = freckle_position + 6;

        mesh_.indices[triangle_index+15] = freckle_position;
        mesh_.indices[triangle_index+16] = freckle_position + 6;
        mesh_.indices[triangle_index+17] = freckle_position + 7;

        mesh_.indices[triangle_index+18] = freckle_position; // Quadrant 4
        mesh_.indices[triangle_index+19] = freckle_position + 7;
        mesh_.indices[triangle_index+20] = freckle_position + 8;

        mesh_.indices[triangle_index+21] = freckle_position;
        mesh_.indices[triangle_index+22] = freckle_position + 8;
        mesh_.indices[triangle_index+23] = freckle_position + 1;

        freckle_index += 27;
        triangle_index += 24;
    }

    UploadMesh(&mesh_,false);
    update_matrix();
}

void TaperedPetal::set_slices(std::pair<int,int> slices) {
    slices_ = slices;
}

void TaperedPetal::initialize_parameters() {
    std::mt19937_64 rng(seed_);
    parameter_map_.set_parameter("Sharpness", Parameter{0.5f,0.75f,1.0f});
    parameter_map_.seed_gaussian("Sharpness",rng);

    parameter_map_.set_parameter("Length", Parameter{0.25f,0.5f,1.0f});
    parameter_map_.seed_gaussian("Length",rng);

    parameter_map_.set_parameter("Height", Parameter{0.1f,0.25f,0.5f});
    parameter_map_.seed_gaussian("Height",rng);

    parameter_map_.set_parameter("Curl", Parameter{2.0f,2.25f,3.0f});
    parameter_map_.seed_gaussian("Curl",rng);

    parameter_map_.set_parameter("Width", Parameter{0.1f,0.125f,0.25f});
    parameter_map_.seed_gaussian("Width",rng);

    parameter_map_.set_parameter("Curvature", Parameter{0.1f,0.175f,0.35f});
    parameter_map_.seed_gaussian("Curvature",rng);

    parameter_map_.set_parameter("BaseHue", Parameter{250.0f,340.0f,440.0f});
    parameter_map_.seed_gaussian("BaseHue",rng);
    parameter_map_.set_parameter("BaseSaturation", Parameter{0.0f,0.5f,1.0f});
    parameter_map_.seed_log_normal("BaseSaturation",rng);
    parameter_map_.set_parameter("BaseValue", Parameter{0.2f,0.7f,1.0f});
    parameter_map_.seed_log_normal_inverse("BaseValue",rng);

    parameter_map_.set_parameter("BorderWidth", Parameter{0.0f,0.5f,3.0f});
    parameter_map_.seed_gaussian("BorderWidth",rng);

    parameter_map_.set_parameter("BorderHue", Parameter{250.0f,340.0f,440.0f});
    parameter_map_.seed_gaussian("BorderHue",rng);
    parameter_map_.set_parameter("BorderSaturation", Parameter{0.0f,0.5f,1.0f});
    parameter_map_.seed_log_normal("BorderSaturation",rng);
    parameter_map_.set_parameter("BorderValue", Parameter{0.2f,0.7f,1.0f});
    parameter_map_.seed_log_normal_inverse("BorderValue",rng);

    parameter_map_.set_parameter("GradientWidth", Parameter{0.0f,1.5f,3.0f});
    parameter_map_.seed_gaussian("GradientWidth",rng);

    parameter_map_.set_parameter("GradientHue", Parameter{250.0f,340.0f,440.0f});
    parameter_map_.seed_gaussian("GradientHue",rng);
    parameter_map_.set_parameter("GradientSaturation", Parameter{0.0f,0.5f,1.0f});
    parameter_map_.seed_log_normal("GradientSaturation",rng);
    parameter_map_.set_parameter("GradientValue", Parameter{0.2f,0.7f,1.0f});
    parameter_map_.seed_log_normal_inverse("GradientValue",rng);

    parameter_map_.set_parameter("StripeWidth", Parameter{0.0f,0.125f,0.25f});
    parameter_map_.seed_gaussian("StripeWidth",rng);

    parameter_map_.set_parameter("StripeHue", Parameter{250.0f,340.0f,440.0f});
    parameter_map_.seed_gaussian("StripeHue",rng);
    parameter_map_.set_parameter("StripeSaturation", Parameter{0.0f,0.5f,1.0f});
    parameter_map_.seed_log_normal("StripeSaturation",rng);
    parameter_map_.set_parameter("StripeValue", Parameter{0.2f,0.7f,1.0f});
    parameter_map_.seed_log_normal_inverse("StripeValue",rng);

    parameter_map_.set_parameter("FreckleAmount", Parameter{0.0f,0.45f,0.9f});
    parameter_map_.seed_gaussian("FreckleAmount",rng);

    parameter_map_.set_parameter("FreckleCentrality", Parameter{1.0f,2.0f,4.0f});
    parameter_map_.seed_gaussian("FreckleCentrality",rng);

    parameter_map_.set_parameter("FreckleSize", Parameter{0.1f,1.5f,3.0f});
    parameter_map_.seed_gaussian("FreckleSize",rng);

    parameter_map_.set_parameter("FreckleCoverage", Parameter{0.0f,0.6f,0.9f});
    parameter_map_.seed_gaussian("FreckleCoverage",rng);

    parameter_map_.set_parameter("FreckleHue", Parameter{250.0f,340.0f,440.0f});
    parameter_map_.seed_gaussian("FreckleHue",rng);
    parameter_map_.set_parameter("FreckleSaturation", Parameter{0.0f,0.5f,1.0f});
    parameter_map_.seed_log_normal("FreckleSaturation",rng);
    parameter_map_.set_parameter("FreckleValue", Parameter{0.0f,0.2f,0.3f});
    parameter_map_.seed_log_normal("FreckleValue",rng);
    
    parameter_map_.set_parameter("CreaseBoolean", Parameter{0.0f,0.0f,1.0f});
    
    parameter_map_.set_parameter("ConcaveBoolean", Parameter{0.0f,0.0f,1.0f});
}

Vector3 TaperedPetal::tip_vector() const {
    float curl = parameter_map_.get_parameter("Curl").value;
    float height = parameter_map_.get_parameter("Height").value;
    float length = parameter_map_.get_parameter("Length").value;

    float mid_x = length*curl/3.0f;
    float a = std::sqrtf(height)/mid_x;

    return Vector3{length,2.0f*curl*a*a*length*length/3.0f-a*a*length*length};
}

float TaperedPetal::base_width() const {
    int index_one = vertex_index(1,0,slices_,false);
    int index_two = vertex_index(1,slices_.second-1,slices_,false);
    return mesh_.vertices[index_two+2] - mesh_.vertices[index_one+2];
}

float TaperedPetal::X(float u, float v) const {
    return u;
}

float TaperedPetal::Y(float u, float v) const {
    bool crease = parameter_map_.get_parameter("CreaseBoolean").value > 0.5f;
    bool concave = parameter_map_.get_parameter("ConcaveBoolean").value > 0.5f;
    float length = parameter_map_.get_parameter("Length").value;
    float height = parameter_map_.get_parameter("Height").value;
    float width = parameter_map_.get_parameter("Width").value;
    float curvature = parameter_map_.get_parameter("Curvature").value;
    if (crease)
        curvature = std::powf(curvature,2.0f);
    float curl = parameter_map_.get_parameter("Curl").value;
    float z = Z(u,v);
    float midpoint = length*curl/3.0f;
    float a = std::sqrt(height)/midpoint;
    float t1 = std::powf((a*(u-midpoint)),2);
    float t2 = std::abs(std::powf(curvature*z/width,(float)(2-crease)));
    float t3 = std::powf(midpoint*a,2);
    return -t1 + (1.0f-2.0f*concave)*t2 + t3;
}

float TaperedPetal::Z(float u, float v) const {
    float length = parameter_map_.get_parameter("Length").value;
    float sharpness = parameter_map_.get_parameter("Sharpness").value;
    float temp = std::abs(-4.0f*(u-length/2.0f)*(u-length/2.0f)/(length*length)+1);
    return v * std::powf(temp,1/(4.0f-3.0f*sharpness));
}

TaperedLeaf::TaperedLeaf() : TaperedLeaf(std::random_device{}()) {}
TaperedLeaf::TaperedLeaf(uint32_t seed) : TaperedPetal(seed) {
    initialize_parameters();
}
TaperedLeaf::TaperedLeaf(Quaternion quaternion, Vector3 position, float scale) : TaperedLeaf(quaternion, position, scale, std::random_device{}()) {}
TaperedLeaf::TaperedLeaf(Quaternion quaternion, Vector3 position, float scale, uint32_t seed) : TaperedPetal(quaternion,position,scale,seed) {
    initialize_parameters();
}
TaperedLeaf::TaperedLeaf(const json& j) {
    from_json(j);
    initialize_parameters();
}

json TaperedLeaf::to_json() const {
    json j = {
        {"type","TaperedLeaf"},
        {"object_type",object_type_},
        {"position",{{"x",position_.x},{"y",position_.y},{"z",position_.z}}},
        {"scale",scale_},
        {"quaternion",{{"x",quaternion_.x},{"y",quaternion_.y},{"z",quaternion_.z},{"w",quaternion_.w}}},
        {"seed",seed_},
        {"parameter_map",parameter_map_.to_json()},
        {"slices",{{"first",slices_.first},{"second",slices_.second}}}
    };
    return j;
}

void TaperedLeaf::from_json(const json& j) {
    object_type_ = j.at("object_type");
    position_ = {j.at("position")["x"],j.at("position")["y"],j.at("position")["z"]};
    scale_ = j.at("scale");
    quaternion_ = {j.at("quaternion")["x"],j.at("quaternion")["y"],j.at("quaternion")["z"],j.at("quaternion")["w"]};
    seed_ = j.at("seed");
    parameter_map_ = ParameterMap{j.at("parameter_map")};
    slices_ = {j.at("slices")["first"],j.at("slices")["second"]};
}

void TaperedLeaf::initialize_parameters() {
    std::mt19937_64 rng(seed_);
    parameter_map_.set_parameter("Sharpness", Parameter{0.8f,0.9f,1.0f});
    parameter_map_.seed_gaussian("Sharpness",rng);

    parameter_map_.set_parameter("Length", Parameter{0.5f,0.75f,1.0f});
    parameter_map_.seed_gaussian("Length",rng);

    parameter_map_.set_parameter("Height", Parameter{0.05f,0.25f,0.4f});
    parameter_map_.seed_gaussian("Height",rng);

    parameter_map_.set_parameter("Curl", Parameter{2.5f,2.75f,3.0f});
    parameter_map_.seed_gaussian("Curl",rng);

    parameter_map_.set_parameter("Width", Parameter{0.1f,0.15f,0.2f});
    parameter_map_.seed_gaussian("Width",rng);

    parameter_map_.set_parameter("Curvature", Parameter{0.1f,0.15f,0.3f});
    parameter_map_.seed_gaussian("Curvature",rng);

    parameter_map_.set_parameter("BaseHue", Parameter{90.0f,110.0f,135.0f});
    parameter_map_.seed_gaussian("BaseHue",rng);
    parameter_map_.set_parameter("BaseSaturation", Parameter{0.8f,0.9f,1.0f});
    parameter_map_.seed_log_normal("BaseSaturation",rng);
    parameter_map_.set_parameter("BaseValue", Parameter{0.5f,0.7f,0.9f});
    parameter_map_.seed_log_normal("BaseValue",rng);

    parameter_map_.set_parameter("BorderWidth", Parameter{0.0f,0.0f,3.0f});

    parameter_map_.set_parameter("BorderHue", parameter_map_.get_parameter("BaseHue"));
    parameter_map_.set_parameter("BorderSaturation", parameter_map_.get_parameter("BaseSaturation"));
    parameter_map_.set_parameter("BorderValue", parameter_map_.get_parameter("BaseValue"));

    parameter_map_.set_parameter("GradientWidth", Parameter{0.0f,0.0f,3.0f});

    parameter_map_.set_parameter("GradientHue", parameter_map_.get_parameter("BaseHue"));
    parameter_map_.set_parameter("GradientSaturation", parameter_map_.get_parameter("BaseSaturation"));
    parameter_map_.set_parameter("GradientValue", parameter_map_.get_parameter("BaseValue"));

    parameter_map_.set_parameter("StripeWidth", Parameter{0.1f,0.175f,0.25f});
    parameter_map_.seed_gaussian("StripeWidth",rng);

    parameter_map_.set_parameter("StripeHue", parameter_map_.get_parameter("BaseHue"));
    parameter_map_.set_parameter("StripeSaturation", parameter_map_.get_parameter("BaseSaturation"));
    parameter_map_.set_parameter("StripeValue", Parameter{0.5f,parameter_map_.get_parameter("BaseValue").value-0.3f,0.9f});

    parameter_map_.set_parameter("FreckleAmount", Parameter{0.0f,0.0f,0.9f});

    parameter_map_.set_parameter("FreckleCentrality", Parameter{1.0f,2.0f,4.0f});

    parameter_map_.set_parameter("FreckleSize", Parameter{0.1f,1.5f,3.0f});

    parameter_map_.set_parameter("FreckleCoverage", Parameter{0.0f,0.0f,0.9f});

    parameter_map_.set_parameter("FreckleHue", Parameter{250.0f,340.0f,440.0f});
    parameter_map_.set_parameter("FreckleSaturation", Parameter{0.0f,0.5f,1.0f});
    parameter_map_.set_parameter("FreckleValue", Parameter{0.0f,0.2f,0.3f});
    
    parameter_map_.set_parameter("CreaseBoolean", Parameter{0.0f,1.0f,1.0f});
    
    parameter_map_.set_parameter("ConcaveBoolean", Parameter{0.0f,0.0f,1.0f});
}