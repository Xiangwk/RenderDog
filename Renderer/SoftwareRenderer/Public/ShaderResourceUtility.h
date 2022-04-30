///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShaderResourceUtility.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	class ISRDevice;
	class ISRShaderResourceView;

	bool CreateShaderResourceViewFromFile(ISRDevice* pDevice, const char* strSrcFile, ISRShaderResourceView** ppShaderResourceView);
}