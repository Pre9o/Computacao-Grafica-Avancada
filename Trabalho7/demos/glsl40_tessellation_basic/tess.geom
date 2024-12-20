#version 400 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out vec3 normal;

void main() {
    vec3 v0 = gl_in[0].gl_Position.xyz;
    vec3 v1 = gl_in[1].gl_Position.xyz;
    vec3 v2 = gl_in[2].gl_Position.xyz;

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    normal = normalize(cross(edge1, edge2));

    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
