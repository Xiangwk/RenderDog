///////////////////////////////////////////
//RenderDog <·,·>
//FileName: FbxImporter.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "FbxImporter.h"
#include "Matrix.h"

namespace RenderDog
{
	RDFbxImporter	g_RDFbxImporter;
	RDFbxImporter*	g_pRDFbxImporter = &g_RDFbxImporter;

	RDFbxImporter::RawBoneData* RDFbxImporter::RawSkeletonData::GetBone(const std::string& name)
	{
		auto bone = boneMap.find(name);
		if (bone != boneMap.end())
		{
			return bone->second;
		}
		else
		{
			return nullptr;
		}
	}

	RDFbxImporter::RDFbxImporter() :
		m_pManager(nullptr),
		m_pScene(nullptr),
		m_pImporter(nullptr),
		m_RawData(0),
		m_bNeedBoneSkin(false)
	{}

	RDFbxImporter::~RDFbxImporter()
	{
		m_RawData.clear();

		if (m_pSkeleton)
		{
			for (int i = 0; i < m_pSkeleton->bones.size(); ++i)
			{
				RawBoneData* pBone = m_pSkeleton->bones[i];
				if (pBone)
				{
					delete pBone;
					pBone = nullptr;
				}
			}

			delete m_pSkeleton;
			m_pSkeleton = nullptr;
		}
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

	bool RDFbxImporter::LoadFbxFile(const std::string& filePath, FBX_LOAD_TYPE loadType, const FbxLoadParam& loadParam/* = FbxLoadParam()*/)
	{
		m_bNeedBoneSkin = (loadType == FBX_LOAD_TYPE::SKIN_MODEL);
		
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

		//NOTE!!! 这里的代码不知道为何不起作用，暂时在读取顶点位置数据的时候，手动转换为Y轴朝上的左手系（详见StaticModel::LoadFromRawMeshData函数）
		/*FbxAxisSystem SceneAxisSystem = m_pScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem RenderDogAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
		if (SceneAxisSystem != RenderDogAxisSystem)
		{
			RenderDogAxisSystem.ConvertScene(m_pScene);
		}*/

		if (loadType == FBX_LOAD_TYPE::ANIMATION)
		{
			m_BoneAnimations.boneAnimations.clear();

			auto strIter0 = filePath.rfind("/") + 1;
			auto strIter1 = filePath.rfind(".");
			std::string animName = filePath.substr(strIter0, strIter1 - strIter0);

			m_BoneAnimations.name = animName;
			ProcessAnimation(m_pScene->GetRootNode());
		}
		else
		{
			if (m_bNeedBoneSkin)
			{
				if (m_pSkeleton)
				{
					for (int i = 0; i < m_pSkeleton->bones.size(); ++i)
					{
						RawBoneData* pBone = m_pSkeleton->bones[i];
						if (pBone)
						{
							delete pBone;
							pBone = nullptr;
						}
					}

					m_pSkeleton->boneMap.clear();
				}
				else
				{
					m_pSkeleton = new RawSkeletonData;
				}

				ProcessSkeletonNode(m_pScene->GetRootNode(), nullptr);

				//对于SkinModel来说，坐标系转换应该在根节点上做
				Matrix4x4 transAxisMatrix(1.0f, 0.0f, 0.0f, 0.0f,
										  0.0f, 0.0f, 1.0f, 0.0f,
										  0.0f, 1.0f, 0.0f, 0.0f,
										  0.0f, 0.0f, 0.0f, 1.0f);
				m_pSkeleton->LocalMatrix = transAxisMatrix;
			}

			m_RawData.clear();
			if (!ProcessMeshNode(m_pScene->GetRootNode(), loadParam.bIsFlipTexcoordV))
			{
				return false;
			}
		}

		return true;
	}

	bool RDFbxImporter::ProcessMeshNode(FbxNode* pNode, bool bIsFlipTexcoordV)
	{
		if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			if (!GetMeshData(pNode, bIsFlipTexcoordV))
			{
				return false;
			}
		}

		int childCnt = pNode->GetChildCount();
		for (int i = 0; i < childCnt; ++i)
		{
			FbxNode* pChildNode = pNode->GetChild(i);
			if (!ProcessMeshNode(pChildNode, bIsFlipTexcoordV))
			{
				return false;
			}
		}

		return true;
	}

