//
//  Shader.fsh
//  iphoneplayer3
//
//  Created by Atilim Cetin on 3/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
