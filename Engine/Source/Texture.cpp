#include "Texture.hpp"

std::vector<std::shared_ptr<Texture>> Texture::TextureStorage = std::vector<std::shared_ptr<Texture>>{};

Texture::Texture(std::string path, const WRAPPING wrapping, const MAGNIFYING magnifying, const MIPMAPPING mipmapping) : path(path)
{
	glGenTextures(1, &ID);
	
	unsigned char* textureData = stbi_load(("Application/Models/" + path).c_str(), &width, &height, &nrChannels, 0);
	if (textureData)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, ID);
		Setup(wrapping, magnifying, mipmapping);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	// Cleaning
	stbi_image_free(textureData);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
}

void Texture::Setup(const WRAPPING wrapping, const MAGNIFYING magnifying, const MIPMAPPING mipmapping)
{
	if (wrapping & WRAPPING::REPEATU)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	}
	if (wrapping & WRAPPING::REPEATV)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	if (wrapping & WRAPPING::REPEATW)
	{
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}

	if (wrapping & WRAPPING::MIRRORU)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	}
	if (wrapping & WRAPPING::MIRRORV)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	}
	if (wrapping & WRAPPING::MIRRORW)
	{
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	}

	if (wrapping & WRAPPING::STRECH_BORDERU)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	}
	if (wrapping & WRAPPING::STRECH_BORDERW)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	if (wrapping & WRAPPING::STRECH_BORDERW)
	{
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	if (wrapping & WRAPPING::DEFAULTU)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	}
	if (wrapping & WRAPPING::DEFAULTW)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	if (wrapping & WRAPPING::DEFAULTW)
	{
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}


	if (magnifying == MAGNIFY_LINEAR)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmappingMap.at(mipmapping));
}
