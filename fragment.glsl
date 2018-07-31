#version 330 core
in  vec2 fPosition;
out vec4 color;

uniform float uTime;
uniform vec2 uSize;

vec4
fragment
(vec2 p)
{
  float s = sin(uTime);
  return vec4(s*s, p.x, p.y, 1);
}

void main()
{
  color.rgba = fragment(fPosition);
}
