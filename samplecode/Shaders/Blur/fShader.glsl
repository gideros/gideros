uniform lowp vec4 fColor;
uniform lowp sampler2D fTexture;
uniform int fRad;
uniform mediump vec4 fTexelSize;
varying mediump vec2 fTexCoord;

void main() {
 lowp vec4 frag=vec4(0,0,0,0);
 int ext=2*fRad+1;	
 mediump vec2 tc=fTexCoord-fTexelSize.xy*fRad;
 for (int v=0;v<ext;v++)	
 {
	frag=frag+texture2D(fTexture, tc);
    tc+=fTexelSize.xy;
 }
 frag=frag/ext;
 if (frag.a==0.0) discard;
 gl_FragColor = frag;
}
