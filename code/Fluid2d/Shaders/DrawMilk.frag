
#version 450

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D textureSdf;

void main() {

	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i = -10; i <= 10; i++) {
		for(int j = -10; j <= 10; j++) {
			float x = i / 1000.0;
			float y = j / 1000.0;
			color += texture(textureSdf, texCoord + vec2(x, y));
		}
	}
	FragColor = color / 441;
	if (FragColor.x < 0.5) {
		FragColor = vec4(1.0, 1.0, 0.9, 1.0);
	}
	else {
		FragColor = vec4(0.5, 0.8, 1.0, 1.0);
	}
}
