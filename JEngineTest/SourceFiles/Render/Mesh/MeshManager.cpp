#include "MeshManager.h"
//#include "Util/File/FileUtil.h"
//#include "Util/String/StringConverter.h"
//#include <assimp/Importer.hpp>      
//#include <assimp/scene.h>           
//#include <assimp/postprocess.h> 

MeshManager::MeshManager()
{
}

MeshManager::~MeshManager()
{
	for (UINT i = 0; i < mMeshes.CurrentSize(); i++)
	{
		delete mMeshes[i];
		mMeshes[i] = NULL;
	}
	mMeshes.Clear();
	mMeshLookup.clear();
}

Mesh *MeshManager::GetMesh(const std::string &meshName)
{
	return mMeshLookup[meshName];
}

void MeshManager::LoadAllMeshes(Direct3DManager *direct3DManager)
{
	mManifestLoader.LoadManifest(ApplicationSpecification::MeshManifestFileLocation);

	std::string serializedFileLocation = ApplicationSpecification::MeshSerializationLocation;

	DynamicArray<std::string, false> &fileNames = mManifestLoader.GetFileNames();

	for(UINT i = 0; i < fileNames.CurrentSize(); i++)
	{
		UINT lastSlash = (UINT)fileNames[i].find_last_of("/");
		UINT lastDot = (UINT)fileNames[i].find_last_of(".");
		std::string justFileName = fileNames[i].substr(lastSlash+1, (lastDot-lastSlash)-1);
		Mesh *newMesh = NULL;

		std::string serializedName = serializedFileLocation + justFileName + ".sem";

		if(StringConverter::DoesStringEndWith(fileNames[i], ".fbx"))
		{
			if(FileUtil::DoesFileExist(serializedName) && !ApplicationSpecification::RebuildAllMeshes)
			{
				newMesh = DeserializeMeshFromFile(direct3DManager, (char*)serializedName.c_str());
			}
			else
			{
				newMesh = LoadFBXMesh(direct3DManager, (char*)fileNames[i].c_str(), (char*)serializedName.c_str());
			}
		}
		else
		{
			if(FileUtil::DoesFileExist(serializedName) && !ApplicationSpecification::RebuildAllMeshes)
			{
				newMesh = DeserializeMeshFromFile(direct3DManager, (char*)serializedName.c_str());
			}
			else
			{
				newMesh = LoadFromAssimp(direct3DManager, (char*)fileNames[i].c_str(), (char*)serializedName.c_str());
			}
		}

		mMeshLookup.insert(std::pair<std::string, Mesh*>(justFileName, newMesh));
		mMeshes.Add(newMesh);
	}

    Mesh *gridMesh = GetGridMesh(direct3DManager, 50, 50, 5.0f, 5.0f);
    mMeshLookup.insert(std::pair<std::string, Mesh*>("WaterPlane", gridMesh));
    mMeshes.Add(gridMesh);
}

Mesh *MeshManager::LoadFBXMesh(Direct3DManager *direct3DManager, char *fileName, char *serializationFile)
{
	DynamicArray<MeshVertex> fbxData;
	DynamicArray<UINT> indexSplits;

	mFBXLoader.LoadFBX(fileName, fbxData, indexSplits);

	UINT fbxDataCount = fbxData.CurrentSize();

	UINT *meshIndices = new UINT[fbxDataCount];
	for (UINT i = 0; i < fbxDataCount; i++)
	{
		meshIndices[i] = i;
	}

	Mesh *fbxMesh = new Mesh(direct3DManager, fbxDataCount, fbxDataCount, fbxData.GetInnerArrayCopy(), indexSplits, meshIndices);
	SerializeMeshToFile(fbxMesh, serializationFile);

	return fbxMesh;
}

