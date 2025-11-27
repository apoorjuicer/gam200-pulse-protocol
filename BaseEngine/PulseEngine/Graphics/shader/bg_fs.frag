#version 450 core
	layout (location=0) in vec3 vColor;
    	layout (location=1) in vec2 vTex;

	layout (location=0) out vec4 color;
    	uniform sampler2D uTex;

	void main () {
        vec4 texColor = texture(uTex, vTex);
        if(texColor.a < 0.1)
            discard;
        color = texColor;
	}
