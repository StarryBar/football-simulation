/*****************************
 * File: fshader42.glsl
 *       A simple fragment shader
 *****************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color;
in  vec3 vPositionF;
in  vec2 TexCoord;
in  float TexCoord1D;
in  vec2 TexCoord2Ds;
in vec2 TexCoord2D_lattice;
out vec4 fColor;


uniform float fog_start;
uniform float fog_end;
uniform float fog_density;
uniform int fogtype;
uniform vec4 fog_color;

uniform int groundEnable;
uniform int gmapEnable;

uniform sampler2D Gtexture_2D; // sampler properties.
uniform sampler1D Gtexture_1D;

uniform int cubeEnable;
uniform int cubeFilled;

uniform int textureType;//0 for contour lines;1 for checkboard on sphere
uniform int smapFrameType;//0 for world frame mapping ;1 for eye frame
uniform int latticeType;//0 for vertical hole;1 for slanted hole
uniform int smapType;//0 for vertical;1 for slant
uniform int latticeEnable;
uniform int shadowEnable;
uniform int noRenderTag;

void main() 
{ 
float d=length(vPositionF);
float f;
float tempX,tempY; 
vec4 TColor=vec4(1.0,1.0,1.0,1.0);
switch(fogtype)
{
    case 0: f = 1;break;
	case 1: f =clamp((fog_end-d)/(fog_end-fog_start),0,1); break;
	case 2: f = exp(-fog_density*d);break;
	case 3: f =exp(-pow(fog_density*d,2)); break;//
}
	if(groundEnable==1&&gmapEnable==1)//for texture mapping
		{
			TColor=texture(Gtexture_2D,TexCoord);
		}
//down:when wireframe,cube dynamic control is placed in cpu drawObj.
	else if(cubeFilled==1&&cubeEnable==1)
		{
		if(textureType==0)
			TColor=texture(Gtexture_1D,TexCoord1D);
		if(textureType==1)
			{
			TColor=texture(Gtexture_2D,TexCoord2Ds);
			if(TColor.x==0.0) TColor=vec4(0.9,0.1,0.1,1.0);
			}
		//decide whether to discard:
		if(latticeEnable==1&&smapFrameType==0)//create sphere holes
			{
				tempX=fract(4*TexCoord2D_lattice.x);
				tempY=fract(4*TexCoord2D_lattice.y);
				if(tempX<0.35&&tempY<0.35) discard;
			}


		}

	if(shadowEnable==1&&latticeEnable==1&&smapFrameType==0&&noRenderTag==1)//create shadow holes
	{
		tempX=fract(4*TexCoord2D_lattice.x);
		tempY=fract(4*TexCoord2D_lattice.y);
		if(tempX<0.35&&tempY<0.35) discard;
	}


 	fColor = vec4(((f * TColor * color).xyz+(1-f)*fog_color.xyz),color.w);
 	
 	//Q1:I think should first blending then apply fog;otherwise you will blend two objects with fog together,which will count fog twice!!!!???
 	//Q2:I think should first blending then apply fog;only in this way you can utilize the 'alpha'value of fog?????!!!!!!
} 

