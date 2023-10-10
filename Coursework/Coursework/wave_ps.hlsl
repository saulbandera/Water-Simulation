Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);



cbuffer LightBuffer : register(b0)
{
    float4 ambient;
    float4 diffuse;
    float4 position;
    float4 specularPower;
    float4 specular;
    float4 direction;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float3 heightVals : TEXCOORD3;

};


float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate((diffuse) * intensity);
    return colour;
}

float4 calcSpecular(float3 lightDirection, float3 normal, float3 viewVector, float4 specularColour, float specularPower)
{
    // Blinn-phong specular calculation
    float3 halfway = normalize(lightDirection + viewVector);
    float specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower);
    return saturate((specularColour) * specularIntensity);
}

float4 main(InputType input) : SV_TARGET
{
    float4 lightColourResult = float(0);

    // Fetch the texture color
    float4 waveTexColour = texture0.Sample(sampler0, input.tex);
    waveTexColour.a = 0.50f;


    // Directional light
    float3 dir = normalize(-direction);

    // Calculate specular color
    float4 specularCol = calcSpecular(dir, input.normal, input.viewVector, specular, specularPower.x);


    // Accumulate light color
    lightColourResult += ambient + calculateLighting(dir, input.normal, diffuse) + specularCol;


    // Making the color more blue and darker as the depth increases
    float depth = input.worldPosition.y;
    waveTexColour.xyz *= lerp(1.0, 0.5, depth * 0.01); // Decrease color intensity with depth
    waveTexColour.z += lerp(0.3, 0.4, depth * 0.001); // Increase blue component with depth


	float4 finalResult = lightColourResult * waveTexColour;

    return finalResult;
}
