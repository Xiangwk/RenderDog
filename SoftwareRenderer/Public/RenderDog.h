#pragma once

#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"

namespace RenderDog
{
	bool CreateDeviceAndSwapChain(IDevice** pDevice, IDeviceContext** pDeviceContext, SwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc);
}