#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 2) in vec2 vertices_uv;

//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform sampler2D texture_height1;

// Output
out vec2 UV;
out float Height;

void main(){

        // TODO : Output position of the vertex, in clip space : MVP * position
        float height = texture(texture_height1, vertices_uv).r;
        vec3 newPos = vertices_position_modelspace;
        newPos.y = height - 0.5;
        gl_Position = MVP * vec4(newPos,1);
        UV = vertices_uv;
        Height = newPos.y;

}

