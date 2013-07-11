//
//  Shader.fsh
//  iphoneplayer2
//
//  Created by Atilim Cetin on 12/7/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
