#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec3 normal;

//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 Normal;
out vec3 Position;

void main(){

        // TODO : Output position of the vertex, in clip space : MVP * position
        gl_Position = Projection * View * Model * vec4(vertices_position_modelspace, 1.0);
        Position = vec3(Model * vec4(vertices_position_modelspace, 1.0));
        Normal = mat3(transpose(inverse(Model))) * normal;
}

