#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colorDiffuse;

uniform vec3 viewPos;
uniform vec3 sunPos;
uniform vec4 sunColor;
uniform vec4 ambient;

out vec4 finalColor;

void main() {
    float diff = max(dot(normalize(sunPos-fragPosition),fragNormal),0.0);
    vec4 diffuse = vec4(diff*sunColor.xyz,sunColor.w);

    finalColor = (ambient+diffuse)*colorDiffuse*fragColor;
}