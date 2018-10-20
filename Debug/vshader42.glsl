/***************************
 * File: vshader42.glsl:
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

in  vec3 vPosition;
in  vec3 vColor;
in  vec3 vNormal;
in  vec2 vTexCoord; //uv map size|square
out vec3 vPositionF;
out vec4 color;

out vec2 TexCoord;//for ground
out float TexCoord1D;//for sphere
out vec2 TexCoord2Ds;//for sphere
uniform int textureType;//0 for contour lines;1 for checkboard on sphere

uniform mat4 model_view;
uniform mat4 projection;
uniform mat3 Normal_Matrix;


uniform float shadowflag;
uniform int noRenderTag;
uniform int stillObj;
uniform vec3 LightType;
uniform int smapType;//0 for vertical;1 for slant
uniform int smapFrameType;//0 for world frame mapping ;1 for eye frame

uniform int latticeType;
out vec2 TexCoord2D_lattice;

uniform vec4 GlobalProduct;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 PosAmbientProduct,PosDiffuseProduct,PosSpecularProduct;
uniform vec4 LightDirection;   // parallel in eye frame
uniform vec4 PosLightPosition_e;  //positional in eye frame
uniform vec4 SpotlightAt;       //positional_at in eye frame
uniform float Shininess; //material specular coefficient
uniform float SpotExp;  //spotlight coefficient
uniform float SpotCutoff;//spot light angle

uniform float ConstAtt,POSConstAtt;  // Constant Attenuation
uniform float LinearAtt,POSLinearAtt; // Linear Attenuation
uniform float QuadAtt,POSQuadAtt;   // Quadratic Attenuation

void main() 
{
    TexCoord=vTexCoord;//such a terrible neglitable sentence!!!
    vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
    vec4 vColor4 = vec4(vColor.r, vColor.g, vColor.b, shadowflag);
    vec3 pos = (model_view*vPosition4).xyz;
//-----------------directional light------------------
vec4  ambient;
vec4  diffuse;
vec4  specular;
float attenuation= 0.0;
float spotfct = 1.0;
//--------------------------------------------------------


    if(LightType[0]==1)//directional light
    {
        
        vec3 L = normalize(-LightDirection.xyz);
        vec3 E = normalize(-pos);
        vec3 H = normalize(L+E);
        vec3 N;
        if(stillObj==1){N = normalize((model_view*vec4(vNormal,0)).xyz);} 
        else {N = normalize(Normal_Matrix*vNormal);}
     
    //-- To Do: Compute attenuation 
        attenuation = 1.0; 

    // Compute terms in the illumination equation
        ambient = AmbientProduct;

        float d = max( dot(L, N), 0.0 );
        diffuse = d * DiffuseProduct;

        float s = pow( max(dot(N, H), 0.0), Shininess );
        specular = s * SpecularProduct;
        
        if( dot(L, N) < 0.0 ) {	specular = vec4(0.0, 0.0, 0.0, 1.0);    } 
    }


//----------positional light------------------------------
vec4  ambient_pos;
vec4  diffuse_pos;
vec4  specular_pos;
float attenuation_pos= 0.0;
//--------------------------------------------------------

    if(LightType[1]==1||LightType[2]==1)//spot,point sources
    {
//something terrible happens here: must apply model_view to vPosition4
        float distance = length((PosLightPosition_e - model_view * vPosition4).xyz);
        vec3 L1 = normalize((PosLightPosition_e - model_view * vPosition4).xyz);
        vec3 E1 = normalize(-pos);
        vec3 H1 = normalize(L1+E1);
        vec3 N1;
        if(stillObj==1){N1 = normalize((model_view*vec4(vNormal,0)).xyz);} 
        else {N1 = normalize(Normal_Matrix*vNormal);}
         
//--- To Do: Compute attenuation ---
        attenuation_pos = 1/(POSConstAtt+POSLinearAtt*distance+POSQuadAtt*distance*distance); 

        if(LightType[1]==1)    //if using spot light
        {
//must first.xyz then normalize
            vec3 Lf= normalize((SpotlightAt - PosLightPosition_e).xyz);
            float cos=dot(Lf,-L1);
            spotfct=pow(cos,SpotExp);
            if(cos>cos(SpotCutoff)) {spotfct=0.0;}
        }

// Compute terms in the illumination equation
        ambient_pos = PosAmbientProduct;

        float d1 = max(dot(L1, N1), 0.0 );
        diffuse_pos = d1 * PosDiffuseProduct;

        float s1 = pow( max(dot(N1, H1), 0.0), Shininess );
        specular_pos = s1 * PosSpecularProduct;
        
        if( dot(L1, N1) < 0.0 ) { specular_pos = vec4(0.0, 0.0, 0.0, 1.0);  } 
     }
//transfer the variable position to fshader,
//after transfer,we get the interporated position.
    vPositionF=(projection * model_view * vPosition4).xyz;

//compute the position 
    gl_Position = projection * model_view * vPosition4;

//compute the color
    if (noRenderTag==1) {color = vColor4;}
    else  {color = GlobalProduct + attenuation * (ambient + diffuse + specular) + attenuation_pos * spotfct * (ambient_pos + diffuse_pos + specular_pos);}//
   
//1D/2D texture mapping ways  
vec3 tempPos;
    if(smapFrameType==0)
    tempPos=vPosition;
    if(smapFrameType==1)//control for swimming
    tempPos=(model_view * vPosition4).xyz;

    if(textureType==0&&smapType==0)// control the 1D orientation verticle
    TexCoord1D=2.5 * tempPos.x;
    if(textureType==0&&smapType==1)// control the 1D orientation horizontal
    TexCoord1D=1.5*(tempPos.x+tempPos.y+tempPos.z);

    if(textureType==1&&smapType==0)// control the 2D orientation verticle
    {
        TexCoord2Ds.x=0.5*(tempPos.x+1);
        TexCoord2Ds.y=0.5*(tempPos.y+1);
    }
    if(textureType==1&&smapType==1)// control the 2D orientation horizontal
    {
        TexCoord2Ds.x=0.3*(tempPos.x+tempPos.y+tempPos.z);
        TexCoord2Ds.y=0.3*(tempPos.x-tempPos.y+tempPos.z);
    }
//generate the lattice texture map.
    if(latticeType==0)//don't swim, 2D UPright
    {
        TexCoord2D_lattice.x=0.5*(vPosition.x+1);
        TexCoord2D_lattice.y=0.5*(vPosition.y+1);
    }
    if(latticeType==1)//don't swim, 2D tilted
    {
        TexCoord2D_lattice.x=0.3*(vPosition.x+vPosition.y+vPosition.z);
        TexCoord2D_lattice.y=0.3*(vPosition.x-vPosition.y+vPosition.z);
    }
} 