void MeshManager::SerializeMeshToFile(Mesh *mesh, char *fileName)
{
	std::ofstream outputFile(fileName, std::ios::binary);

	MeshVertex *meshData = mesh->GetMeshData();
	UINT indexCount = mesh->GetIndexCount();

	outputFile.write((char*)&indexCount, sizeof(indexCount));
	outputFile.write((char*)meshData, sizeof(MeshVertex) * mesh->GetIndexCount());
	outputFile.write((char*)mesh->GetIndices(), sizeof(UINT) * mesh->GetIndexCount());
	UINT splitCount = mesh->GetMeshSplitCount();

	outputFile.write((char*)&splitCount, sizeof(splitCount));
	for(UINT i = 0; i < splitCount; i++)
	{
		UINT indexSplit = mesh->GetMeshIndexSplitByIndex(i);
		outputFile.write((char*)&indexSplit, sizeof(indexSplit));
	}

	outputFile.close();
}

Mesh *MeshManager::DeserializeMeshFromFile(Direct3DManager *direct3DManager, char *fileName)
{
	UINT indexCount = 0;
	std::ifstream inputFile(fileName, std::ios::binary);

	inputFile.read((char *)&indexCount, sizeof(indexCount));

	MeshVertex *meshData = new MeshVertex[indexCount];
	UINT *meshIndices = new UINT[indexCount];

	inputFile.read((char *)meshData, sizeof(MeshVertex) * indexCount);
	inputFile.read((char *)meshIndices, sizeof(UINT) * indexCount);

	UINT indexSplitCount = 0;
	inputFile.read((char *)&indexSplitCount, sizeof(indexSplitCount));

	DynamicArray<UINT> indexSplits(indexSplitCount);

	for(UINT i = 0; i < indexSplitCount; i++)
	{
		UINT indexNum = 0;
		inputFile.read((char *)&indexNum, sizeof(indexNum));
		indexSplits.Add(indexNum);
	}

	inputFile.close();

	Mesh *newMesh = new Mesh(direct3DManager, indexCount, indexCount, meshData, indexSplits, meshIndices);

	return newMesh;
}

Mesh *MeshManager::LoadFromAssimp(Direct3DManager *direct3DManager, char *fileName, char *serializationFile)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_Triangulate            |
		aiProcess_CalcTangentSpace		 | 
		aiProcess_OptimizeMeshes 
		);

	if( !scene)
	{
		return NULL;
	}

	UINT vertCount = 0;
	UINT indexCount = 0;
	DynamicArray<UINT> indexSplits;

	for(UINT i = 0; i < scene->mNumMeshes; i++)
	{
		vertCount += scene->mMeshes[i]->mNumVertices;
		indexCount += scene->mMeshes[i]->mNumFaces * 3;
		indexSplits.Add(indexCount);
	}

	MeshVertex *meshData = new MeshVertex[vertCount];
	UINT *indices = new UINT[indexCount];

	UINT meshDataIndex = 0;
	UINT indexIndex = 0;

	for(UINT i = 0; i < scene->mNumMeshes; i++)
	{
		for(UINT f = 0; f < scene->mMeshes[i]->mNumFaces; f++)
		{
			indices[indexIndex] = scene->mMeshes[i]->mFaces[f].mIndices[0] + meshDataIndex;
			indexIndex++;
			indices[indexIndex] = scene->mMeshes[i]->mFaces[f].mIndices[1] + meshDataIndex;
			indexIndex++;
			indices[indexIndex] = scene->mMeshes[i]->mFaces[f].mIndices[2] + meshDataIndex;
			indexIndex++;
		}

		for(UINT j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
		{
			aiVector3D vert = scene->mMeshes[i]->mVertices[j];
			aiVector3D normal = scene->mMeshes[i]->mNormals[j];
			aiVector3D texcoord = scene->mMeshes[i]->mTextureCoords[0][j];
			aiVector3D tangent = scene->mMeshes[i]->mTangents[j];
			aiVector3D bitangent = scene->mMeshes[i]->mBitangents[j];

			meshData[meshDataIndex].Position = Vector4(vert.x, vert.y, vert.z, 1.0f);
			meshData[meshDataIndex].Normal = Vector3(normal.x, normal.y, normal.z);
			meshData[meshDataIndex].TexCoord = Vector4(texcoord.x, 1.0f - texcoord.y, texcoord.x, 1.0f - texcoord.y);
			meshData[meshDataIndex].Tangent = Vector3(tangent.x, tangent.y, tangent.z);
			meshData[meshDataIndex].Binormal = Vector3(bitangent.x, bitangent.y, bitangent.z);

			meshDataIndex++;
		}
	}

	Mesh *mesh = new Mesh(direct3DManager, vertCount, indexCount, meshData, indexSplits, indices);

	SerializeMeshToFile(mesh, serializationFile);

	return mesh;
}

