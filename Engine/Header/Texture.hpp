#pragma once

// OpenGL Includes
#include <glad/glad.h>

// Third Party Incudes
#include "STB/stb_image.h"

// STD Includes
#include <string>
#include <map>
#include <vector>

// Project Includes
#include "Shader.hpp"

enum TextureType
{
	Albedo,
	Normal,
	Metalic,
	Roughness,
	AmbientOclusion,
};

class Texture
{
public:
	enum WRAPPING
	{
		REPEATU = 1 << 0,
		REPEATV = 1 << 1,
		REPEATW = 1 << 2,

		MIRRORU = 1 << 3,
		MIRRORV = 1 << 4,
		MIRRORW = 1 << 5,

		STRECH_BORDERU = 1 << 6,
		STRECH_BORDERV = 1 << 7,
		STRECH_BORDERW = 1 << 8,

		DEFAULTU = 1 << 9,
		DEFAULTV = 1 << 10,
		DEFAULTW = 1 << 11,

		UNSET = 0x07 // REPEATU | REPEATV | REPEATW
	};



	enum MAGNIFYING
	{
		MAGNIFY_NEAREST,
		MAGNIFY_LINEAR
	};

	enum MIPMAPPING
	{
		MINIFY_NEAREST,
		MINIFY_LINEAR,
		NEAREST_NEAREST,
		LINEAR_NEAREST,
		NEAREST_LINEAR,
		LINEAR_LINEAR,
	};

	std::map<int, int> mipmappingMap { 
		{MINIFY_NEAREST, GL_NEAREST}, 
		{MINIFY_LINEAR, GL_LINEAR}, 
		{NEAREST_NEAREST, GL_NEAREST_MIPMAP_NEAREST},
		{LINEAR_NEAREST, GL_LINEAR_MIPMAP_NEAREST},
		{NEAREST_LINEAR, GL_NEAREST_MIPMAP_LINEAR},
		{LINEAR_LINEAR, GL_LINEAR_MIPMAP_LINEAR}
	};

public:
	Texture(std::string path, const WRAPPING wrapping = UNSET, const MAGNIFYING magnifying = MAGNIFY_NEAREST, const MIPMAPPING mipmapping = NEAREST_NEAREST);
	~Texture();

	static std::vector<std::shared_ptr<Texture>> TextureStorage;
	unsigned int ID;
	std::string path;

private:
	int width, height, nrChannels;
	void Setup(const WRAPPING wrapping, const MAGNIFYING magnifying, const MIPMAPPING mipmapping);
};

inline Texture::WRAPPING operator|(Texture::WRAPPING a, Texture::WRAPPING b)
{
	return static_cast<Texture::WRAPPING>(static_cast<int>(a) | static_cast<int>(b));
}