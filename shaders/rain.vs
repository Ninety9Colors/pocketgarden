#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

in mat4 instanceTransform;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec4 fragColor;

uniform float subseconds;
uniform uint seconds;
uniform float rainSpeed;
uniform vec3 rainDirection;

uint hash_uint(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

void main() {
    // TODO: raycast this height
    float height = instanceTransform[3][1];
    vec3 yVector = vec3(0.,height,0.);
    float cosTheta = dot(normalize(rainDirection),-normalize(yVector));
    float distanceToGround = height/cosTheta+1.;

    // RNG noise for rain
    float rngOffset = float((hash_uint(uint(gl_InstanceID))%uint(10000000)))/float(10000000);

    float rainDistance = mod(rngOffset+mod((float(seconds)+subseconds),distanceToGround/rainSpeed)/(distanceToGround/rainSpeed),1.0)*distanceToGround;

    vec4 newPosition = vec4(vertexPosition + normalize(rainDirection)*rainDistance,1.);

    fragPosition = vec3(instanceTransform*newPosition);
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal,0)));
    fragColor = vertexColor;

    gl_Position = mvp*instanceTransform*newPosition;
}