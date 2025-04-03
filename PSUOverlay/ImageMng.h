#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <imgui.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

#include "../resource.h"

class ImageData
{
public:
	PDIRECT3DTEXTURE9 m_data;
	ImTextureID m_texId;
	ImVec2 m_size;

	ImageData()
	{
		m_data = nullptr;
		m_texId = 0;
		m_size = ImVec2( 0, 0 );
	}
};

typedef std::shared_ptr< ImageData > sptr_image;

class ImageManager
{
private:
	static inline LPDIRECT3DDEVICE9 m_pDevice;

	static inline sptr_image m_whiteTexture;
	static inline std::unordered_map < int, sptr_image > m_textures;

public:
	ImageManager();
	~ImageManager();

	static ImageManager &Get()
	{
		static ImageManager instance;
		return instance;
	}

	ImageManager( const ImageManager & ) = delete;
	ImageManager &operator=( const ImageManager & ) = delete;

	void Initialize( LPDIRECT3DDEVICE9 pDevice );
	void CreateWhiteTexture();
	sptr_image LoadTextureFromResource( int resourceId ) const;
	sptr_image LoadTextureFromMemory( int resourceId ) const;

	// Overloading operator[] for convenient access
	sptr_image operator[]( int resourceId ) const
	{
		auto it = m_textures.find( resourceId );
		if( it != m_textures.end() )
			return it->second;

		return m_whiteTexture;
	}
};