#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 2) in vec2 vertices_uv;

//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform sampler2D texture_height1;

// Output
out vec2 UV;

void main(){

        // TODO : Output position of the vertex, in clip space : MVP * position
        gl_Position = Projection * View * Model * vec4(vertices_position_modelspace, 1.0);
        UV = vertices_uv;

}

