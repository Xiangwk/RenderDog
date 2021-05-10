#pragma once

#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"

namespace RenderDog
{
	bool CreateDeviceAndSwapChain(Device** pDevice, DeviceContext** pDeviceContext, SwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc);
}