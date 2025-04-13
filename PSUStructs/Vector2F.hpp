#pragma once

#include <limits>
#include <cmath>

class Vector2F
{
public:

	Vector2F( float x, float y )
		: x( x ), y( y )
	{
	}

	Vector2F()
		: x( 0.0f ), y( 0.0f )
	{
	}

	Vector2F( const Vector2F &other )
		: x( other.x ), y( other.y )
	{
	}

	~Vector2F()
	{
	}

	Vector2F &operator=( const Vector2F &other )
	{
		x = other.x;
		y = other.y;
		return *this;
	}

	Vector2F operator+( const Vector2F &other ) const
	{
		return Vector2F( x + other.x, y + other.y );
	}

	Vector2F operator-( const Vector2F &other ) const
	{
		return Vector2F( x - other.x, y - other.y );
	}

	Vector2F operator*( float scalar ) const
	{
		return Vector2F( x * scalar, y * scalar );
	}

	Vector2F operator/( float scalar ) const
	{
		return Vector2F( x / scalar, y / scalar );
	}

	Vector2F &operator+=( const Vector2F &other )
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	Vector2F &operator-=( const Vector2F &other )
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	Vector2F &operator*=( float scalar )
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}

	float Length() const
	{
		return sqrtf( x * x + y * y );
	}

	void Normalize()
	{
		auto length = sqrtf( x * x + y * y );

		if( length < std::numeric_limits<float>::epsilon() )
		{
			return;
		}

		x /= length;
		y /= length;
	}

	float Dot( const Vector2F &other ) const
	{
		return ( x * other.x ) + ( y * other.y );
	}

	float DistanceFrom( const Vector2F &o ) const
	{
		return static_cast< float >( sqrt( ( x - o.x ) * ( x - o.x ) + ( y - o.y ) * ( y - o.y ) ) );
	}

	float x, y;
};