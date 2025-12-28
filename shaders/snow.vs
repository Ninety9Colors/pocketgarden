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
uniform float snowSpeed;
uniform vec3 snowDirection;
uniform float snowSway;
uniform vec3 playerPosition;

uint pcg(uint v) {
	uint state = v * uint(747796405) + uint(2891336453);
	uint word = ((state >> ((state >> uint(28)) + uint(4))) ^ state) * uint(277803737);
	return (word >> uint(22)) ^ word;
}

uint pcg2(uvec2 v)
{
    uint h = v.x * 747796405u + v.y * 2891336453u + 277803737u;
    h = ((h >> ((h >> 28u) + 4u)) ^ h) * 277803737u;
    return (h >> 22u) ^ h;
}

float hash(uint seed) {
    return float(pcg(seed)) * (1.0 / 4294967296.0);
}

float hash21(uvec2 v)
{
    return float(pcg2(v)) * (1.0 / 4294967296.0);
}

void main() {
    float time = (float(seconds)+subseconds);

    mat4 currTransform = instanceTransform;
    float x_offset = floor(playerPosition.x)+0.1;
    float z_offset = floor(playerPosition.z)+0.1;
    currTransform[3][0] += x_offset;
    currTransform[3][2] += z_offset;

    // RNG noise
    uint xSeed = uint(abs(100.*currTransform[3][0]+10000.));
    uint zSeed = uint(100.*abs(currTransform[3][2]+10000.));
    uvec2 seed = uvec2(xSeed,zSeed);
    float rngOffset = fract(hash21(seed));

    float xSway = snowSway*sin(time+30.*(hash(xSeed)));
    float zSway = snowSway*cos(time+30.*(hash(zSeed)));

    float distanceToGround = instanceTransform[3][1]/abs(normalize(snowDirection).y);
    float distancePhase = mod(time,distanceToGround/snowSpeed)/(distanceToGround/snowSpeed);

    distancePhase = (distancePhase*10. + sin(distancePhase*10.+time*rngOffset)/10.)/(10.+sin(10.));

    // Distance along the track in meters
    float snowDistance = mod(rngOffset+distancePhase,1.0)*distanceToGround;

    vec3 snowOffset = normalize(snowDirection)*snowDistance;
    currTransform[3][0] += snowOffset.x + rngOffset + xSway;
    currTransform[3][1] += snowOffset.y;
    currTransform[3][2] += snowOffset.z + rngOffset + zSway;
    // vec4 newPosition = vec4(vertexPosition + normalize(snowDirection)*snowDistance,1.);

    fragPosition = vec3(currTransform*vec4(vertexPosition,1.));
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal,0)));
    fragColor = vertexColor;

    gl_Position = mvp*currTransform*vec4(vertexPosition,1.);
}