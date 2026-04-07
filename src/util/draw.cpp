#include "util/draw.hpp"

#include "raymath.h"
#include "application.hpp"

// TODO: optimize
void draw_mesh_skeleton(Mesh mesh, Matrix transform) {
    int n = mesh.vertexCount;
    int m = mesh.triangleCount;
    for (int i = 0; i < n; i++) {
        float x = mesh.vertices[i*3];
        float y = mesh.vertices[i*3+1];
        float z = mesh.vertices[i*3+2];

        float xn = mesh.normals[i*3];
        float yn = mesh.normals[i*3+1];
        float zn = mesh.normals[i*3+2];

        DrawSphere(Vector3Transform(Vector3{x,y,z},transform),0.005f,RED);
        DrawLine3D(Vector3Transform(Vector3{x,y,z},transform),Vector3Transform(Vector3Add(Vector3{x,y,z},Vector3Scale(Vector3Normalize(Vector3{xn,yn,zn}),0.01f)),transform),GREEN);
    }
    for (int i = 0; i < m; i++) {
        int a = mesh.indices[i*3];
        int b = mesh.indices[i*3+1];
        int c = mesh.indices[i*3+2];

        float x1 = mesh.vertices[a*3];
        float y1 = mesh.vertices[a*3+1];
        float z1 = mesh.vertices[a*3+2];

        float x2 = mesh.vertices[b*3];
        float y2 = mesh.vertices[b*3+1];
        float z2 = mesh.vertices[b*3+2];

        float x3 = mesh.vertices[c*3];
        float y3 = mesh.vertices[c*3+1];
        float z3 = mesh.vertices[c*3+2];

        DrawLine3D(Vector3Transform(Vector3{x1,y1,z1},transform),Vector3Transform(Vector3{x2,y2,z2},transform),BLUE);
        DrawLine3D(Vector3Transform(Vector3{x2,y2,z2},transform),Vector3Transform(Vector3{x3,y3,z3},transform),BLUE);
        DrawLine3D(Vector3Transform(Vector3{x3,y3,z3},transform),Vector3Transform(Vector3{x1,y1,z1},transform),BLUE);
    }
}

void draw_binary_voxels(const std::vector<std::vector<std::vector<double>>>& coordinates, float size, Mesh mesh) {
    long long n = coordinates.size();
    long long m = coordinates[0].size();
    long long q = coordinates[0][0].size();
    long long total = n*m*q;
    std::vector<Matrix> transforms {};
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < q; k++) {
                bool inside = coordinates[i][j][k] < 0.0; // -1.0 means inside object, 1.0 means outside
                if (inside)
                    transforms.push_back(MatrixMultiply(MatrixScale(size,size,size),MatrixMultiply(MatrixRotateX(0.0f),MatrixTranslate(i*size,j*size,k*size))));
            }
        }
    }
    Material mat = LoadMaterialDefault();
    mat.shader = Application::get_shader_instanced();
    mat.maps[MATERIAL_MAP_DIFFUSE].color = Color(255,0,0,255);
    DrawMeshInstanced(mesh,mat,transforms.data(),transforms.size());
}