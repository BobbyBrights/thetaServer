
/*
	pulled from the theta sdk: UVSphere.m

	needs to be called twice, to make each half the sphere:
	mesh1 = thetaMesh( 2, 48, 0 );
	mesh2 = thetaMesh( 2, 48, Math.PI );
*/

//return the geometry triangle strip
function thetaMesh(radius,divide,rotate)
{
	var vertices = [];
	var texCoords = [];

	var i,j,altitude,altitudeDelta,azimuth;

	var mDivide = divide;



        vertexArray = malloc(sizeof(GLfloat *)*mDivide/2);
        texCoordsArray = malloc(sizeof(GLfloat *)*mDivide/2);
        
        for(i = 0; i < (mDivide/2); i++){

            altitude      = M_PI/2.0 - ( i ) * (M_PI*2/mDivide);
            altitudeDelta = M_PI/2.0 - (i+1) * (M_PI*2/mDivide);
            
            GLfloat *vertices = malloc(sizeof(GLfloat)*((mDivide+1)*6));
            GLfloat *texCoords = malloc(sizeof(GLfloat)*((mDivide+1)*4));
            
            for(j = 0; j <= mDivide/2; j++){
                
                azimuth = rotate - ((float)j) * (2*M_PI/(float)(mDivide));

                // 1st point
                vertices[j*6+0] = radius * cos(altitudeDelta) * cos(azimuth);
                vertices[j*6+1] = radius * sin(altitudeDelta);
                vertices[j*6+2] = radius * cos(altitudeDelta) * sin(azimuth);
                
                texCoords[j*4+0] =  1.0 - (2.0 * j / (float)(mDivide));
                texCoords[j*4+1] =  2*(i + 1) / (float)(mDivide);
                
                // 2nd point
                vertices[j*6+3] = radius * cos(altitude) * cos(azimuth);
                vertices[j*6+4] = radius * sin(altitude);
                vertices[j*6+5] = radius * cos(altitude) * sin(azimuth);
                
		texCoords[j*4+2] =  1.0 - (2.0 * j / (float)(mDivide));
                texCoords[j*4+3] =  2 * i / (float)(mDivide);
            }
            
            vertexArray[i] = vertices;
            texCoordsArray[i] = texCoords;
        }


	return { mesh:vertices, texCoords:texCoords };
}

//return the UV array
function thetaUV()
{

}

