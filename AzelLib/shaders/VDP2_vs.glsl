in vec3 a_position;
in vec2 a_texcoord;
out  highp vec2 pixelCoordinate;
void main()
{
   gl_Position = vec4(a_position, 1);
   pixelCoordinate = a_position.xy;
}

