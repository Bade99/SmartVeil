Texture2D tx : register(t0);
Texture2D previousTx : register(t1);
SamplerState samLinear : register(s0);
cbuffer buf : register(b0)
{
	float Threshold;
	float Opacity;
	float unused1;
	float unused2;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

//y1 is the current pixelWeight, y2 is the Threshold, desiredX is Opacity
//all must be in [0,1] space
//it will return the y value of the function at the desired x value
/*
float pointInLine(float y1, float y2, float desiredX) {
	//y = mx + b
	//b = y - mx
	float m = (y2 - y1) / (1 - 0);//the threshold is at x = 1 and your original pixelWeight is at x = 0
	float b = y1 - m * 0;//you already knew (y1,x1=0) intersects the y axis so this part is obvious
	float yResult = m * desiredX + b;
	//resumido es = (y2-y1)*desiredX + y1;
	return yResult;
	//TODO(fran): now my problem is the stupid pre-multiplied alpha, this is not the end value, just the alpha, then it does rgb*(1-a)
}
*/

/*Understanding pre-multiplied alpha
valueIWantToDisplay = pixelWeight + (Threshold-pixelWeight)*Opacity
valueIHave = pixelWeight
valueIHave*(1-alpha) = valueIWantToDisplay
->
pixelWeight*(1 - alpha) = pixelWeight + (Threshold-pixelWeight)*Opacity
1 - alpha = (pixelWeight + (Threshold-pixelWeight)*Opacity)/ pixelWeight
alpha = 1 - ((pixelWeight + (Threshold-pixelWeight)*Opacity) / pixelWeight)
*/

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSV(PS_INPUT input) : SV_Target
{
	float4 color = tx.Sample(samLinear, input.Tex);
	float4 previousColor = previousTx.Sample(samLinear, input.Tex);

	//TODO(fran): if we could add things together first then we would do less floating point operations

	//Add Previous Alpha Value
	if (previousColor.a == 1) {
		color.r = 1;//TODO(fran): im not sure this is right any more
		color.g = 1;
		color.b = 1;
	}
	else {
		//TODO(fran): si asignamos color al veil (osea no negro) lo agregaríamos acá, ver smartcolorveil.cpp
		color.r /= (1 - previousColor.a);
		color.g /= (1 - previousColor.a);
		color.b /= (1 - previousColor.a);
	}

	//Compute New Alpha Value
	float pixelWeight = (color.r + color.g + color.b)/3.0;
	if (pixelWeight > Threshold) {
		//color.a = .5;
		//color.a = (pixelWeight - Threshold)*Opacity;//This is wrong, at a big enough alpha a brighter pixel will get darker than one less bright that is also getting darkened
		//also it's wrong because it does not produce a line that goes from pixelWeight to Threshold (remember geogebra)
													
		//color.a = 1 - ((pixelWeight + (Threshold - pixelWeight)*Opacity) / pixelWeight);//TODO(fran): can we reduce this function?
		//->reducing this function:
		color.a = Opacity - Threshold * Opacity / pixelWeight;

		//If I understood correctly then windows will do rgb = dest.rgb * (1-myAlpha) ; dest.rgb==the pixel it already has
	}
	else {
		color.a = 0;
	}
	color.r = 0;//TODO(fran): maybe I can set this only the first time through and in later passes not execute this code until the textures need to be recreated
	color.g = 0;
	color.b = 0;
	return color;
}

//TODO IMPORTANT: I feel like the rapid flipping (flickering) that we are seeing is because of the fact that we take each pixel as a single value when actually it's r,g,b with different values each
// maybe we are destroying some values, for example if we are supposed to subtract 30 from each component but one only has 10, 
// then probably the value is not what we expect and we are breaking stuff, not sure though
