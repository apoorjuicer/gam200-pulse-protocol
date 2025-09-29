R"( #version 450 core
out vec4 FragColor;

uniform vec3 uColor = vec3(1.0, 1.0, 1.0);  // Default white

void main() {
    FragColor = vec4(uColor, 1.0);  // Use the uniform color
}
)"