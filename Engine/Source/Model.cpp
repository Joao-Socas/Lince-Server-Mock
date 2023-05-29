#include "Model.hpp"


Model::Model(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    // Error handling
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode_Recursive(scene->mRootNode, scene);
}

Model::~Model()
{
}

void Model::Draw(ShaderProgram& shader_program)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].Draw(shader_program);
    }
}

void Model::processNode_Recursive(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode_Recursive(node->mChildren[i], scene);
    }
}

void Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // Unique pointer generation to prevent copy constructors to be called on extensive vectors
    std::unique_ptr<std::vector<Vertex>>                                    vertices = std::make_unique<std::vector<Vertex>>();
    std::unique_ptr<std::vector<std::array<unsigned int, 3>>>               triangles = std::make_unique<std::vector<std::array<unsigned int, 3>>>();
    std::unique_ptr<std::multimap<TextureType, std::shared_ptr<Texture>>>   textures = std::make_unique<std::multimap<TextureType, std::shared_ptr<Texture>>>();

    // Vertices processing
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;

        if (mesh->HasNormals())
        {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }

        // texture coordinates
        if (mesh->mTextureCoords)
        {
            vertex.TextureCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.TextureCoords.y = mesh->mTextureCoords[0][i].y;
            
            vertex.Tangent.x = mesh->mTangents[i].x;
            vertex.Tangent.y = mesh->mTangents[i].y;
            vertex.Tangent.z = mesh->mTangents[i].z;

            vertex.Bitangent.x = mesh->mBitangents[i].x;
            vertex.Bitangent.y = mesh->mBitangents[i].y;
            vertex.Bitangent.z = mesh->mBitangents[i].z;
        }
        else
        {
            vertex.TextureCoords = {0.0f, 0.0f};
        }

        vertices->push_back(std::move(vertex));
    }

    // Processing triangles
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        // Since aiProcess_Triangulate, faces are aways triangles
        triangles->push_back(std::array<unsigned int, 3>{ mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1] ,mesh->mFaces[i].mIndices[2] });
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Albedo, textures);
        loadMaterialTextures(material, aiTextureType_HEIGHT, TextureType::Normal, textures);
        loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Normal, textures);

    }
    meshes.push_back(std::move(Mesh(std::move(vertices), std::move(triangles), std::move(textures))));


}

void Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texture_type, std::unique_ptr<std::multimap<TextureType, std::shared_ptr<Texture>>>& textures)
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString FilePath;
        mat->GetTexture(type, i, &FilePath);

        bool textureLoaded = false;
        for (auto texture = Texture::TextureStorage.begin(); texture!= Texture::TextureStorage.end() && !textureLoaded; texture++)
        {
            if (std::strcmp((*texture)->path.c_str(), FilePath.C_Str()) == 0)
            {
                textures->insert({texture_type, *texture});
                textureLoaded = true;
            }
        }

        if (!textureLoaded)
        {
            std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(FilePath.C_Str(), (Texture::DEFAULTU | Texture::DEFAULTV));
            Texture::TextureStorage.push_back(newTexture);
            textures->insert({ texture_type, newTexture });
        }
    }
}
