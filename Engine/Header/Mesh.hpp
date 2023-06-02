#pragma once

// OpenGL Includes
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// STD Includes
#include <string>
#include <vector>
#include <array>

// Project Includes
#include "Shader.hpp"
#include "Texture.hpp"

#
#define MAX_BONE_INFLUENCE 4

struct Vertex 
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;

    glm::vec2 TextureCoords;

    //bone indexes which will influence this vertex
    int BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float BoneWeights[MAX_BONE_INFLUENCE];
};

class Mesh
{
public:
    
    Mesh(std::unique_ptr<std::vector<Vertex>> _vertices, std::unique_ptr<std::vector<std::array<unsigned int, 3>>> _triangles, std::unique_ptr<std::multimap<TextureType, std::shared_ptr<Texture>>> _textures);
    //~Mesh();

    void Draw(ShaderProgram& shader_program);

private:
    std::unique_ptr<std::vector<Vertex>> vertices;
    std::unique_ptr<std::vector<std::array<unsigned int,3>>> triangles;
    std::unique_ptr<std::multimap<TextureType,std::shared_ptr<Texture>>> textures;
    //std::unique_ptr<ShaderProgram> shader_program;

    unsigned int Vertex_Array_Object_ID, Vertex_Buffer_Object_ID, Element_Buffer_Object_ID;
};