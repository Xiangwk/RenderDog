#pragma once

#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"

#define DEBUG_RASTERIZATION 0

namespace RenderDog
{
	bool CreateDeviceAndSwapChain(Device** pDevice, DeviceContext** pDeviceContext, SwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc);
}