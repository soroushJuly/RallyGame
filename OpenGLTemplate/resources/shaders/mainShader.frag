#version 400 core

in vec2 vTexCoord;			// Interpolated texture coordinate using texture coordinate from the vertex shader
in vec3 n;
in vec4 p;

float rho = 0.005f;
vec3 fogColour = vec3(0.6f, 0.46f, 0.32f);

out vec4 vOutputColour;		// The output colour

uniform sampler2D sampler0, sampler1;  // The texture sampler
uniform samplerCube CubeMapTex;
uniform bool bUseTexture;    // A flag indicating if texture-mapping should be applied
uniform bool renderSkybox;
uniform bool lightingOn;
uniform bool isCartoon;
uniform bool isMultiTexture;
uniform bool isBlur;
in vec3 worldPosition;

// Structure holding light information:  its position as well as ambient, diffuse, and specular colours
struct LightInfo
{
	vec4 position;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
	vec3 direction;
	float exponent;
	float cutoff;
};

// Structure holding material information:  its ambient, diffuse, and specular colours, and shininess
struct MaterialInfo
{
	vec3 Ma;
	vec3 Md;
	vec3 Ms;
	float shininess;
};

// Lights and materials passed in as uniform variables from client programme
uniform LightInfo light1;
uniform LightInfo lights[5];
uniform LightInfo breakLightRight; 
uniform MaterialInfo material1; 

const int levels = 3;

// This function implements the Phong shading model
// The code is based on the OpenGL 4.0 Shading Language Cookbook, Chapter 2, pp. 62 - 63, with a few tweaks. 
// Please see Chapter 2 of the book for a detailed discussion.
vec3 PhongModel(vec4 eyePosition, vec3 eyeNorm)
{
	vec3 s = normalize(vec3(lights[0].position - eyePosition));
	vec3 v = normalize(-eyePosition.xyz);
	vec3 r = reflect(-s, eyeNorm);
	vec3 n = eyeNorm;
	vec3 ambient = lights[0].La * material1.Ma;
	float sDotN = max(dot(s, n), 0.0f);
	vec3 diffuse = lights[0].Ld * material1.Md * sDotN;
	vec3 specular = vec3(0.0f);
	float eps = 0.000001f; // add eps to shininess below -- pow not defined if second argument is 0 (as described in GLSL documentation)
	if (sDotN > 0.0f) 
		specular = lights[0].Ls * material1.Ms * pow(max(dot(r, v), 0.0f), material1.shininess + eps);
	

	return ambient + diffuse + specular;

}

vec3 BlinnPhongSpotlightModel(LightInfo light, vec4 p, vec3 n)
{
	vec3 s = normalize(vec3(light.position - p));
	float angle = acos(dot(-s, light.direction));
	float cutoff = radians(clamp(light.cutoff, 0.0, 90.0));
	vec3 ambient = light.La * material1.Ma;
	if (angle < cutoff) {
		float spotFactor = pow(dot(-s, light.direction), light.exponent);
		vec3 v = normalize(-p.xyz);
		vec3 h = normalize(v + s);
		float sDotN = max(dot(s, n), 0.0);
		vec3 diffuse = light.Ld * material1.Md * sDotN;
		vec3 specular = vec3(0.0);
		if (sDotN > 0.0)
			specular = light.Ls * material1.Ms * pow(max(dot(h, n), 0.0), material1.shininess);
		return ambient + spotFactor * (diffuse + specular);
	} else
		//return vec3(0.0);
		return ambient;
}


void main()
{
	// Fog calculation
	float d = length(p.xyz);
	float w = exp(-(rho*d)*(rho*d));

	// Calculate lights
	vec3 vColour = vec3(1.0f);
	if(lightingOn)
	{
		vec3 vColourDirectional = PhongModel(p , normalize(n));
		vec3 vColourSpotlight = vec3(0.0f);

		for(int i=1;i<lights.length();++i)
		{
		  vColourSpotlight += BlinnPhongSpotlightModel(lights[i], p, n);
		}
		vColour = vColourDirectional + vColourSpotlight;
	}

	// Render Skybox
	if (renderSkybox) 
	{
		vOutputColour = texture(CubeMapTex, worldPosition);
		float interpolate = 1/(worldPosition.y * 0.03f);
		if(worldPosition.y<0.f)
			interpolate = 1.f;
		if(interpolate>1)
			interpolate = 1.f;
		vOutputColour = mix(vOutputColour, vec4(fogColour, 1.f) , interpolate);
	} 
	else {
		// Get the texel colour from the texture sampler
		vec4 vTexColour = vec4(1.f);	
		vec4 vTexColour0 = texture(sampler0, vTexCoord);	
		vec4 vTexColour1 = texture(sampler1, vTexCoord);
		// Calculate multiTexturing (if "on")
		if(isMultiTexture)
		{
			if(vTexCoord.x > 1.f)
				vTexColour = mix(vTexColour0, vTexColour1, 1 * (vTexCoord.x - 1.f));
			else
			{
				vTexColour = mix(vTexColour1, vTexColour0, 1 * vTexCoord.x);
			}
		}
		else
		{
			vTexColour = vTexColour0;
		}

		// Make the transparent textures work
		    if(vTexColour.a < 0.1)
        discard;

		// calculate cartoon shading
		vec3 quantisedTextColour = floor(vTexColour.rgb * levels) / levels;
		vec4 cartoonShading = vec4(1.0);
		if (isCartoon)
			cartoonShading = vec4(quantisedTextColour, 1.0);

		if (bUseTexture)
		{
			// Use mix to linearly interpolate fog
			vOutputColour = vTexColour * vec4(vColour, 1.0f) * cartoonShading;
			// Calculate radial blur if in "on"
			if (isBlur)
			{
				const float sampleDist = 1.0;
				const float sampleStrength = 2.2; 
				float samples[10];
				samples[0] = -0.085;
				samples[1] = -0.06;
				samples[2] = -0.03;
				samples[3] = -0.02;
				samples[4] = -0.01;
				samples[5] =  0.01;
				samples[6] =  0.02;
				samples[7] =  0.03;
				samples[8] =  0.06;
				samples[9] =  0.085;

				vec2 dir = 0.5 - vTexCoord; 
				float dist = sqrt(dir.x*dir.x + dir.y*dir.y); 
				dir = dir/dist;

				vec4 color = texture(sampler0,vTexCoord); 
				vec4 sum = color;

				for (int i = 0; i < 10; i++)
					sum += texture( sampler0, vTexCoord + dir * samples[i] * sampleDist );

				sum *= 1.0/11.0;
				float t = dist * sampleStrength;
				t = clamp( t ,0.0,1.0);

				vOutputColour = mix( color, sum, t );
			}
			if (lightingOn)
			{
				vOutputColour.rgb = mix(fogColour, vOutputColour.rgb, w);
			}
		}
		// If texture is off (which hasn't happend yet)
		else
		{
			vOutputColour = vec4(vColour, 1.0f);	// Just use the colour instead
			vOutputColour.rgb = mix(fogColour, vOutputColour.rgb, w);
		}
	}	
}
