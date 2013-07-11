//
//  Shader.fsh
//  ipadplayer
//
//  Created by Atilim Cetin on 6/11/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
