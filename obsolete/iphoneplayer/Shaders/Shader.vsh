//
//  Shader.vsh
//  iphoneplayer
//
//  Created by Atilim Cetin on 2/28/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

attribute vec4 position;
attribute vec4 color;

varying vec4 colorVarying;

uniform float translate;

void main()
{
	gl_Position = position;
	gl_Position.y += sin(translate) / 2.0;
	
	colorVarying = color;
}
