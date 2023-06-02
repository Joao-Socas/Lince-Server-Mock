// Project Includes
#include "Mesh.hpp"

Mesh::Mesh(std::unique_ptr<std::vector<Vertex>> _vertices, std::unique_ptr<std::vector<std::array<unsigned int, 3>>> _triangles, std::unique_ptr<std::multimap<TextureType, std::shared_ptr<Texture>>> _textures)
{
    vertices = std::move(_vertices);
    triangles = std::move(_triangles);
    textures = std::move(_textures);
	glGenVertexArrays(1, &Vertex_Array_Object_ID);
	glGenBuffers(1, &Vertex_Buffer_Object_ID);
	glGenBuffers(1, &Element_Buffer_Object_ID);

    //std::vector<Vertex> TestVertices = {};
    //Vertex a, b, c, d, e, f;
    //a.Position = { 0.0f, 0.0f, 0.0f };
    //b.Position = { 0.5f, 0.0f, 0.0f };
    //c.Position = { 0.5f, 0.5f, 0.0f };
    //d.Position = { 0.0f, 0.0f, 0.0f };
    //e.Position = { 0.5f, 0.5f, 0.0f };
    //f.Position = { 0.0f, 0.5f, 0.0f };
    //TestVertices.push_back(a);
    //TestVertices.push_back(b);
    //TestVertices.push_back(c);
    //TestVertices.push_back(d);
    //TestVertices.push_back(e);
    //TestVertices.push_back(f);
    //TestVertices = *vertices.get();


	// Vertex array and buffer binding and data setting
	glBindVertexArray(Vertex_Array_Object_ID);
	glBindBuffer(GL_ARRAY_BUFFER, Vertex_Buffer_Object_ID);
	glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(Vertex), &(*vertices.get())[0], GL_STATIC_DRAW);

    //std::vector<unsigned int> TestTriangles = {0,1,2,3,4,5};
	// Element buffer binding and data setting
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Element_Buffer_Object_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles->size() * sizeof(unsigned int) * 3, &(*triangles.get())[0], GL_STATIC_DRAW);

	// Setting pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneWeights));
    // Unbiding
    glBindVertexArray(0);
}


void Mesh::Draw(ShaderProgram& shader_program)
{
    unsigned int TextureNumber = 0, lastTextureNumber = 0;

    // Lambda to not repeat the same code a lot of times
    auto bindTextures
    {
        [&shader_program, &TextureNumber, &lastTextureNumber](std::pair<std::multimap<TextureType, std::shared_ptr<Texture>>::iterator, std::multimap<TextureType, std::shared_ptr<Texture>>::iterator> iterator_range, const char* NTextureUniform)
        {
            for (auto bindingTexture = iterator_range.first; bindingTexture != iterator_range.second; bindingTexture++)
            {
                glActiveTexture(GL_TEXTURE0 + TextureNumber);
                glUniform1i(glGetUniformLocation(shader_program.ID, std::string("Texture" + std::to_string(TextureNumber)).c_str()), TextureNumber);
                glBindTexture(GL_TEXTURE_2D, bindingTexture->second->ID);
                TextureNumber++;
            }
            glUniform1i(glGetUniformLocation(shader_program.ID, NTextureUniform), TextureNumber - lastTextureNumber);
            lastTextureNumber = TextureNumber;
        }
    };

    // Albedo textures bindings
    auto iterator_range = textures->equal_range(TextureType::Albedo);
    bindTextures(iterator_range, "NTextureAlbedo");

    // Normal textures bidings
    iterator_range = textures->equal_range(TextureType::Normal);
    bindTextures(iterator_range, "NTextureNormal");

    // Metalic textures bidings
    iterator_range = textures->equal_range(TextureType::Metalic);
    //bindTextures(iterator_range, "NTextureMetalic");

    // Roughness textures bidings
    iterator_range = textures->equal_range(TextureType::Roughness);
    //bindTextures(iterator_range, "NTextureRoughness");

    // AmbientOclusion textures bidings
    iterator_range = textures->equal_range(TextureType::AmbientOclusion);
    //bindTextures(iterator_range, "NTextureAmbientOclusion");

    // Drawing
    glBindVertexArray(Vertex_Array_Object_ID);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(triangles->size() * 3), GL_UNSIGNED_INT, 0);
    
    // Cleaning
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}
