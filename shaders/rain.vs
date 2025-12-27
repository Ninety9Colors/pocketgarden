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
uniform int seconds;
uniform float rainSpeed;
uniform vec3 rainDirection;
uniform vec3 playerPosition;

uint pcg2(uvec2 v)
{
    uint h = v.x * 747796405u + v.y * 2891336453u + 277803737u;
    h = ((h >> ((h >> 28u) + 4u)) ^ h) * 277803737u;
    return (h >> 22u) ^ h;
}

float hash21(uvec2 v)
{
    return float(pcg2(v)) * (1.0 / 4294967296.0);
}

void main() {
    mat4 currTransform = instanceTransform;
    float x_offset = floor(playerPosition.x)+0.1;
    float z_offset = floor(playerPosition.z)+0.1;
    currTransform[3][0] += x_offset;
    currTransform[3][2] += z_offset;

    float distanceToGround = instanceTransform[3][1]/abs(normalize(rainDirection).y);

    // RNG noise for rain
    uvec2 seed = uvec2(uint(abs(100.*currTransform[3][0]+10000.)),uint(100.*abs(currTransform[3][2]+10000.)));
    float rngOffset = fract(hash21(seed));

    float rainDistance = mod(rngOffset+mod((float(seconds)+subseconds),distanceToGround/rainSpeed)/(distanceToGround/rainSpeed),1.0)*distanceToGround;

    vec3 rainOffset = normalize(rainDirection)*rainDistance;
    currTransform[3][0] += rainOffset.x + rngOffset;
    currTransform[3][1] += rainOffset.y;
    currTransform[3][2] += rainOffset.z + rngOffset;
    // vec4 newPosition = vec4(vertexPosition + normalize(rainDirection)*rainDistance,1.);

    fragPosition = vec3(currTransform*vec4(vertexPosition,1.));
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal,0)));
    fragColor = vertexColor;

    gl_Position = mvp*currTransform*vec4(vertexPosition,1.);
}