attribute vec4 POSITION0;
attribute vec2 TEXCOORD0;

uniform mat4 g_MVPMatrix;

varying vec2 position;
varying vec2 texCoord;

void main()
{
	texCoord = TEXCOORD0;
	position = POSITION0.xy;
	gl_Position = g_MVPMatrix * POSITION0;
}
