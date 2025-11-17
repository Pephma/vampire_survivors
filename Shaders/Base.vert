// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE.txt for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// Attribute 0 is position (xyz), 1 is tex coords (uv)
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;

// Output tex coord to fragment shader
out vec2 fragTexCoord;

uniform mat4 uWorldTransform;
uniform mat4 uOrthoProj;

void main()
{
	gl_Position = uOrthoProj * uWorldTransform * vec4(inPosition, 1.0);
	fragTexCoord = inTexCoord;
}

