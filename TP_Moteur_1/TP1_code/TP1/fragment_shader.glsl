#version 330 core

// Input data
in vec2 UV;

// Ouput data
out vec3 color;

uniform sampler2D textureSampler;

void main(){

        color = texture(textureSampler, UV);

}
