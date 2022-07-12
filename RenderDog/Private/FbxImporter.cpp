///////////////////////////////////////////
//RenderDog <��,��>
//FileName: FbxImporter.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "FbxImporter.h"
#include "Matrix.h"

namespace RenderDog
{
	RDFbxImporter	g_RDFbxImporter;
	RDFbxImporter*	g_pRDFbxImporter = &g_RDFbxImporter;

	RDFbxImporter::RDFbxImporter() :
		m_pManager(nullptr),
		m_pScene(nullptr),
		m_pImporter(nullptr),
		m_RawData(0)
	{}

	RDFbxImporter::~RDFbxImporter()
	{

	}

	bool RDFbxImporter::Init()
	{
		m_pManager = FbxManager::Create();
		if (!m_pManager)
		{
			FBXSDK_printf("Error: Unable to create FBX Manager!\n");
			return false;
		}
		else
		{
			FBXSDK_printf("Autodesk FBX SDK version %s\n", m_pManager->GetVersion());
		}

		FbxIOSettings* ioSettings = FbxIOSettings::Create(m_pManager, IOSROOT);
		m_pManager->SetIOSettings(ioSettings);

		
		FbxString lPath = FbxGetApplicationDirectory();
		m_pManager->LoadPluginsDirectory(lPath.Buffer());

		
		m_pScene = FbxScene::Create(m_pManager, "RenderDog FBX Scene");
		if (!m_pScene)
		{
			FBXSDK_printf("Error: Unable to create FBX scene!\n");
			return false;
		}

		m_pImporter = FbxImporter::Create(m_pManager, "");
		if (!m_pImporter)
		{
			FBXSDK_printf("Error: Unable to create FBX importer!\n");
			return false;
		}

		return true;
	}

	void RDFbxImporter::Release()
	{
		if (m_pManager)
		{
			m_pManager->Destroy();
			m_pManager = nullptr;
		}
	}

	bool RDFbxImporter::LoadFbxFile(const std::string& filePath, bool bIsSkinModel, bool bFlipUV)
	{
		if (!m_pImporter->Initialize(filePath.c_str(), -1, m_pManager->GetIOSettings()))
		{
			return false;
		}

		int fileVersionMajor = -1;
		int fileVersionMinor = -1;
		int fileVersionRevision = -1;
		m_pImporter->GetFileVersion(fileVersionMajor, fileVersionMinor, fileVersionRevision);
		//TODO: print file version

		if (!m_pImporter->Import(m_pScene))
		{
			m_pImporter->Destroy();
			return false;
		}

		//NOTE!!! Why not work??? Have to use a Matrix to transform vertices to Y up Left handed Axis;
		/*FbxAxisSystem SceneAxisSystem = m_pScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem RenderDogAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
		if (SceneAxisSystem != RenderDogAxisSystem)
		{
			RenderDogAxisSystem.ConvertScene(m_pScene);
		}*/

		m_RawData.clear();

		if (!ProcessMeshNode(m_pScene->GetRootNode(), bFlipUV))
		{
			return false;
		}

		return true;
	}

	bool RDFbxImporter::ProcessMeshNode(FbxNode* pNode, bool bFlipUV)
	{
		if (!GetMeshData(pNode, bFlipUV))
		{
			return false;
		}

		for (int i = 0; i < pNode->GetChildCount(); ++i)
		{
			FbxNode* pChildNode = pNode->GetChild(i);
			if (!pChildNode)
			{
				return false;
			}
			
			if (!ProcessMeshNode(pChildNode, bFlipUV))
			{
				return false;
			}
		}

		return true;
	}

