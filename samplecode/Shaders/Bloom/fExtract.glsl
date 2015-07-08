uniform lowp float BloomThreshold;
uniform lowp sampler2D fTexture;
varying mediump vec2 fTexCoord;
 
void main() {
 lowp vec4 c=texture2D(fTexture, fTexCoord);
 c=clamp((c-BloomThreshold)/(1.0-BloomThreshold), 0.0, 1.0);
 gl_FragColor = c;
}
