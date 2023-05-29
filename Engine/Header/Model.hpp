#pragma once
// OpenGL Includes
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// STD Includes
#include <string>
#include <vector>

// Engine Includes
#include "Shader.hpp"
#include "Mesh.hpp"

class Model
{
public:
	Model(std::string path);
	~Model();

	void Draw(ShaderProgram& shader_program);

private:
	std::vector<Mesh> meshes;
	std::string directory;

	void processNode_Recursive(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);

	void loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texture_type, std::unique_ptr<std::multimap<TextureType, std::shared_ptr<Texture>>>& textures);

};

