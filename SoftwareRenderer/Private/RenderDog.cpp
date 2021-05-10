#include "RenderDog.h"

namespace RenderDog
{
	bool CreateDeviceAndSwapChain(Device** pDevice, DeviceContext** pDeviceContext, SwapChain** pSwapChain, const SwapChainDesc* pSwapChainDesc)
	{
		*pDevice = new Device;
		if (!pDevice)
		{
			return false;
		}

		*pDeviceContext = new DeviceContext(pSwapChainDesc->nWidth, pSwapChainDesc->nHeight);
		if (!pDeviceContext)
		{
			return false;
		}

		*pSwapChain = new SwapChain(pSwapChainDesc);
		if (!pSwapChain)
		{
			return false;
		}

		return true;
	}
}