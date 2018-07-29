#version 330 core
in  vec2 fPosition;
out vec4 color;

uniform vec2 uSize;

vec4
fragment
(vec2 p)
{
  return vec4(1, p.x, p.y, 1);
}

void main()
{
  color.rgba = fragment(fPosition);
}
