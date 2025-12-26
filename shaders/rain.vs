#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

uniform float subseconds;
uniform int seconds;
uniform float rainSpeed;
uniform vec3 rainDirection;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec4 fragColor;

void main() {
    // TODO: raycast this height
    float height = matModel[3][1];
    vec3 yVector = vec3(0.,height,0.);
    float cosTheta = dot(normalize(rainDirection),-normalize(yVector));
    float distanceToGround = height/cosTheta;
    float rainDistance = mod((float(seconds)+subseconds),distanceToGround/rainSpeed)*rainSpeed;

    vec4 newPosition = vec4(vertexPosition + normalize(rainDirection)*rainDistance,1.);

    fragPosition = vec3(matModel*newPosition);
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal,0)));
    fragColor = vertexColor;

    gl_Position = mvp*newPosition;
}