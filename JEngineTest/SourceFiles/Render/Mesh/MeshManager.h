#pragma once
#include "../../../stdafx.h"
#include "Mesh.h"
//#include "Render/Mesh/FBX/FBXLoader.h"
//#include "Render/Mesh/Mesh.h"
//#include "Asset/Manifest/ManifestLoader.h"
//#include "Core/Containers/DynamicArray.h"

class MeshManager
{
public:
	MeshManager();
	~MeshManager();

	void LoadAllMeshes(Direct3DManager *direct3DManager);
	Mesh *GetMesh(const std::string &meshName);

private:
	Mesh *LoadFromAssimp(Direct3DManager *direct3DManager, char *fileName, char *serializationFile);
	Mesh *LoadFBXMesh(Direct3DManager *direct3DManager, char *fileName, char *serializationFile);
	
	void SerializeMeshToFile(Mesh *mesh, char *fileName);
	Mesh *DeserializeMeshFromFile(Direct3DManager *direct3DManager, char *fileName);
    Mesh *GetGridMesh(Direct3DManager *direct3DManager, UINT xTiles, UINT yTiles, float texTileX, float texTileZ);

	std::unordered_map<std::string, Mesh*> mMeshLookup;
	std::vector<Mesh*> mMeshes;

	ManifestLoader mManifestLoader;
	FBXLoader mFBXLoader;
};