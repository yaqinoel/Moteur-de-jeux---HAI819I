#version 330 core

in vec3 Position;
in vec3 Normal;

// Ouput data
out vec4 color;

uniform vec3 viewPos;

void main(){
        vec3 lightPos = vec3(0, 100.0, 0); // 光源在右上角
        vec3 lightColor = vec3(1.0, 1.0, 1.0);  // 白光
        vec3 objectColor = vec3(0.5, 0.7, 0.6); // 兔子的基础颜色 (你原本的颜色)

        // Ambient
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - Position); // 从片段指向光源的向量
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - Position); // 从片段指向相机的向量
        vec3 reflectDir = reflect(-lightDir, norm);  // 光线的反射向量

        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;

        vec3 result = (ambient + diffuse + specular) * objectColor;
        color = vec4(result, 1.0);

}