	bool RDFbxImporter::GetMeshData(FbxNode* pNode, bool bIsFlipTexcoordV)
	{
		RawMeshData meshData;

		FbxMesh* pMesh = pNode->GetMesh();

		FbxAMatrix pivotTransform;
		FbxVector4 Translation = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		FbxVector4 Rotation = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		FbxVector4 Scaling = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		pivotTransform.SetT(Translation);
		pivotTransform.SetR(Rotation);
		pivotTransform.SetS(Scaling);

		FbxSkin* pFbxSkin = nullptr;
		std::unordered_map<int, VertexBones> vertBonesMap;
		if (m_bNeedBoneSkin)
		{
			int deformerCnt = pMesh->GetDeformerCount();
			for (int i = 0; i < deformerCnt; ++i)
			{
				FbxDeformer* pFbxDeformer = pMesh->GetDeformer(i);

				if (!pFbxDeformer)
				{
					continue;
				}

				if (pFbxDeformer->GetDeformerType() == FbxDeformer::eSkin)
				{
					pFbxSkin = (FbxSkin*)(pFbxDeformer);
					break;
				}
			}
			
			GetOffsetMatrix(pFbxSkin);

			GetBoneSkins(pFbxSkin, vertBonesMap);
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
			meshData.color.clear();
			meshData.texcoords.clear();
			meshData.smoothGroup.clear();
			meshData.boneIndices.clear();
			meshData.boneWeighs.clear();
			//遍历所有三角形，找到MaterialIndex相同的三角形放入到一个Mesh中
			for (int i = 0; i < triangleNum; ++i)
			{
				if (triangleMaterialIndices[i] == matIndex)
				{
					//Positions
					Vector3 pos[3];
					Vector4 colors[3] = { Vector4(1.0f), Vector4(1.0f), Vector4(1.0f) };
					Vector2 tex[3];
					//NOTE!!! 这里需要变换顶点的环绕方向为顺时针
					for (int j = 2; j >= 0; --j)
					{
						int ctrlPointIndex = pMesh->GetPolygonVertex(i, j);
						ReadPositions(pMesh, ctrlPointIndex, pivotTransform, pos[j]);
						meshData.postions.push_back(pos[j]);

						//NOTE!!! 暂时不知道为什么vertexColorLayer是0
						int vertexColorLayer = 0;
						int colorIndex = i * 3 + j;
						ReadColors(pMesh, ctrlPointIndex, colorIndex, vertexColorLayer, colors[j]);
						meshData.color.push_back(colors[j]);

						int uvIndex = pMesh->GetTextureUVIndex(i, j);
						//NOTE!!! 暂时不知道为什么uvLayer是0
						int uvLayer = 0;
						ReadTexcoord(pMesh, ctrlPointIndex, uvIndex, uvLayer, tex[j]);
						if (bIsFlipTexcoordV)
						{
							tex[j].y = 1.0f - tex[j].y;
						}
						meshData.texcoords.push_back(tex[j]);

						meshData.smoothGroup.push_back(smoothGroup[i]);

						if (pFbxSkin)
						{
							//找到影响当前顶点的所有骨骼及其权重
							std::vector<std::string> tempBones;
							std::vector<float> tempWeights;
							ReadBoneSkin(ctrlPointIndex, vertBonesMap, tempBones, tempWeights);

							if (tempBones.empty())
							{
								return false;
							}

							//如果影响当前顶点的骨骼数量超过4个，则按从小到大的顺序，一次剔除到仅剩4个
							while (tempBones.size() > 4)
							{
								float minWeight = tempWeights[0];
								size_t minWeightIndex = 0;
								for (size_t boneIndex = 0; boneIndex < tempBones.size(); ++boneIndex)
								{
									if (tempWeights[boneIndex] < minWeight)
									{
										minWeight = tempWeights[boneIndex];
										minWeightIndex = boneIndex;
									}
								}

								tempBones.erase(tempBones.begin() + minWeightIndex);
								tempWeights.erase(tempWeights.begin() + minWeightIndex);
							}

							//重新计算权重
							float totalWeight = 0.0f;
							for (size_t boneIndex = 0; boneIndex < tempBones.size(); ++boneIndex)
							{
								totalWeight += tempWeights[boneIndex];
							}

							Vector4 boneWeight(0.0f);
							for (size_t boneIndex = 0; boneIndex < tempBones.size(); ++boneIndex)
							{
								tempWeights[boneIndex] = tempWeights[boneIndex] / totalWeight;
								if (boneIndex == 0)
								{
									boneWeight.x = tempWeights[boneIndex];
								}
								else if (boneIndex == 1)
								{
									boneWeight.y = tempWeights[boneIndex];
								}
								else if (boneIndex == 2)
								{
									boneWeight.z = tempWeights[boneIndex];
								}
								else
								{
									boneWeight.w = tempWeights[boneIndex];
								}
							}
							meshData.boneWeighs.push_back(boneWeight);

							//查找骨骼索引
							float boneIndices[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
							Vector4 vBoneIndice(0.0f);
							for (size_t boneIndex = 0; boneIndex < tempBones.size(); ++boneIndex)
							{
								RawBoneData* pBoneData = m_pSkeleton->GetBone(tempBones[boneIndex]);
								if (!pBoneData)
								{
									return false;
								}

								boneIndices[boneIndex] = (float)pBoneData->index;

								if (boneIndex == 0)
								{
									vBoneIndice.x = boneIndices[boneIndex];
								}
								else if (boneIndex == 1)
								{
									vBoneIndice.y = boneIndices[boneIndex];
								}
								else if (boneIndex == 2)
								{
									vBoneIndice.z = boneIndices[boneIndex];
								}
								else if (boneIndex == 3)
								{
									vBoneIndice.w = boneIndices[boneIndex];
								}
							}
							meshData.boneIndices.push_back(vBoneIndice);
						}
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

	void RDFbxImporter::ReadPositions(FbxMesh* pMesh, int vertexIndex, const FbxAMatrix& pivotTransform, Vector3& outputPos)
	{
		FbxVector4* pCtrlPoint = pMesh->GetControlPoints();

		FbxVector4 position = pivotTransform.MultT(pCtrlPoint[vertexIndex]);
		outputPos.x = static_cast<float>(position[0]);
		outputPos.y = static_cast<float>(position[1]);
		outputPos.z = static_cast<float>(position[2]);
	}

	void RDFbxImporter::ReadColors(FbxMesh* pMesh, int vertexIndex, int colorIndex, int layer, Vector4& outputColor)
	{
		FbxLayer* pBaseLayer = pMesh->GetLayer(layer);
		if (!pBaseLayer)
		{
			return;
		}

		FbxLayerElementVertexColor* pVertexColor = pBaseLayer->GetVertexColors();
		if (!pVertexColor)
		{
			return;
		}

		if (pVertexColor->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			if (pVertexColor->GetReferenceMode() == FbxGeometryElement::eDirect)
			{
				outputColor.x = static_cast<float>(pVertexColor->GetDirectArray().GetAt(vertexIndex).mRed);
				outputColor.y = static_cast<float>(pVertexColor->GetDirectArray().GetAt(vertexIndex).mGreen);
				outputColor.z = static_cast<float>(pVertexColor->GetDirectArray().GetAt(vertexIndex).mBlue);
				outputColor.w = static_cast<float>(pVertexColor->GetDirectArray().GetAt(vertexIndex).mAlpha);
			}
			else if (pVertexColor->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
			{
				int id = pVertexColor->GetIndexArray().GetAt(vertexIndex);
				outputColor.x = static_cast<float>(pVertexColor->GetDirectArray().GetAt(id).mRed);
				outputColor.y = static_cast<float>(pVertexColor->GetDirectArray().GetAt(id).mGreen);
				outputColor.z = static_cast<float>(pVertexColor->GetDirectArray().GetAt(id).mBlue);
				outputColor.w = static_cast<float>(pVertexColor->GetDirectArray().GetAt(id).mAlpha);
			}
		}
		else if (pVertexColor->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			if (pVertexColor->GetReferenceMode() == FbxGeometryElement::eDirect)
			{
				outputColor.x = static_cast<float>(pVertexColor->GetDirectArray().GetAt(colorIndex).mRed);
				outputColor.y = static_cast<float>(pVertexColor->GetDirectArray().GetAt(colorIndex).mGreen);
				outputColor.z = static_cast<float>(pVertexColor->GetDirectArray().GetAt(colorIndex).mBlue);
				outputColor.w = static_cast<float>(pVertexColor->GetDirectArray().GetAt(colorIndex).mAlpha);
			}
			else if (pVertexColor->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
			{
				int id = pVertexColor->GetIndexArray().GetAt(colorIndex);
				outputColor.x = static_cast<float>(pVertexColor->GetDirectArray().GetAt(id).mRed);
				outputColor.y = static_cast<float>(pVertexColor->GetDirectArray().GetAt(id).mGreen);
				outputColor.z = static_cast<float>(pVertexColor->GetDirectArray().GetAt(id).mBlue);
				outputColor.w = static_cast<float>(pVertexColor->GetDirectArray().GetAt(id).mAlpha);
			}
		}
	}

	void RDFbxImporter::ReadTexcoord(FbxMesh* pMesh, int vertexIndex, int textureUVIndex, int uvLayer, Vector2& OutputUV)
	{
		FbxGeometryElementUV* pVertexUV = pMesh->GetElementUV(uvLayer);

		if (pVertexUV->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			if (pVertexUV->GetReferenceMode() == FbxGeometryElement::eDirect)
			{
				OutputUV.x = static_cast<float>(pVertexUV->GetDirectArray().GetAt(vertexIndex)[0]);
				OutputUV.y = static_cast<float>(pVertexUV->GetDirectArray().GetAt(vertexIndex)[1]);
			}
			else if (pVertexUV->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
			{
				int id = pVertexUV->GetIndexArray().GetAt(vertexIndex);
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

	void RDFbxImporter::ProcessSkeletonNode(FbxNode* pNode, RawBoneData* pParentBone)
	{
		RawBoneData* pBone = nullptr;
		if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			pBone = new RawBoneData();

			FbxTimeSpan timeSpan;
			m_pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(timeSpan);
			FbxTime start = timeSpan.GetStart();
			FbxTime end = timeSpan.GetStop();
			pBone->name = pNode->GetName();

			FbxAMatrix fbxLocalMatrix = pNode->EvaluateLocalTransform(start);
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					pBone->upToParentMatrix(i, j) = static_cast<float>(fbxLocalMatrix.Get(i, j));
				}
			}

			pBone->index = static_cast<int>(m_pSkeleton->bones.size());
			pBone->parentIndex = pParentBone ? pParentBone->index : -1;
			m_pSkeleton->bones.push_back(pBone);
			m_pSkeleton->boneMap.insert({ pBone->name, pBone });
		}
		
		for (int i = 0; i < pNode->GetChildCount(); ++i)
		{
			ProcessSkeletonNode(pNode->GetChild(i), pBone);
		}
	}

	void RDFbxImporter::GetBoneSkins(FbxSkin* pSkin, std::unordered_map<int, VertexBones>& vertBonesMap)
	{
		int clusterCnt = pSkin->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex < clusterCnt; clusterIndex++)
		{
			FbxCluster* pCluster = pSkin->GetCluster(clusterIndex);
			FbxNode* pFbxBone = pCluster->GetLink();

			int* pControlPointIndex = pCluster->GetControlPointIndices();
			double* pWeights = pCluster->GetControlPointWeights();

			int ctrlPointIndexCnt = pCluster->GetControlPointIndicesCount();
			for (int ctrlPointIndex = 0; ctrlPointIndex < ctrlPointIndexCnt; ctrlPointIndex++)
			{
				int vertexIndex = pControlPointIndex[ctrlPointIndex];
				auto vertBone = vertBonesMap.find(vertexIndex);
				if (vertBone == vertBonesMap.end())
				{
					VertexBones vertBones;
					vertBones.boneNames.push_back(pFbxBone->GetName());
					vertBones.boneWeights.push_back((float)pWeights[ctrlPointIndex]);
					vertBonesMap.insert({ vertexIndex, vertBones });
				}
				else
				{
					VertexBones& vertBones = vertBone->second;
					size_t j = 0;
					for (j = 0; j < vertBones.boneNames.size(); ++j)
					{
						if (pFbxBone->GetName() == vertBones.boneNames[j])
						{
							vertBones.boneWeights[j] += (float)pWeights[ctrlPointIndex];
							break;
						}
					}
					if (j == vertBones.boneNames.size())
					{
						vertBones.boneNames.push_back(pFbxBone->GetName());
						vertBones.boneWeights.push_back((float)pWeights[ctrlPointIndex]);
					}
				}
			}
		}
	}

	void RDFbxImporter::GetOffsetMatrix(FbxSkin* pSkin)
	{
		int clusterCnt = pSkin->GetClusterCount();
		for (int i = 0; i < clusterCnt; i++)
		{
			FbxCluster* pCluster = pSkin->GetCluster(i);
			FbxNode* pFbxBone = pCluster->GetLink();
			RawBoneData* pBone = m_pSkeleton->GetBone(pFbxBone->GetName());
			
			FbxAMatrix fbxWorldToLocal;
			FbxAMatrix fbxWorldToBone;
			pCluster->GetTransformLinkMatrix(fbxWorldToLocal);
			pCluster->GetTransformMatrix(fbxWorldToBone);
			FbxAMatrix fbxOffsetMatrix = fbxWorldToLocal.Inverse() * fbxWorldToBone;

			Matrix4x4 boneOffsetMatrix;
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					boneOffsetMatrix(i, j) = static_cast<float>(fbxOffsetMatrix.Get(i, j));
				}
			}
			pBone->offsetMatrix = boneOffsetMatrix;
		}
	}

	void RDFbxImporter::ReadBoneSkin(int vertexIndex, const std::unordered_map<int, VertexBones>& vertBonesMap, 
									 std::vector<std::string>& outputBone, std::vector<float>& outputWeights)
	{
		auto vertBoneIter = vertBonesMap.find(vertexIndex);
		if (vertBoneIter == vertBonesMap.end())
		{
			return;
		}

		const VertexBones& vertBones = vertBoneIter->second;

		outputBone = vertBones.boneNames;
		outputWeights = vertBones.boneWeights;
	}

	void RDFbxImporter::ProcessAnimation(FbxNode* pNode)
	{
		FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
		if (pNodeAttribute && 
			(pNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNull || pNodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton))
		{
			RawBoneAnimation* pCurrBoneAnimation = new RawBoneAnimation();
			pCurrBoneAnimation->boneName = pNode->GetName();

			FbxTimeSpan AnimTimeSpan(FBXSDK_TIME_INFINITE, FBXSDK_TIME_MINUS_INFINITE);
			pNode->GetAnimationInterval(AnimTimeSpan);

			FbxTime frameTime;
			frameTime.SetTime(0, 0, 0, 1, 0, m_pScene->GetGlobalSettings().GetTimeMode());

			FbxTimeSpan timeSpan = AnimTimeSpan;
			FbxTime startTime = timeSpan.GetStart();
			FbxTime endTime = timeSpan.GetStop();

			size_t stepTime = (size_t)((endTime.Get() - startTime.Get()) / frameTime.Get());
			pCurrBoneAnimation->keyFrames.reserve(stepTime + 1);
			for (FbxTime currTime = startTime; currTime < endTime; currTime += frameTime)
			{
				FbxLongLong msTime = currTime.GetMilliSeconds() - startTime.GetMilliSeconds();

				FbxAMatrix fbxBoneUpToParent = pNode->EvaluateLocalTransform(currTime);

				FbxVector4 fbxTrans = fbxBoneUpToParent.GetT();
				FbxVector4 fbxScales = fbxBoneUpToParent.GetS();
				FbxQuaternion fbxQuat = fbxBoneUpToParent.GetQ();

				RawKeyFrameData keyFrameData;
				keyFrameData.timePos = static_cast<float>(msTime);

				keyFrameData.translation.x = static_cast<float>(fbxTrans[0]);
				keyFrameData.translation.y = static_cast<float>(fbxTrans[1]);
				keyFrameData.translation.z = static_cast<float>(fbxTrans[2]);

				keyFrameData.scales.x = static_cast<float>(fbxScales[0]);
				keyFrameData.scales.y = static_cast<float>(fbxScales[1]);
				keyFrameData.scales.z = static_cast<float>(fbxScales[2]);

				keyFrameData.rotationQuat.x = static_cast<float>(fbxQuat[0]);
				keyFrameData.rotationQuat.y = static_cast<float>(fbxQuat[1]);
				keyFrameData.rotationQuat.z = static_cast<float>(fbxQuat[2]);
				keyFrameData.rotationQuat.w = static_cast<float>(fbxQuat[3]);

				pCurrBoneAnimation->keyFrames.push_back(keyFrameData);
			}

			FbxLongLong msEndTime = endTime.GetMilliSeconds() - startTime.GetMilliSeconds();
			FbxAMatrix fbxEndBoneUpToParent = pNode->EvaluateLocalTransform(msEndTime);

			FbxVector4 fbxEndTrans = fbxEndBoneUpToParent.GetT();
			FbxVector4 fbxEndScales = fbxEndBoneUpToParent.GetS();
			FbxQuaternion fbxEndQuat = fbxEndBoneUpToParent.GetQ();
			//FbxVector4 fbxEndEulers = fbxEndBoneUpToParent.GetR();

			RawKeyFrameData endFrameData;
			endFrameData.timePos = static_cast<float>(msEndTime);

			endFrameData.translation.x = static_cast<float>(fbxEndTrans[0]);
			endFrameData.translation.y = static_cast<float>(fbxEndTrans[1]);
			endFrameData.translation.z = static_cast<float>(fbxEndTrans[2]);

			endFrameData.scales.x = static_cast<float>(fbxEndScales[0]);
			endFrameData.scales.y = static_cast<float>(fbxEndScales[1]);
			endFrameData.scales.z = static_cast<float>(fbxEndScales[2]);

			endFrameData.rotationQuat.x = static_cast<float>(fbxEndQuat[0]);
			endFrameData.rotationQuat.y = static_cast<float>(fbxEndQuat[1]);
			endFrameData.rotationQuat.z = static_cast<float>(fbxEndQuat[2]);
			endFrameData.rotationQuat.w = static_cast<float>(fbxEndQuat[3]);

			pCurrBoneAnimation->keyFrames.push_back(endFrameData);

			m_BoneAnimations.boneAnimations.push_back(pCurrBoneAnimation);
		}

		for (int i = 0; i < pNode->GetChildCount(); ++i)
		{
			ProcessAnimation(pNode->GetChild(i));
		}
	}

}// namespace RenderDog