#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colorDiffuse;

out vec4 finalColor;

void main() {
    finalColor = colorDiffuse;
    // finalColor = vec4(vec3(gl_FragCoord.w*0.5),1.);
}