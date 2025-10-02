R"( #version 450 core
layout(location = 0) in vec2 aVertexPosition;
layout(location = 1) in vec3 aVertexColor;
layout(location = 2) in vec2 aTexture;

layout(location = 0) out vec3 vColor;
layout(location = 1) out vec2 vTex;

uniform mat3 uModel_to_NDC;                  // Model-to-NDC matrix
uniform vec2 uOffset;                        // UV offset for the current frame
uniform vec2 uScale;                         // UV scale for the current frame

void main() {
    gl_Position = vec4(vec2(uModel_to_NDC * vec3(aVertexPosition, 1.f)), 0.0, 1.0);
    // gl_Position = vec4(aVertexPosition, 0,1);
    vColor = aVertexColor;
    vTex = aTexture;
}
)"