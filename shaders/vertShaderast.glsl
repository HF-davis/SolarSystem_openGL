#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;
out vec2 tc;

#define pi 3.141592

uniform mat4 v_matrix;
//uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
layout (binding=0) uniform sampler2D s;
uniform float tf;

mat4 buildRotateX(float grad);
mat4 buildRotateY(float grad);
mat4 buildRotateZ(float grad);
mat4 buildTranslate(float x, float y, float z);
mat4 buildScale(float x,float y, float z);

void main(void)
{	
	float w=gl_InstanceID + tf;
	float a,b,c;
	a=sin(w/5)*5.0;
    	b=cos(0.52*w)*5.0;
    	c=cos(w/5)*5.0;
    	float m;
    	mat4 localRotX=buildRotateX(3.0*w);
    	mat4 localRotY=buildRotateY(w);
    	mat4 localRotZ=buildRotateZ(3.0*w);
    	
    	if(gl_InstanceID%2==0){m=0.0;}
    	else if(gl_InstanceID%3==0){m=0.25;}
    	else{m=-0.25;}
    	
    	mat4 localTrans=buildTranslate(a,m,c);
    	mat4 localScale=buildScale(0.125,0.125,0.125);
    	mat4 newM_matrix=localTrans*localScale*localRotY;
    	mat4 mv_matrix=v_matrix*newM_matrix;
    	
	gl_Position = proj_matrix * mv_matrix * vec4(position,1.0);
	tc = tex_coord;
		
}


mat4 buildTranslate(float x,float y,float z) 
{
    //float rad = grad * M_PI/180.0;
    mat4 trans = mat4(1.0,  0.0, 0.0, 0.0,
                   0.0,       1.0, 0.0,     0.0,
                   0.0, 0.0, 1.0, 0.0,
                   x,       y, z,      1.0);
    return trans;
}

mat4 buildScale(float x,float y, float z)
{
	mat4 scale=mat4(x,0.0,0.0,0.0,
			0.0,y,0.0,0.0,
			0.0,0.0,z,0.0,
			0.0,0.0,0.0,1.0		
			);
	return scale;

}
mat4 buildRotateX(float grad) 
{
    float rad = grad; //grad * M_PI/180.0;
    mat4 xrot = mat4(1.0,  0.0, 0.0, 0.0,
                   0.0,     cos(rad), -sin(rad),0.0,
                   1.0, sin(rad), cos(rad), 0.0,
                   0.0,       0.0, 0.0,      1.0);
    return xrot;
}
mat4 buildRotateY(float grad) 
{
    float rad = grad; //* M_PI/180.0;
    mat4 yrot = mat4(cos(rad),  0.0, sin(rad), 0.0,
                   0.0,       1.0, 0.0,      0.0,
                   -sin(rad), 0.0, cos(rad), 0.0,
                   0.0,       0.0, 0.0,      1.0);
    return yrot;
}
mat4 buildRotateZ(float grad) 
{
    float rad = grad; //* M_PI/180.0;
    mat4 zrot = mat4(cos(rad),  sin(rad),0.0 , 0.0,
                   -sin(rad), cos(rad), 0.0,      0.0,
                   0.0, 0.0, 1.0, 0.0,
                   0.0,       0.0, 0.0,      1.0);
    return zrot;
}
