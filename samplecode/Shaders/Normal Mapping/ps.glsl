uniform lowp sampler2D g_Texture;
uniform lowp vec4 g_Color;

uniform mediump vec4 lightPos;

varying highp vec2 texCoord;
varying mediump vec2 position;

void main()
{
	lowp vec3 color0 = texture2D(g_Texture, texCoord).rgb;
	lowp vec3 color1 = vec3(0.3, 0.3, 0.3);
	mediump vec3 normal = texture2D(g_Texture, texCoord + vec2(0.5, 0.0)).rgb * 2.0 - 1.0;
	mediump vec3 lightDir = normalize(vec3(lightPos.xy, 150) - vec3(position.xy, 0));
	mediump vec3 halfdir = normalize(normalize(lightDir) + vec3(0, 0, 1));

	lowp float diff = max(0.0, dot(normal, lightDir));
	mediump float nh = max(0.0, dot(normal, halfdir));
	mediump float spec = pow(nh, 10.0);
	
	gl_FragColor = g_Color * vec4(color0 * diff + color1 * spec, 1);
}
