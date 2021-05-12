//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    FlagDecal.fx
//
//  AUTHOR:  Bart Muzzin
//			 Tom Whittaker
//
//  PURPOSE: Draw the flags with primary color and modulated decal texture
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------  
texture BaseTexture <string NTM = "base";>;
texture DecalTexture < string NTM = "detail";>;
texture BaseColor <string NTM = "decal"; int NTMIndex = 0;>;
texture DecalColor <string NTM = "decal"; int NTMIndex = 1;>;

//------------------------------------------------------------------------------------------------
//                          SAMPLERS
//------------------------------------------------------------------------------------------------  
sampler BaseSampler = sampler_state
{
    Texture = (BaseTexture);
    AddressU  = WRAP;
    AddressV  = WRAP;
    MagFilter = LINEAR;
    MinFilter = LINEAR;
    MipFilter = LINEAR;
};

sampler BaseColorSampler = sampler_state
{
    Texture = (BaseColor);
    AddressU  = WRAP;
    AddressV  = WRAP;
    MagFilter = LINEAR;
    MinFilter = LINEAR;
    MipFilter = LINEAR;
};

sampler DecalSampler = sampler_state
{ 
	Texture = (DecalTexture);
	AddressU = Clamp;
	AddressV = Clamp;
	MagFilter = Linear;
	MipFilter = Linear;
	MinFilter = Linear; 
};

sampler DecalColorSampler = sampler_state
{ 
	Texture = (DecalColor);
	AddressU = Clamp;
	AddressV = Clamp;
	MagFilter = Linear;
	MipFilter = Linear;
	MinFilter = Linear; 
};


//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------
technique TFlagShader_2TPP< string shadername = "TFlagShader_2TPP"; int implementation=0;>
{
    pass P0
    {
        // Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = TRUE;
        ZFunc          = LESSEQUAL;
        
        // Enable alpha blending & testing
        AlphaBlendEnable = TRUE;
        AlphaTestEnable	 = TRUE;
        AlphaRef		 = 0;
        AlphaFunc		 = GREATER;
        SrcBlend         = SRCALPHA;
        DestBlend        = INVSRCALPHA;
        
   		// Allow the use of multiple texcoord indices
        TexCoordIndex[0] = 0;
        TexCoordIndex[1] = 0;
        TexCoordIndex[2] = 0;
        TexCoordIndex[3] = 0;
        TextureTransformFlags[0] = 0;
        TextureTransformFlags[1] = 0;
        TextureTransformFlags[2] = 0;
        TextureTransformFlags[3] = 0;

        // Set the smaplers
		Sampler[0] = <BaseColorSampler>;
        Sampler[1] = <DecalSampler>;
        Sampler[2] = <DecalColorSampler>;
   		Sampler[3] = <BaseSampler>;

        // Set up texture stage states
        ColorArg1[0] = Texture;
        ColorOp[0] = SelectArg1;
        AlphaArg1[0] = Texture;
        AlphaOp[0] = SelectArg1;

		ColorArg2[1] = Current;
		ColorArg1[1] = Texture;
		ColorOp[1] = Modulate;
		AlphaArg1[1] = Texture;
		AlphaOp[1] = SelectArg1;

		ColorArg1[2] = Texture;
		ColorArg2[2] = Current;
		ColorOp[2] = BlendCurrentAlpha;
		AlphaArg1[2] = Texture;
		AlphaArg2[2] = Current;
		AlphaOp[2] = Modulate;

		ColorArg2[3] = Current;
		ColorArg1[3] = Texture;
		ColorOp[3] = Modulate;
		AlphaArg1[3] = Texture;
		AlphaOp[3] = SelectArg1;
		
		// shaders
        VertexShader     = NULL;
        PixelShader      = NULL;
   	}
}


//This is actually the 2 Textures Per Pass shader version. However the Nifs already have the 
//shaders attached as TFlagShader_2TPP.
technique TFlagShader_2TPP_2TPP
< string shadername = "TFlagShader_2TPP"; int implementation=1;>
{
    pass P0
    {
        // Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = TRUE;
        ZFunc          = LESSEQUAL;
        
        // Enable alpha blending & testing
        AlphaBlendEnable = TRUE;
        AlphaTestEnable	 = TRUE;
        AlphaRef		 = 0;
        AlphaFunc		 = GREATER;
        SrcBlend         = SRCALPHA;
        DestBlend        = INVSRCALPHA;
        
   		// Allow the use of multiple texcoord indices
        TexCoordIndex[0] = 0;
        TexCoordIndex[1] = 0;
        TextureTransformFlags[0] = 0;
        TextureTransformFlags[1] = 0;
    
        // Set the smaplers
		Sampler[0] = <BaseColorSampler>;
        Sampler[1] = <BaseSampler>;

        // Set up texture stage states
        ColorArg1[0] = Texture;
        ColorOp[0] = SelectArg1;
        AlphaArg1[0] = Texture;
        AlphaOp[0] = SelectArg1;

		ColorArg2[1] = Current;
		ColorArg1[1] = Texture;
		ColorOp[1] = Modulate;
		AlphaArg1[1] = Texture;
		AlphaOp[1] = SelectArg1;
		
        // shaders
        VertexShader     = NULL;
        PixelShader      = NULL;

	}
	
	// Second Pass to combine Seconday color
	pass P1
	{
		//TomWtodo: if you really want to make the flags a 2 pass effect on cards that only have 2 textures per pass	
		Sampler[0] = <DecalColorSampler>;
	    Sampler[1] = <DecalSampler>;
	    
	    // Set up texture stage states
        ColorArg1[0] = Texture;
        ColorOp[0] = SelectArg1;
        AlphaArg1[0] = Texture;
        AlphaOp[0] = SelectArg1;

		ColorArg2[1] = Current;
		ColorArg1[1] = Texture;
		ColorOp[1] = Modulate;
		AlphaArg1[1] = Texture;
		AlphaOp[1] = SelectArg1;
	}
	
	
}
