#version 330 core

// Input data
in vec2 UV;

// Ouput data
out vec4 color;
uniform sampler2D texture_diffuse1;

void main(){

        vec4 water = texture(texture_diffuse1, UV);

        color = vec4(water.x, water.y, water.z, 0.5);


}
