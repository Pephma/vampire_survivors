// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE.txt for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// Input from vertex shader
in vec2 fragTexCoord;

// This corresponds to the output color to the color buffer
out vec4 outColor;

uniform vec3 uColor;
uniform sampler2D uTexture;
uniform float uTextureFactor;
uniform vec4 uTexRect;

void main()
{
	if (uTextureFactor > 0.0) {
		// Calculate adjusted texture coordinates based on sprite sheet rect
		vec2 texCoord = uTexRect.xy + fragTexCoord * uTexRect.zw;
		
		// Sample color from texture
		vec4 texColor = texture(uTexture, texCoord);
		
		// Mix between solid color and texture based on textureFactor
		outColor = texColor * vec4(uColor, 1.0);
	} else {
		// No texture, just use solid color
		outColor = vec4(uColor, 1.0);
	}
}
