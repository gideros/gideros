uniform lowp sampler2D g_Texture;
uniform lowp vec4 g_Color;
uniform highp float time;

varying highp vec2 texCoord;

void main()
{
	highp vec2 tc = texCoord.xy;
	highp float dist = cos(tc.x * 24.0 - time * 4.0) * 0.02;
	highp vec2 uv = tc + dist;
	gl_FragColor =  g_Color * texture2D(g_Texture, uv);
}
