//
//  Shader.fsh
//  UltraPew-IOS
//
//  Created by Justin Bowes on 2013-05-13.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
