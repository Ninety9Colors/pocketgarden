#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;

uniform sampler2D depthTexture;
uniform sampler2D colorTexture;
uniform float fogFarPlane;
uniform vec4 fogColor;

out vec4 finalColor;

const float nearPlane = 0.1;

void main() {
    float depth = texture(depthTexture,fragTexCoord).r;
    float linearDepth = (2.0*nearPlane)/(fogFarPlane + nearPlane - depth*(fogFarPlane - nearPlane));
    vec4 ogColor = texture(colorTexture,fragTexCoord);
    vec4 newColor = mix(ogColor,fogColor,linearDepth);

    // finalColor = vec4(vec3(linearDepth),1.);
    finalColor = newColor;
}