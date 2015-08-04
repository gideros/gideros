uniform lowp sampler2D fTexture;
varying mediump vec2 fTexCoord;
#define SAMPLE_COUNT 15
 
uniform mediump vec4 SampleOffsets[SAMPLE_COUNT];
uniform mediump float SampleWeights[SAMPLE_COUNT];
 
void main() {
 lowp vec4 c=vec4(0.0,0.0,0.0,0.0);
 for (int i=0; i <SAMPLE_COUNT; i++)
	c+=texture2D(fTexture, fTexCoord+SampleOffsets[i].xy)*SampleWeights[i];
 gl_FragColor = c;
}
