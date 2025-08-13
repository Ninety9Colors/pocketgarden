#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>
#include <iostream>

#include "raylib.h"
#include "raymath.h"

#include "tapered_petal.hpp"
#include "util.hpp"

// Returns the true float index offset, not the triplet offset
static int vertex_index(int i, int j, std::pair<int,int> slices, bool bottom) {
    return ((slices.second+1)*i + j)*3 + ((slices.first+1)*(slices.second+1)*3*bottom);
}

static float lerp(float a, float b, float amount) {
    return (1-amount)*a + amount*b;
}

TaperedPetal::TaperedPetal() : ParameterObject(), slices_{40,20} {initialize_parameters();generate_mesh();}
TaperedPetal::TaperedPetal(float scale) : ParameterObject(scale), slices_{40,20} {initialize_parameters();generate_mesh();}
TaperedPetal::TaperedPetal(Vector3 position, float scale) : ParameterObject(position, scale), slices_{40,20} {initialize_parameters();generate_mesh();}
TaperedPetal::TaperedPetal(std::string data) : slices_{40,20} {
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "TaperedPetal");
    position_ = Vector3{std::stof(split[1]), std::stof(split[2]), std::stof(split[3])};
    scale_ = std::stof(split[4]);
    quaternion_ = Quaternion{std::stof(split[5]),std::stof(split[6]),std::stof(split[7]),std::stof(split[8])};
    parameter_map_ = ParameterMap(split[9]);
    generate_mesh();
}

