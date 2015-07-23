attribute vec4 POSITION0;
attribute vec2 TEXCOORD0;

uniform mat4 g_MVPMatrix;

varying vec2 texCoord;

void main()
{
	gl_Position = g_MVPMatrix * POSITION0;
	texCoord = TEXCOORD0;
}
