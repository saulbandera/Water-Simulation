Texture2D predictedGridTexture : register(t0); 
Texture2D correctedGridTexture : register(t1);
Texture2D predictedGridRT : register(t2);
Texture2D correctedGridRT : register(t3);

SamplerState sampler0 : register(s0);


cbuffer TimeBuffer : register(b0)
{
    float DTDXDY;
    float gridSizeX;
    float gridSizeY;
    float firstPass;
};

cbuffer simulationBuffer : register(b1)
{
    float gravity;
    float n;
    float timeStepSize;
    float cr;

};

// Define structure to hold the output for both render targets
struct PixelShaderOutput
{
    float4 Target0 : SV_TARGET0;
    float4 Target1 : SV_TARGET1;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

//// Update the pixel shader function signature.
PixelShaderOutput main(InputType input)
{
    PixelShaderOutput output;

    if (firstPass == 1.f)
    {
        float2 textureCoord = input.tex;

        float4 pGridData = predictedGridTexture.Sample(sampler0, textureCoord);
        float4 cGridData = correctedGridTexture.Sample(sampler0, textureCoord);

        output.Target0 = pGridData; // Output the predicted grid data to the render texture
        output.Target1 = cGridData; // Output the corrected grid data to the render texture
    }

    else
    {
        

        /////////////////        MACCORMACK PREDICTOR STEP        /////////////////
        // The following code performs the predictor step of the MacCormack scheme used to solve the
        // 2D shallow water equations, provided by Hubbard and Baines (1997), based on 1D scheme by
        // Nurlathifah (2022). This step approximates the partial derivatives of space and time of
        // the shallow water equations using the forward finite difference scheme.

        // Obtaining the texture coordinates for sampling the render textures containing the grid data
        float2 textureCoord = float2(input.tex.x, 1.0f - input.tex.y); // Flip the y-coordinate

        float4 predictedGridRTData = predictedGridRT.Sample(sampler0, textureCoord);
        float4 correctedGridRTData = correctedGridRT.Sample(sampler0, textureCoord);


        float2 texelSize = 1.0 / float2(gridSizeX, gridSizeY); // Size of one texel

		// Obtaining the neighboring coordinates
        float2 leftCoord = textureCoord - float2(texelSize.x, 0.0);
        float2 rightCoord = textureCoord + float2(texelSize.x, 0.0);
        float2 topCoord = textureCoord - float2(0.0, texelSize.y);
        float2 bottomCoord = textureCoord + float2(0.0, texelSize.y);

    	// Sampling the corrected grid texture at neighboring coordinates
        float4 leftData = correctedGridRT.Sample(sampler0, leftCoord);
        float4 rightData = correctedGridRT.Sample(sampler0, rightCoord);
        float4 topData = correctedGridRT.Sample(sampler0, topCoord);
        float4 bottomData = correctedGridRT.Sample(sampler0, bottomCoord);


        // Obtaining u and v
        float u = correctedGridRTData.g / correctedGridRTData.r;
        float v = correctedGridRTData.b / correctedGridRTData.r;

        // Obtaining the necessary velocities u and v from the surrounding grid nodes:
        float rightVelU = rightData.g / rightData.r;
        float rightVelV = rightData.b / rightData.r;
        float bottomVelU = bottomData.g / bottomData.r;
        float bottomVelV = bottomData.b / bottomData.r;



        float F1 = rightData.g - correctedGridRTData.g;


        float G1 = bottomData.b - correctedGridRTData.b;

        float F2 = rightData.g * rightVelU + 0.5 * gravity * rightData.r * rightData.r
    	            - (correctedGridRTData.g * u + 0.5 * gravity * correctedGridRTData.r * correctedGridRTData.r);

        float G2 = bottomData.b * bottomVelU - correctedGridRTData.b * u;


        float F3 = rightData.g * rightVelV - correctedGridRTData.g * v;
        float G3 = bottomData.b * bottomVelV + 0.5 * gravity * bottomData.r * bottomData.r
                      - (correctedGridRTData.b * v + 0.5 * gravity * correctedGridRTData.r * correctedGridRTData.r);


        // Obtaining the new height and flux values from the corrected simulation grid data
        float newHP = correctedGridRTData.r - DTDXDY * (F1 + G1);
        float newQP = correctedGridRTData.g - DTDXDY * (F2 + G2);
        float newPP = correctedGridRTData.b - DTDXDY * (F3 + G3);

        // Update the values in the predicted simulation grid
        predictedGridRTData.r = newHP;
    	predictedGridRTData.g = newQP;
        predictedGridRTData.b = newPP;

    	// Pass the output
        output.Target0 = predictedGridRTData;
        output.Target1 = correctedGridRTData; 
    }

    return output;
}






