///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShaderResourceUtility.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	class IDevice;
	class IShaderResourceView;

	bool CreateShaderResourceViewFromFile(IDevice* pDevice, const char* strSrcFile, IShaderResourceView** ppShaderResourceView);
}