Mesh *MeshManager::GetGridMesh(Direct3DManager *direct3DManager, UINT xTiles, UINT zTiles, float texTileX, float texTileZ)
{
    UINT vertexCount = xTiles * zTiles * 6;
    UINT indexCount = vertexCount;
    DynamicArray<UINT> indexSplits;

    MeshVertex *meshData = new MeshVertex[vertexCount];
    UINT *indices = new UINT[indexCount];

    float oneOverXTiles = 1.0f / (float)xTiles;
    float oneOverZTiles = 1.0f / (float)zTiles;

    for (UINT x = 0; x < xTiles; x++)
    {
        for (UINT z = 0; z < zTiles; z++)
        {
            UINT tileIndex = (x * zTiles + z) * 6;
            float xBeginTile = (oneOverXTiles * (float)x) * texTileX;
            float xEndTile = (oneOverXTiles * (float)(x + 1)) * texTileX;
            float zBeginTile = (oneOverZTiles * (float)z) * texTileZ;
            float zEndTile = (oneOverZTiles * (float)(z + 1)) * texTileZ;

            float xBegin = (oneOverXTiles * (float)x);
            float xEnd = (oneOverXTiles * (float)(x + 1));
            float zBegin = (oneOverZTiles * (float)z);
            float zEnd = (oneOverZTiles * (float)(z + 1));

            meshData[tileIndex].Position = Vector4((float)x, 0.0f, (float)z, 1.0f);
            meshData[tileIndex].TexCoord = Vector4(xBeginTile, zBeginTile, xBegin, zBegin);
            indices[tileIndex] = tileIndex;
            tileIndex++;

            meshData[tileIndex].Position = Vector4((float)x, 0.0f, (float)z + 1.0f, 1.0f);
            meshData[tileIndex].TexCoord = Vector4(xBeginTile, zEndTile, xBegin, zEnd);
            indices[tileIndex] = tileIndex;
            tileIndex++;

            meshData[tileIndex].Position = Vector4((float)x + 1.0f, 0.0f, (float)z, 1.0f);
            meshData[tileIndex].TexCoord = Vector4(xEndTile, zBeginTile, xEnd, zBegin);
            indices[tileIndex] = tileIndex;
            tileIndex++;

            meshData[tileIndex].Position = Vector4((float)x, 0.0f, (float)z + 1.0f, 1.0f);
            meshData[tileIndex].TexCoord = Vector4(xBeginTile, zEndTile, xBegin, zEnd);
            indices[tileIndex] = tileIndex;
            tileIndex++;

            meshData[tileIndex].Position = Vector4((float)x + 1.0f, 0.0f, (float)z + 1.0f, 1.0f);
            meshData[tileIndex].TexCoord = Vector4(xEndTile, zEndTile, xEnd, zEnd);
            indices[tileIndex] = tileIndex;
            tileIndex++;

            meshData[tileIndex].Position = Vector4((float)x + 1.0f, 0.0f, (float)z, 1.0f);
            meshData[tileIndex].TexCoord = Vector4(xEndTile, zBeginTile, xEnd, zBegin);
            indices[tileIndex] = tileIndex;
            tileIndex++;
        }
    }

    Mesh *mesh = new Mesh(direct3DManager, vertexCount, indexCount, meshData, indexSplits, indices);
    return mesh;
}