	bool RDFbxImporter::GetMeshData(FbxNode* pNode, bool bFlipUV)
	{
		RawMeshData meshData;

		FbxMesh* pMesh = pNode->GetMesh();
		if (!pMesh)
		{
			//TODO!!! Log some imformation
			return true;
		}

		//Triangulate
		FbxGeometryConverter triangleConverter(m_pManager);
		FbxNodeAttribute* pNodeAttribute = triangleConverter.Triangulate(pMesh, true);
		if (pNodeAttribute && pNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			pMesh = static_cast<FbxMesh*>(pNodeAttribute);
		}
		else
		{
			pMesh = nullptr;
			return false;
		}

		int triangleNum = pMesh->GetPolygonCount();
		meshData.postions.reserve(triangleNum * 3);

		std::vector<uint32_t> triangleMaterialIndices;
		triangleMaterialIndices.reserve(triangleNum);
		GetTriangleMaterialIndices(pMesh, triangleNum, triangleMaterialIndices);

		std::vector<uint32_t> smoothGroup;
		smoothGroup.reserve(triangleNum);
		GetTriangleSmoothIndices(pMesh, triangleNum, smoothGroup);

		int texcoordNum = pMesh->GetElementUVCount();
		meshData.texcoords.reserve(texcoordNum);
		
		int materialNum = pNode->GetMaterialCount();
		for (int matIndex = 0; matIndex == 0 || matIndex < materialNum; ++matIndex)
		{
			meshData.postions.clear();
			meshData.texcoords.clear();
			meshData.smoothGroup.clear();
			//�������������Σ��ҵ�MaterialIndex��ͬ�������η��뵽һ��Mesh��
			for (int i = 0; i < triangleNum; ++i)
			{
				if (triangleMaterialIndices[i] == matIndex)
				{
					//Positions
					Vector3 pos[3];
					Vector2 tex[3];
					//NOTE!!! ������Ҫ�任����Ļ��Ʒ���Ϊ˳ʱ��
					for (int j = 2; j >= 0; --j)
					{
						int ctrlPointIndex = pMesh->GetPolygonVertex(i, j);
						ReadPositions(pMesh, ctrlPointIndex, pos[j]);
						meshData.postions.push_back(pos[j]);

						int uvIndex = pMesh->GetTextureUVIndex(i, j);
						int uvLayer = 0;
						ReadTexcoord(pMesh, ctrlPointIndex, uvIndex, uvLayer, tex[j]);
						if (bFlipUV)
						{
							tex[j].y = 1.0f - tex[j].y;
						}
						meshData.texcoords.push_back(tex[j]);

						meshData.smoothGroup.push_back(smoothGroup[i]);
					}
				}
			}

			std::string meshName = pMesh->GetName();
			meshData.name = meshName + "_" + std::to_string(matIndex);
			m_RawData.push_back(meshData);
		}

		return true;
	}

	void RDFbxImporter::GetTriangleMaterialIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices)
	{
		FbxLayerElementMaterial* pMaterial = pMesh->GetElementMaterial();
		if (!pMaterial)
		{
			return;
		}

		for (int triangleIndex = 0; triangleIndex < triNum; ++triangleIndex)
		{
			uint32_t materialIndex = pMaterial->GetIndexArray().GetAt(triangleIndex);
			outputIndices.push_back(materialIndex);
		}
	}

	void RDFbxImporter::GetTriangleSmoothIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices)
	{
		FbxLayerElementSmoothing* pSmoothing = pMesh->GetElementSmoothing();
		if (pSmoothing)
		{
			for (int i = 0; i < triNum; ++i)
			{
				int SmoothGroupIndex = (pSmoothing->GetReferenceMode() == FbxLayerElement::eDirect) ? i : pSmoothing->GetIndexArray().GetAt(i);
				int smoothIndex = pSmoothing->GetDirectArray().GetAt(SmoothGroupIndex);

				outputIndices.push_back(smoothIndex);
			}
		}
	}

	void RDFbxImporter::ReadPositions(FbxMesh* pMesh, int ctrlPointIndex, Vector3& outputPos)
	{
		FbxVector4* pCtrlPoint = pMesh->GetControlPoints();

		outputPos.x = static_cast<float>(pCtrlPoint[ctrlPointIndex][0]);
		outputPos.y = static_cast<float>(pCtrlPoint[ctrlPointIndex][1]);
		outputPos.z = static_cast<float>(pCtrlPoint[ctrlPointIndex][2]);

		//NOTE!!! ����������ʹ��FbxSDK�е�Axis�任֮��Ӧ�ð�����ȥ��
		Matrix4x4 transAxisMatrix(1.0f, 0.0f, 0.0f, 0.0f,
								  0.0f, 0.0f, 1.0f, 0.0f,
								  0.0f, 1.0f, 0.0f, 0.0f,
								  0.0f, 0.0f, 0.0f, 1.0f);

		Vector4 newPos(outputPos, 1.0f);
		newPos = newPos * transAxisMatrix;
		outputPos = Vector3(newPos.x, newPos.y, newPos.z);
	}

	void RDFbxImporter::ReadTexcoord(FbxMesh* pMesh, int ctrlPointIndex, int textureUVIndex, int uvLayer, Vector2& OutputUV)
	{
		FbxGeometryElementUV* pVertexUV = pMesh->GetElementUV(uvLayer);

		if (pVertexUV->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			if (pVertexUV->GetReferenceMode() == FbxGeometryElement::eDirect)
			{
				OutputUV.x = static_cast<float>(pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[0]);
				OutputUV.y = static_cast<float>(pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[1]);
			}
			else if (pVertexUV->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
			{
				int id = pVertexUV->GetIndexArray().GetAt(ctrlPointIndex);
				OutputUV.x = static_cast<float>(pVertexUV->GetDirectArray().GetAt(id)[0]);
				OutputUV.y = static_cast<float>(pVertexUV->GetDirectArray().GetAt(id)[1]);
			}
		}
		else if (pVertexUV->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			OutputUV.x = static_cast<float>(pVertexUV->GetDirectArray().GetAt(textureUVIndex)[0]);
			OutputUV.y = static_cast<float>(pVertexUV->GetDirectArray().GetAt(textureUVIndex)[1]);
		}
	}

}// namespace RenderDog