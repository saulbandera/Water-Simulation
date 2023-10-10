
Texture2D gridTexture : register(t0);
SamplerState samplerGrid : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}


cbuffer CameraBuffer : register(b1)
{
    float3 cameraPosition;
    float toggleGerstner;
    float2 waveDirection;
};

cbuffer TimeBuffer : register(b2)
{
    float time;
    float defSpeed;
    float defAmplitude;
    float defSteepness;
    float2 PI_wavelength;
}

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float3 heightVals : TEXCOORD3;
};



struct Wave
{
    float wavenumber;
    float amplitude;
    float WA;
    float phaseSpeed;
    float2 direction;
    float var;
    float steepness;
};

Wave calculateWave(float wavelengthMul, float ampMul, float speedMul, float2 direction, float2 positionXZ)
{
    Wave wave;
    
    wave.wavenumber = (2 * PI_wavelength.x) / (PI_wavelength.y * wavelengthMul); // 2*pi/wavelength - frequency of waves
    wave.amplitude = defAmplitude * ampMul; // height of waves
    wave.WA = wave.wavenumber * wave.amplitude; // reduce the number of calculations 
    wave.phaseSpeed = defSpeed * wave.wavenumber * speedMul; // speed of wave movement
    wave.direction = normalize(direction); // direction in which the waves travel 
    wave.var = wave.wavenumber * dot(wave.direction, positionXZ) + wave.phaseSpeed * time; // reduce num calculations
    wave.steepness = defSteepness / (wave.WA * 3); // horizontal amplitude(steepness)
    
    return wave;
}

float3 waveGetOffset(Wave wave)
{
    // precalculate
    float sac = wave.steepness * wave.amplitude * cos(wave.var);
    return float3(
        sac * wave.direction.x,
        wave.amplitude * sin(wave.var),
        sac * wave.direction.y
    );
}

float3 waveGetNormal(Wave wave, float2 positionXZ)
{
    float phaseVel = wave.phaseSpeed * time;
    float A = wave.wavenumber * dot(wave.direction, positionXZ) + phaseVel;
    
    return float3(
        wave.direction.x * wave.WA * cos(A),
        wave.steepness * wave.WA * sin(A),
        wave.direction.y * wave.WA * cos(A)
    );
}

OutputType main(InputType input)
{
    OutputType output;



    if (toggleGerstner == 1)
    {
    	// definining the gerstner waves:
        // 3 for smoother surface, 4 for smooth surface over large wave
        int numWaves = 4;
        Wave waves[] =
        {
            calculateWave(1.0, 1.0, 1.0, float2(1, 0.5), input.position.xz),
            calculateWave(1.4, 0.9, 0.6, float2(0.6, 0.7), input.position.xz),
            calculateWave(2.7, 0.6, 1.2, float2(0.3, 0.5), input.position.xz),

            calculateWave(4.0, 3.04, 1.5, float2(1, 0.6), input.position.xz),


        };
        
        input.normal = 0;

        for (uint i = 0; i < numWaves; ++i)
        {
            // gerstner wave function: moves the vertices of a plane in a circular motion 
            // as the waves pass by giving the rolling effect that water waves have
            input.position.xyz += waveGetOffset(waves[i]);

            // calculate normals using the cross product of the tangent and the bitangent of the gerstner wave
            input.normal += waveGetNormal(waves[i], input.position.xz);
        }
        
        input.normal.x = -input.normal.x;
        input.normal.y = 1 - input.normal.y;
        input.normal.z = -input.normal.z;
    }
    else if(toggleGerstner ==0)
    {
        // using two sine waves to manipulate the vertices
    	//// Defining wave parameters
        float frequency1 = 0.2; // Frequency of the first wave
        float amplitude1 = 1.2; // Amplitude of the first wave
        float speed1 = 1.0; // Speed of the first wave

        float frequency2 = 0.2; // Frequency of the second wave
        float amplitude2_ = 0.5; // Amplitude of the second wave
        float speed2 = 1.5; // Speed of the second wave

    	// Superposition of two sine waves for displacement
        input.position.y = amplitude1 * sin(frequency1 * input.position.x + speed1 * time) + amplitude2_ * sin(frequency2 * input.position.z + speed2 * time);

    	// Calculating normals using the derivative of the y position with respect to x and z
        float dx = -amplitude1 * frequency1 * cos(frequency1 * input.position.x + speed1 * time) - amplitude2_ * frequency2 * cos(frequency2 * input.position.z + speed2 * time);
        float dy = 1.0;
        float dz = -amplitude1 * frequency1 * cos(frequency1 * input.position.x + speed1 * time) - amplitude2_ * frequency2 * cos(frequency2 * input.position.z + speed2 * time);



        input.normal = normalize(float3(dx, dy, dz));
    }
    else if(toggleGerstner == 3)
    {
	     
    	// [SWE] Getting height from the simulation grid
        int2 textureSize;
        gridTexture.GetDimensions(textureSize.x, textureSize.y);
        int2 location = int2(input.tex.x * textureSize.x, input.tex.y * textureSize.y);
        float height = gridTexture.Load(int3(location, 0)).r;

    	// [SWE] setting the y position of the mesh vertices to the height in the grid
        input.position.y = height;

        output.heightVals.r = height;
  
    }



	// Calculate the position of the vertex against the world direction, view, and projection matrices
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Store the texture coordinates for the pixel shader
    output.tex = input.tex;
  
    // Lighting calculations
    float4 worldPosition = mul(input.position, worldMatrix);
    output.viewVector = cameraPosition.xyz - worldPosition.xyz;
    output.viewVector = normalize(output.viewVector);
    output.worldPosition = mul(input.position, worldMatrix).xyz;

    // Calculate the normal vector against the world matrix only and normalize
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
    
    return output;


}