void TaperedPetal::generate_mesh() {
    UnloadMesh(mesh_);

    float length = parameter_map_.get_parameter("Length").value;
    float width = parameter_map_.get_parameter("Width").value;

    float u_step = length/(1.0f*slices_.first);
    float v_step = 2.0f*width/(1.0f*slices_.second);

    constexpr int seed = 1232141241;
    SetRandomSeed(seed);

    // Generate Freckle Positions
    std::vector<unsigned short> freckle_positions {};
    for (int i = 0; i <= slices_.first; i++) {
        for (int j = 0; j <= slices_.second; j++) {
            float u = i*u_step;
            int index_top = vertex_index(i,j,slices_,false);
            float roll = 1.0f*GetRandomValue(0,10000);
            float freckle_coverage = parameter_map_.get_parameter("FreckleCoverage").value;
            float freckle_chance = parameter_map_.get_parameter("FreckleAmount").value*std::powf(1.0f-u/(length*freckle_coverage),parameter_map_.get_parameter("FreckleCentrality").value);
            if (i > 1 && j > 1 && j < slices_.second-1 && u/length < freckle_coverage && roll < freckle_chance*10000.0f)
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

            float base_r = parameter_map_.get_parameter("BaseColor").min*255;
            float base_g = parameter_map_.get_parameter("BaseColor").value*255;
            float base_b = parameter_map_.get_parameter("BaseColor").max*255;

            float gradient_r = parameter_map_.get_parameter("GradientColor").min*255;
            float gradient_g = parameter_map_.get_parameter("GradientColor").value*255;
            float gradient_b = parameter_map_.get_parameter("GradientColor").max*255;

            float border_r = parameter_map_.get_parameter("BorderColor").min*255;
            float border_g = parameter_map_.get_parameter("BorderColor").value*255;
            float border_b = parameter_map_.get_parameter("BorderColor").max*255;

            float stripe_r = parameter_map_.get_parameter("StripeColor").min*255;
            float stripe_g = parameter_map_.get_parameter("StripeColor").value*255;
            float stripe_b = parameter_map_.get_parameter("StripeColor").max*255;

            float gradient_width = parameter_map_.get_parameter("GradientWidth").value;
            float gradient_amount = std::max<float>(gradient_width-(length-u)/length,0.0f)/gradient_width;
            if (gradient_width == 0.0f) gradient_amount = 0.0f;

            float border_width = parameter_map_.get_parameter("BorderWidth").value;
            float border_amount = std::max<float>(border_width-(width-std::abs(v))/width,0.0f)/border_width;
            if (border_width == 0.0f) border_amount = 0.0f;

            float stripe_width = parameter_map_.get_parameter("StripeWidth").value;
            float stripe_amount = std::max<float>(stripe_width-std::abs(v)/width,0.0f)/stripe_width;
            if (stripe_width == 0.0f) stripe_amount = 0.0f;

            float r = lerp(base_r,gradient_r,gradient_amount);
            float g = lerp(base_g,gradient_g,gradient_amount);
            float b = lerp(base_b,gradient_b,gradient_amount);

            r = lerp(r, border_r, border_amount);
            g = lerp(g, border_g, border_amount);
            b = lerp(b, border_b, border_amount);

            r = lerp(r, stripe_r, stripe_amount);
            g = lerp(g, stripe_g, stripe_amount);
            b = lerp(b, stripe_b, stripe_amount);

            mesh_.colors[4*index_top/3] = (unsigned short) r;
            mesh_.colors[4*index_top/3+1] = (unsigned short) g;
            mesh_.colors[4*index_top/3+2] = (unsigned short) b;
            mesh_.colors[4*index_top/3+3] = 255;

            mesh_.colors[4*index_bottom/3] = (unsigned short) base_r;
            mesh_.colors[4*index_bottom/3+1] = (unsigned short) base_g;
            mesh_.colors[4*index_bottom/3+2] = (unsigned short) base_b;
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
        int index = i*3;
        Vector3 vertex = {mesh_.vertices[index], mesh_.vertices[index+1], mesh_.vertices[index+2]};
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
        unsigned short r = (unsigned short) (parameter_map_.get_parameter("FreckleColor").min*255);
        unsigned short g = (unsigned short) (parameter_map_.get_parameter("FreckleColor").value*255);
        unsigned short b = (unsigned short) (parameter_map_.get_parameter("FreckleColor").max*255);
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
            mesh_.colors[i*4] = r;
            mesh_.colors[i*4+1] = g;
            mesh_.colors[i*4+2] = b;
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

std::string TaperedPetal::to_string() const {
    return "TaperedPetal " +
    std::to_string(position_.x) + " " + std::to_string(position_.y) + " " + std::to_string(position_.z) + " " +
    std::to_string(scale_) + " " +
    std::to_string(quaternion_.x) + " " + std::to_string(quaternion_.y) + " " + std::to_string(quaternion_.z) + " " + std::to_string(quaternion_.w) + " (" + 
    parameter_map_.to_string() + ")";
}

void TaperedPetal::set_slices(std::pair<int,int> slices) {
    slices_ = slices;
}

void TaperedPetal::initialize_parameters() {
    parameter_map_.set_parameter("Sharpness", Parameter{0.5f,0.75f,1.0f});
    parameter_map_.set_parameter("Length", Parameter{0.1f,0.5f,1.0f});
    parameter_map_.set_parameter("Height", Parameter{0.1f,0.25f,0.5f});
    parameter_map_.set_parameter("Curl", Parameter{1.5f,2.25f,3.0f});
    parameter_map_.set_parameter("Width", Parameter{0.1f,0.125f,0.25f});
    parameter_map_.set_parameter("Curvature", Parameter{0.1f,0.175f,0.35f});

    parameter_map_.set_parameter("BaseColor", Parameter{0.89411764705f,0.88235294117f,0.9294117647f});
    parameter_map_.set_parameter("BorderWidth", Parameter{0.0f,0.5f,3.0f});
    parameter_map_.set_parameter("BorderColor", Parameter{0.47058823529f,0.13725490196f,0.31372549019f});
    parameter_map_.set_parameter("GradientWidth", Parameter{0.0f,1.5f,3.0f});
    parameter_map_.set_parameter("GradientColor", Parameter{0.89411764705f,0.88235294117f,0.9294117647f});
    parameter_map_.set_parameter("StripeWidth", Parameter{0.0f,0.125f,0.25f});
    parameter_map_.set_parameter("StripeColor", Parameter{1.0f,0.0f,0.0f});
    parameter_map_.set_parameter("FreckleAmount", Parameter{0.0f,0.45f,0.9f});
    parameter_map_.set_parameter("FreckleCentrality", Parameter{1.0f,2.0f,4.0f});
    parameter_map_.set_parameter("FreckleSize", Parameter{0.1f,1.5f,3.0f});
    parameter_map_.set_parameter("FreckleCoverage", Parameter{0.0f,0.6f,0.9f});
    parameter_map_.set_parameter("FreckleColor", Parameter{0.0f,0.0f,0.0f});
}

float TaperedPetal::X(float u, float v) const {
    return u;
}

float TaperedPetal::Y(float u, float v) const {
    float length = parameter_map_.get_parameter("Length").value;
    float height = parameter_map_.get_parameter("Height").value;
    float width = parameter_map_.get_parameter("Width").value;
    float curvature = parameter_map_.get_parameter("Curvature").value;
    float curl = parameter_map_.get_parameter("Curl").value;
    float z = Z(u,v);
    float midpoint = length*curl/3.0f;
    float a = std::sqrt(height)/midpoint;
    float t1 = std::powf((a*(u-midpoint)),2);
    float t2 = std::powf(curvature*z/width,2);
    float t3 = std::powf(midpoint*a,2);
    return -t1 + t2 + t3;
}

float TaperedPetal::Z(float u, float v) const {
    float length = parameter_map_.get_parameter("Length").value;
    float sharpness = parameter_map_.get_parameter("Sharpness").value;
    float temp = std::abs(-4.0f*(u-length/2.0f)*(u-length/2.0f)/(length*length)+1);
    return v * std::powf(temp,1/(4.0f-3.0f*sharpness));
}