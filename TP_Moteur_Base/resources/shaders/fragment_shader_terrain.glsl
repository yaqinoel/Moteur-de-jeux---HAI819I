#version 330 core

// Input data
in vec2 UV;
in float Height;

// Ouput data
out vec4 color;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_diffuse4;

void main(){

        vec4 water = texture(texture_diffuse1,UV);
        vec4 grass = texture(texture_diffuse2, UV);
        vec4 rock  = texture(texture_diffuse3, UV);
        vec4 snow  = texture(texture_diffuse4, UV);

        if( Height < 0){
                float t = smoothstep(-0.3, 0.05, Height);
                color = mix(water, grass, t);
        }
        else if(Height < 0.1) {
                float t = smoothstep(0.05, 0.1, Height);
                color = mix(grass, rock, t);
        } else if(Height < 0.4) {
                float t = smoothstep(0.2, 0.4, Height);
                color = mix(rock, snow, t);
        } else {
                color = texture(texture_diffuse3, UV);
        }


}
