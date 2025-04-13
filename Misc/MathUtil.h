#pragma once

#include "Globals.h"
#include "../PSUStructs/Vector2F.hpp"
#include "../PSUStructs/Vector3F.hpp"
#include "../PSUStructs/Matrix4x4.hpp"

namespace MathUtil
{
	template<typename T>
	inline T Clamp( T value, T min, T max )
	{
		if( value < min ) return min;
		if( value > max ) return max;
		return value;
	}

	template<typename T>
	inline bool IsInRange( T value, T min, T max )
	{
		return ( value >= min && value <= max );
	}

	template<typename T>
	inline T RoundUp( T value, T multiple )
	{
		static_assert( std::is_integral<T>::value, "RoundUp requires integral type" );
		if( multiple == 0 ) return value;
		return ( value + multiple - 1 ) / multiple * multiple;
	}

	template<typename T>
	inline T RoundDown( T value, T multiple )
	{
		static_assert( std::is_integral<T>::value, "RoundDown requires integral type" );
		if( multiple == 0 ) return value;
		return value / multiple * multiple;
	}

	inline bool ToScreenSpace( const Vector3F &worldPos, Vector2F &screenPos, const Matrix4x4 &viewProjMatrix )
	{
		float x = worldPos.x * viewProjMatrix.m[ 0 ][ 0 ] + worldPos.y * viewProjMatrix.m[ 1 ][ 0 ] + worldPos.z * viewProjMatrix.m[ 2 ][ 0 ] + viewProjMatrix.m[ 3 ][ 0 ];
		float y = worldPos.x * viewProjMatrix.m[ 0 ][ 1 ] + worldPos.y * viewProjMatrix.m[ 1 ][ 1 ] + worldPos.z * viewProjMatrix.m[ 2 ][ 1 ] + viewProjMatrix.m[ 3 ][ 1 ];
		float z = worldPos.x * viewProjMatrix.m[ 0 ][ 2 ] + worldPos.y * viewProjMatrix.m[ 1 ][ 2 ] + worldPos.z * viewProjMatrix.m[ 2 ][ 2 ] + viewProjMatrix.m[ 3 ][ 2 ];
		float w = worldPos.x * viewProjMatrix.m[ 0 ][ 3 ] + worldPos.y * viewProjMatrix.m[ 1 ][ 3 ] + worldPos.z * viewProjMatrix.m[ 2 ][ 3 ] + viewProjMatrix.m[ 3 ][ 3 ];

		if( w <= 0.0f )
			return false; // Behind the camera

		float invW = 1.0f / w;
		float ndcX = x * invW;
		float ndcY = y * invW;

		screenPos.x = ( ndcX + 1.0f ) * 0.5f * Global::screenWidth;
		screenPos.y = ( 1.0f - ndcY ) * 0.5f * Global::screenHeight;

		return true;
	}
}