R"( #version 450 core
layout(location = 0) in vec2 aPos;      // matches your VAO
layout(location = 1) in vec3 aColor;    // (unused but kept for layout parity)
layout(location = 2) in vec2 aTex;      // (unused here)

uniform mat3 uModel_to_NDC;

void main() {
    // Use the uniform so it cannot be optimized out
    vec3 ndc = uModel_to_NDC * vec3(aPos, 1.0);
    gl_Position = vec4(ndc.xy, 0.0, 1.0);
}
)"