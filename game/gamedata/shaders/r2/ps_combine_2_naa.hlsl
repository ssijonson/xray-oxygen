#include "common.h"
#include "mblur.h"
#include "dof.h"

struct 	v2p
{
  float4 tc0: 		TEXCOORD0;	// Center
  float4 tc1: 		TEXCOORD1;	// LT 		 
  float4 tc2: 		TEXCOORD2;	// RB
  float4 tc3: 		TEXCOORD3;	// RT 
  float4 tc4: 		TEXCOORD4;	// LB
  float4 tc5:		TEXCOORD5;	// Left	/ Right	
  float4 tc6:		TEXCOORD6;	// Top  / Bottom 
};

uniform sampler2D 	s_distort;
uniform half4 		e_barrier;	// x=norm(.8f), y=depth(.1f), z=clr
uniform half4 		e_weights;	// x=norm, y=depth, z=clr
uniform half4 		e_kernel;	// x=norm, y=depth, z=clr
#define	EPSDEPTH	0.001

half4 main (v2p I) : COLOR
{
#ifdef 	USE_DISTORT
  	half 	depth 	= tex2D(s_position, I.tc0).z;
	half4 	distort	= tex2D(s_distort, I.tc0);
	half2	offset	= (distort.xy - (127.0h/255.0h))*def_distort;  // fix newtral offset
	float2	center	= I.tc0 + offset;
	half 	depth_x	= tex2D(s_position, center).z;
	if ((depth_x+EPSDEPTH)<depth) center = I.tc0;	// discard new sample
#else
	float2	center 	= I.tc0;
#endif
	half3	img		= dof(center);
			img 	= mblur(center, tex2D(s_position, I.tc0),img.rgb);
	half4 	bloom	= tex2D(s_bloom, center);
	// end
#ifdef 	USE_DISTORT
 	half3	blurred	= bloom*def_hdr;
	img		= lerp(img, blurred, distort.z);
#endif

 	return combine_bloom(img,bloom);
}
