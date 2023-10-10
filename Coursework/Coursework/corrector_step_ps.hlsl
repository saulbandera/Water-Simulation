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

// Define a new structure to hold the output for both render targets.
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

float phi(float x)
{
    return max(0, min(2 * x, 1));
}


float G(float x)
{

    float C = 0.0f;
    if (cr <= 0.5)
    {
    	C = cr * (1 - cr);
    }
    else
    {
    	C = 0.5f;
    }
    return 0.5 * C * (1 - phi(x));
}

//// Update the pixel shader function signature.
PixelShaderOutput main(InputType input)
{
    PixelShaderOutput output;
    
	/////////////////        [MAIN SIMULATION LOOP]        /////////////////
	// The following code performs the corrector step of the MacCormack scheme used to solve the
	// 2D shallow water equations, provided by Hubbard and Baines (1997), based on 1D scheme by
	// Nurlathifah (2022). This step approximates the partial derivatives of space and time of
	// the shallow water equations using the backward finite difference scheme.

	// Obtaining the texture coordinates for sampling the render textures containing the grid data
	float2 textureCoord = float2(input.tex.x, 1.0f - input.tex.y); // Flip the y-coordinate

    float4 predictedGridRTData = predictedGridRT.Sample(sampler0, textureCoord);
    float4 correctedGridRTData = correctedGridRT.Sample(sampler0, textureCoord);
      
    float2 texelSize = 1.0 / float2(gridSizeX, gridSizeY); // Size of one texel

	// Neighboring coordinates
    float2 leftCoord = textureCoord - float2(texelSize.x, 0.0);
    float2 rightCoord = textureCoord + float2(texelSize.x, 0.0);
    float2 topCoord = textureCoord - float2(0.0, texelSize.y);
    float2 bottomCoord = textureCoord + float2(0.0, texelSize.y);

	// Sampling the predicted grid texture at neighboring coordinates
    float4 leftData = predictedGridRT.Sample(sampler0, leftCoord);
    float4 rightData = predictedGridRT.Sample(sampler0, rightCoord);
    float4 topData = predictedGridRT.Sample(sampler0, topCoord);
    float4 bottomData = predictedGridRT.Sample(sampler0, bottomCoord);


	// obtaining u and v:
    float u = predictedGridRTData.g / predictedGridRTData.r;
    float v = predictedGridRTData.b / predictedGridRTData.r;

	// obtaining velocities u and v for the surrounding grid nodes:
    float leftVelU = leftData.g / leftData.r;
    float leftVelV = leftData.b / leftData.r;
    float topVelU = topData.g / topData.r;
    float topVelV = topData.b / topData.r;



	float F1 = predictedGridRTData.g - leftData.g;
	float G1 = predictedGridRTData.b - topData.b;


	float F2 = predictedGridRTData.g * u + 0.5 * gravity * predictedGridRTData.r * predictedGridRTData.r
    	            - (leftData.g * leftVelU + 0.5 * gravity * leftData.r * leftData.r);

        
	float G2 = predictedGridRTData.b * u - topData.b * topVelU;
    float F3 = predictedGridRTData.g * v - leftData.g * leftVelV;

	float G3 = predictedGridRTData.b * v + 0.5 * gravity * predictedGridRTData.r * predictedGridRTData.r
                      - (topData.b * topVelV + 0.5 * gravity * topData.r * topData.r);


    float HP = 0.5 * (correctedGridRTData.r + predictedGridRTData.r - DTDXDY * (F1 + G1));
    float QP = 0.5 * (correctedGridRTData.g + predictedGridRTData.g - DTDXDY * (F2 + G2));
    float PP = 0.5 * (correctedGridRTData.b + predictedGridRTData.b - DTDXDY * (F3 + G3));


	// Update the values in corrected grid 
	correctedGridRTData.r = HP;
	correctedGridRTData.g = QP;
	correctedGridRTData.b = PP;
        
    
	   // uncompleted TVD-MacCormack scheme based on Kalita (2016)
       // float E1 = predictedGridRTData.g - bottomData.g;
       // float F1 = predictedGridRTData.b -leftData.b;
       // float E2 = predictedGridRTData.g * u + 0.5 * gravity * predictedGridRTData.r * predictedGridRTData.r
    	  //          - (bottomData.g * bottomVelU + 0.5*gravity*bottomData.r*bottomData.r);
       // float F2 = predictedGridRTData.b * u - leftData.b * leftVelU;
       // float S2 = -gravity * n * n * u * sqrt((u * u) + (v * v)) / pow(predictedGridRTData.r, 1 / 3);
       // float E3 = predictedGridRTData.g * v - bottomData.g * bottomVelU;
       // float F3 = predictedGridRTData.b * v + 0.5 * gravity * predictedGridRTData.r * predictedGridRTData.r
       //               - (leftData.b * leftVelV + 0.5 * gravity * leftData.r * leftData.r);
       // float S3 = -gravity * n * n * v * sqrt((u * u) + (v * v)) / pow(predictedGridRTData.r, 1 / 3);
       // float newHC = predictedGridRTData.r - DTDXDY * (E1 + F1);
       // float newQC = predictedGridRTData.g - DTDXDY * (E2 + F2) - timeStepSize * S2;
       // float newPC = predictedGridRTData.b - DTDXDY * (E3 + F3) - timeStepSize * S3;
       // // Update the values in gridRenderTextData
       //// correctedGridRTData.r = newHC;
       //// correctedGridRTData.g = newQC;
       //// correctedGridRTData.b = newPC;
       // // corrected grid rt data now holds updated values
       // float ehb = predictedGridRTData.r - bottomData.r;
       // float ehf = topData.r - predictedGridRTData.r;
       // float eqb = predictedGridRTData.g - bottomData.g;
       // float eqf = topData.g - predictedGridRTData.g;
       // float epb = predictedGridRTData.b - bottomData.b;
       // float epf = topData.b - predictedGridRTData.b;
       // float fhb = predictedGridRTData.r - leftData.r;
       // float fhf = rightData.r - predictedGridRTData.r;
       // float fqb = predictedGridRTData.g - leftData.g;
       // float fqf = rightData.g - predictedGridRTData.g;
       // float fpb = predictedGridRTData.b - leftData.b;
       // float fpf = rightData.b - predictedGridRTData.b;
       // float edhc = ehb * ehf;
       // float edqc = eqb * eqf;
       // float edpc = epb * epf;
       // float edhf = ehf * ehf;
       // float edqf = eqf * eqf;
       // float edpf = epf * epf;
       // float edhb = ehb * ehb;
       // float edqb = eqb * eqb;
       // float edpb = epb * epb;
       // float fdhc = fhb * fhf;
       // float fdqc = fqb * fqf;
       // float fdpc = fpb * fpf;
       // float fdhf = fhf * fhf;
       // float fdqf = fqf * fqf;
       // float fdpf = fpf * fpf;
       // float fdhb = fhb * fhb;
       // float fdqb = fqb * fqb;
       // float fdpb = fpb * fpb;
       // float rxplus = (edhc + edqc + edpc) / (edhf + edqf + edpf);
       // float rxminus = (edhc + edqc + edpc) / (edhb + edqb + edpb);
       // float ryplus = (fdhc + fdqc + fdpc) / (fdhf + fdqf + fdpf);
       // float ryminus = (fdhc + fdqc + fdpc) / (fdhb + fdqb + fdpb);
       // float AU1 = 0.5 * (predictedGridRTData.r + newHC);
       // float EDF1 = (G(rxplus) + G(rxminus)) * ehf;
       // float EDB1 = (G(rxplus) + G(rxminus) * ehb);
       // float FDF1 = (G(ryplus) + G(ryminus)) * fhf;
       // float FDB1 = (G(ryplus) + G(ryminus)) * fhb;
       // float newH = AU1 + EDF1 - EDB1 + FDF1 - FDB1;
       // float AU2 = 0.5 * (predictedGridRTData.g + newQC);
       // float EDF2 = (G(rxplus) + G(rxminus)) * eqf;
       // float EDB2 = (G(rxplus) + G(rxminus) * eqb);
       // float FDF2 = (G(ryplus) + G(ryminus)) * fqf;
       // float FDB2 = (G(ryplus) + G(ryminus)) * fqb;
       // float newQ = AU2 + EDF2 - EDB2 + FDF2 - FDB2;
       // float AU3 = 0.5 * (predictedGridRTData.b + newPC);
       // float EDF3 = (G(rxplus) + G(rxminus)) * epf;
       // float EDB3 = (G(rxplus) + G(rxminus) * epb);
       // float FDF3 = (G(ryplus) + G(ryminus)) * fpf;
       // float FDB3 = (G(ryplus) + G(ryminus)) * fpb;
       // float newP = AU3 + EDF3 - EDB3 + FDF3 - FDB3;
       // correctedGridRTData.r = newH;
       // correctedGridRTData.g = newQ;
       // correctedGridRTData.b = newP;

	// Pass the output
	output.Target0 = predictedGridRTData;
	output.Target1 = correctedGridRTData; // Output for RenderTextureUVA or RenderTextureUVB
    return output;
}






