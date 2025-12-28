#version 330

in vec3 vertexPosition;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;

out vec3 fragPosition;
out vec2 fragTexCoord;

void main() {
    fragTexCoord = vec2(vertexPosition.x*0.5 + 0.5,vertexPosition.y*0.5 + 0.5);
    fragPosition = vertexPosition;

    gl_Position = vec4(vertexPosition,1.0);
}