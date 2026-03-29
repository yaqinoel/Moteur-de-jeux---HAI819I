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

        vec4 water = texture(texture_diffuse1, UV);
        vec4 grass = texture(texture_diffuse2, UV);
        vec4 rock  = texture(texture_diffuse3, UV);
        vec4 snow  = texture(texture_diffuse4, UV);

        // t1: 水到草的过渡 (中心在 0.0)
        float t1 = smoothstep(-0.1, 0.05, Height);

        // t2: 草到岩石的过渡 (中心在 0.15)
        float t2 = smoothstep(0.05, 0.25, Height);

        // t3: 岩石到雪地的过渡 (中心在 0.45)
        float t3 = smoothstep(0.25, 0.35, Height);

        // 3. 嵌套混合 (逐层覆盖)
        vec4 finalColor = water;
        finalColor = mix(finalColor, grass, t1); // 水上盖草
        finalColor = mix(finalColor, rock, t2);  // 草上盖石
        finalColor = mix(finalColor, snow, t3);  // 石上盖雪

        color = finalColor;


}
