let basic_3d_vertex_source = `
attribute vec3 position;
attribute vec2 texCoords;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

varying vec2 vTexCoords;
varying float vFogAmount;

void main() {
    vec4 cameraPos = viewMatrix * modelMatrix * vec4(position, 1.0); 
    gl_Position = projMatrix * cameraPos;
    vTexCoords = texCoords;

    vFogAmount = cameraPos.z;
}`;


let basic_3d_fragment_source = `
#extension GL_OES_standard_derivatives : enable

precision highp float;

varying vec2 vTexCoords;
varying float vFogAmount;

uniform sampler2D texture;

void main() {
    vec2 texSize = vec2(4096.0, 4096.0);
    vec2 uv = vTexCoords;
    uv *= texSize;
    vec2 seam = floor(uv + 0.5);
    vec2 dudv = fwidth(uv);
    uv = seam + clamp((uv - seam) / dudv, -0.5, 0.5);
    uv /= texSize;

    vec3 textureColor = texture2D(texture, uv).rgb;

    vec3 fogColor = vec3(0.144, 0.41, 0.90);
    float fogFactor = clamp(1.0 - vFogAmount / -900.0, 0.0, 1.0);
    vec3 foggedColor = fogFactor * textureColor + (1.0 - fogFactor) * fogColor;
    gl_FragColor = vec4(foggedColor, 1.0);
}`;
