#include "stdafx.h"

void CRenderTarget::PhaseVignette()
{
	float _w = float(Device.dwWidth);
	float _h = float(Device.dwHeight);

	// Pass 0
#if defined(USE_DX10) || defined(USE_DX11)
	ref_rt outRT = RImplementation.o.dx10_msaa ? rt_Generic : rt_Color;

	RenderScreenQuad(_w, _h, rt_Generic_2, s_vignette->E[0]);
	HW.pContext->CopyResource(outRT->pTexture->surface_get(), rt_Generic_2->pTexture->surface_get());
#else
	RenderScreenQuad(_w, _h, rt_Color, s_vignette->E[0]);
#endif
}
