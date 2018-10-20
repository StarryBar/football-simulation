/*****************************
 * File: fshaderfire.glsl
 *       A simple fragment shader
 *****************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color;
in  vec4 particle_Now;
out vec4 fColor;


void main() 
{ 
if(particle_Now.y<0.1)
{
	discard;
}
 	fColor = color;

} 

