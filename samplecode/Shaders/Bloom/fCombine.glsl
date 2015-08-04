uniform lowp float BloomIntensity;
uniform lowp float BaseIntensity; 
uniform lowp float BloomSaturation;
uniform lowp float BaseSaturation;

uniform lowp sampler2D BloomSampler;
uniform lowp sampler2D BaseSampler;
varying mediump vec2 fTexCoord;

lowp vec4 AdjustSaturation(lowp vec4 color, lowp float saturation)
{
    // The constants 0.3, 0.59, and 0.11 are chosen because the
    // human eye is more sensitive to green light, and less to blue.
    lowp float grey = dot(color, vec4(0.3, 0.59, 0.11,1.0));
 
    return mix(vec4(grey,grey,grey,1), color, saturation);
}
 
void main() {
 lowp vec4 bloom=texture2D(BloomSampler, fTexCoord);
 lowp vec4 base=texture2D(BaseSampler, fTexCoord);
 
 // Adjust color saturation and intensity.
 bloom = AdjustSaturation(bloom, BloomSaturation) * BloomIntensity;
 base = AdjustSaturation(base, BaseSaturation) * BaseIntensity;
 
 // Darken down the base image in areas where there is a lot of bloom,
 // to prevent things looking excessively burned-out.
 base *= (1.0 - clamp(bloom,0.0,1.0));
 
    // Combine the two images.
 gl_FragColor = base+bloom;
}
