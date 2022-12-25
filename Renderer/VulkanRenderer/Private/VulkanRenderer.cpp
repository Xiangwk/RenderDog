////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: VulkanRenderer.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Renderer.h"
#include "VulkanRenderer.h"

#include <vector>

namespace RenderDog
{
	class VulkanRenderer : public IRenderer
	{
	public:
		VulkanRenderer();
		virtual ~VulkanRenderer();

		virtual bool				Init(const RendererInitDesc& desc) override;
		virtual void				Release() override;

		virtual void				Update(IScene* pScene) override;
		virtual void				Render(IScene* pScene) override;

		virtual bool				OnResize(uint32_t width, uint32_t height);

	private:
		bool						CheckExtensionSupport();
		bool						CheckValidationLayerSupport();

		bool						CreateInternalShaders();
		void						ReleaseInternalShaders();

		bool						CreateInternalTextures();
		void						ReleaseInternalTextures();

		bool						CreateShadowResources(uint32_t width, uint32_t height);
		void						ReleaseShadowResources();

		//Shadow
		void						AddPrisToShadowView(IScene* pScene);
		void						CalcShadowMatrix(IScene* pScene);
		void						ShadowDepthPass();

		void						ClearBackRenderTarget(float* clearColor);
		void						AddPrisAndLightsToSceneView(IScene* pScene);
		void						RenderPrimitives(IScene* pScene);

		void						RenderSky(IScene* pScene);

	private:
		VkInstance					m_vulkanInstance;

		std::vector<const char*>	m_EnabledLayerNames;
		std::vector<const char*>	m_EnabledExtNames;
	};

	VulkanRenderer	g_VulkanRenderer;
	IRenderer* g_pIRenderer = &g_VulkanRenderer;

	//------------------------------------------------------------------------
	//   Public Function
	//------------------------------------------------------------------------
	VulkanRenderer::VulkanRenderer():
		m_vulkanInstance(),
		m_EnabledLayerNames(0),
		m_EnabledExtNames(0)
	{
		m_EnabledLayerNames.push_back("VK_LAYER_KHRONOS_validationLayers");
	}

	VulkanRenderer::~VulkanRenderer()
	{}

	bool VulkanRenderer::Init(const RendererInitDesc& desc)
	{
		bool bEnableValidationLayers = false;

		if (!CheckExtensionSupport())
		{
			return false;
		}

		if (!CheckValidationLayerSupport())
		{
			return false;
		}

		VkInstanceCreateInfo vulkanCreateInfo = {};
		vulkanCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vulkanCreateInfo.pNext = nullptr;
		vulkanCreateInfo.pApplicationInfo = nullptr;
		vulkanCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_EnabledExtNames.size());
		vulkanCreateInfo.ppEnabledExtensionNames = m_EnabledExtNames.data();
		if (bEnableValidationLayers)
		{
			vulkanCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_EnabledLayerNames.size());
			vulkanCreateInfo.ppEnabledLayerNames = m_EnabledLayerNames.data();
		}
		else
		{
			vulkanCreateInfo.enabledLayerCount = 0;
			vulkanCreateInfo.ppEnabledLayerNames = nullptr;
		}

		if (vkCreateInstance(&vulkanCreateInfo, nullptr, &m_vulkanInstance) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanRenderer::Release()
	{
		vkDestroyInstance(m_vulkanInstance, nullptr);
	}


	//------------------------------------------------------------------------
	//   Private Function
	//------------------------------------------------------------------------
	bool VulkanRenderer::CheckExtensionSupport()
	{
		VkResult result;

		uint32_t instanceExtCnt = 0;
		result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCnt, nullptr);
		if (result != VK_SUCCESS)
		{
			return false;
		}

		if (instanceExtCnt > 0)
		{
			std::vector<VkExtensionProperties> extProps(instanceExtCnt);
			result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCnt, extProps.data());
			if (result != VK_SUCCESS)
			{
				return false;
			}

			for (uint32_t i = 0; i < instanceExtCnt; ++i)
			{
				if (strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extProps[i].extensionName) == 0)
				{
					m_EnabledExtNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
				}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
				if (strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, extProps[i].extensionName) == 0)
				{
					m_EnabledExtNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
				}
#endif
				if (strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, extProps[i].extensionName) == 0)
				{
					m_EnabledExtNames.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
				}
				if (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, extProps[i].extensionName) == 0)
				{
					m_EnabledExtNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}
			}
		}

		return true;
	}

	bool VulkanRenderer::CheckValidationLayerSupport()
	{
		VkResult result;
		uint32_t layerCount = 0;
		
		result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		if (result != VK_SUCCESS)
		{
			return false;
		}

		if (layerCount > 0)
		{
			std::vector<VkLayerProperties> availableLayers(layerCount);
			result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
			if (result != VK_SUCCESS)
			{
				return false;
			}

			bool bFoundLayer = false;
			for (uint32_t i = 0; i < m_EnabledLayerNames.size(); ++i)
			{
				for (uint32_t j = 0; j < availableLayers.size(); ++j)
				{
					if (strcmp(m_EnabledLayerNames[i], availableLayers[j].layerName) == 0)
					{
						bFoundLayer = true;
						break;
					}
				}
			}

			if (!bFoundLayer)
			{
				return false;
			}
		}

		return true;
	}

} // namespace RenderDog