/***************************
 * File: vshaderfire.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of typemat4.
 ***************************/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec3 particle_clr;
in  vec3 particle_spd;
out vec4 color;
out vec4 particle_Now;

uniform mat4 model_view;
uniform mat4 projection;
uniform vec3 particle_pos;//initial pos
uniform float T;

//you have a problem with the y translation! non-uniform units with the T&T^2 elements
void main() 
{
    mat4 activate=mat4(
        1,  0,  0,  particle_spd.x*T*0.001,
        0,  1,  0,  particle_spd.y*T*0.001-0.5*0.5*0.00000098*pow(T,2),
        0,  0,  1,  particle_spd.z*T*0.001,
        0,  0,  0,  1);

//compute the position 
    particle_Now = transpose(activate) * vec4(particle_pos,1.0);
    gl_Position = projection * model_view * particle_Now;
    color=vec4(particle_clr,1.0);
   
} 
