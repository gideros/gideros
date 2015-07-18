attribute highp vec3 vVertex;
attribute mediump vec2 vTexCoord;
uniform highp mat4 vMatrix;
varying mediump vec2 fTexCoord;

void main() {
  vec4 vertex = vec4(vVertex,1.0);
  gl_Position = vMatrix*vertex;
  fTexCoord=vTexCoord;
}
