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

    vFogAmount = clamp((-100.0 - cameraPos.z) / (-100.0 - (-50.0)), 0.0, 1.0);
}`;


let basic_3d_fragment_source = `
precision highp float;

varying vec2 vTexCoords;
varying float vFogAmount;

uniform sampler2D texture;

void main() {
    vec2 sampleCoord;
    sampleCoord.x = vTexCoords.x;
    sampleCoord.y = vTexCoords.y;
    vec3 textureColor = texture2D(texture, sampleCoord).rgb;

    //vec3 fogColor = vec3(0.0, 0.7, 0.8);
    //vec3 foggedColor = vFogAmount * textureColor + (1.0 - vFogAmount) * fogColor;
    //gl_FragColor = vec4(foggedColor, 1.0);
    gl_FragColor = vec4(textureColor, 1.0);
}`;
