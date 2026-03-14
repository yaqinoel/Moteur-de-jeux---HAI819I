#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 2) in vec2 vertices_uv;

//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform float time;

// Output
out vec2 UV;

void main(){

        // 计算绕 Y 轴旋转的矩阵
        float angle = time * 0.5;
        float c = cos(angle);
        float s = sin(angle);
        
        mat4 rotationY = mat4(
            c,   0.0, -s,   0.0,
            0.0, 1.0,  0.0, 0.0,
            s,   0.0,  c,   0.0,
            0.0, 0.0,  0.0, 1.0
        );

        vec4 rotated_position = rotationY * vec4(vertices_position_modelspace, 1.0);
        gl_Position = MVP * rotated_position;
        UV = vertices_uv;
}

