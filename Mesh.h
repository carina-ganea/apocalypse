#pragma once
#include <string>
#include <vector>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include <glm/glm.hpp>

// Project includes
#include "maths_funcs.h"

#pragma region SimpleTypes

typedef struct ModelData
{
	size_t mMeshCount = 0;
	std::vector<size_t> mPointCount;
	std::vector<std::vector<vec3>> mVertices;
	std::vector<std::vector<vec3>> mNormals;
	std::vector<std::vector<vec2>> mTextureCoords;
	std::vector<size_t> useMat;
	size_t mMaterialCount = 0;
	std::vector<glm::vec3> mAmbient;
	std::vector<glm::vec3> mDiffuse;
	std::vector<glm::vec3> mSpec;
	std::vector<float> mSpecExp;
	std::vector<const char*> mTextureFiles;
	std::vector<bool> hasTexture;
};
#pragma endregion SimpleTypes

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData error;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return error;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);


	ModelData modelData;
	modelData.mMeshCount = scene->mNumMeshes;
	modelData.mMaterialCount = scene->mNumMaterials;

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount.push_back(mesh->mNumVertices);
		modelData.mVertices.push_back({});
		modelData.mNormals.push_back({});
		modelData.mTextureCoords.push_back({});

		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices[m_i].push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals[m_i].push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords[m_i].push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
		modelData.useMat.push_back(mesh->mMaterialIndex);
	}
	if (scene->HasMaterials())
	{
		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			aiMaterial* material = scene->mMaterials[i];
			aiString name;
			material->Get(AI_MATKEY_NAME, name);

			aiColor3D ambient = (0.f, 0.f, 0.f);
			material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
			modelData.mAmbient.push_back(glm::vec3(ambient.r, ambient.g, ambient.b));

			aiColor3D diffuse = (0.f, 0.f, 0.f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			//printf("diffuse %f %f %f \n", diffuse.r, diffuse.g, diffuse.b);
			modelData.mDiffuse.push_back(glm::vec3(diffuse.r, diffuse.g, diffuse.b));

			aiColor3D specular = (0.f, 0.f, 0.f);
			material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
			//printf("specular %f %f %f \n", specular.r, specular.g, specular.b);
			modelData.mSpec.push_back(glm::vec3(specular.r, specular.g, specular.b));

			float shininess;
			material->Get(AI_MATKEY_SHININESS, shininess);
			modelData.mSpecExp.push_back(shininess);
			//printf("specular exp %f \n", shininess);

			aiString path;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL);
			printf(path.C_Str());
			modelData.mTextureFiles.push_back(path.C_Str());

			if (path.C_Str() == NULL)
			{
				modelData.hasTexture.push_back(false);
			}
			else
			{
				modelData.hasTexture.push_back(true);
			}
			
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING