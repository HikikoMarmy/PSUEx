#pragma once

#include <limits>
#include <cmath>

class Vector3F
{
public:

	Vector3F( float x, float y, float z )
		: x( x ), y( y ), z( z )
	{
	}

	Vector3F()
		: x( 0.0f ), y( 0.0f ), z( 0.0f )
	{
	}

	Vector3F( const Vector3F &other )
		: x( other.x ), y( other.y ), z( other.z )
	{
	}

	~Vector3F()
	{
	}

	Vector3F &operator=( const Vector3F &other )
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	Vector3F operator+( const Vector3F &other ) const
	{
		return Vector3F( x + other.x, y + other.y, z + other.z );
	}

	Vector3F operator-( const Vector3F &other ) const
	{
		return Vector3F( x - other.x, y - other.y, z - other.z );
	}

	Vector3F operator*( float scalar ) const
	{
		return Vector3F( x * scalar, y * scalar, z * scalar );
	}

	Vector3F operator/( float scalar ) const
	{
		return Vector3F( x / scalar, y / scalar, z / scalar );
	}

	Vector3F &operator+=( const Vector3F &other )
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vector3F &operator-=( const Vector3F &other )
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vector3F &operator*=( float scalar )
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	float Length() const
	{
		return sqrtf( x * x + y * y + z * z );
	}

	void Normalize()
	{
		auto length = sqrtf( x * x + y * y + z * z );

		if( length < std::numeric_limits<float>::epsilon() )
		{
			return;
		}

		x /= length;
		y /= length;
		z /= length;
	}

	float Dot( const Vector3F &other ) const
	{
		return ( x * other.x ) + ( y * other.y ) + ( z * other.z );
	}

	float DistanceFrom( const Vector3F &o ) const
	{
		return static_cast< float >( sqrt( ( x - o.x ) * ( x - o.x ) + ( z - o.z ) * ( z - o.z ) + ( y - o.y ) * ( y - o.y ) ) );
	}

	float DistanceFrom2D( const Vector3F &o ) const
	{
		float dx = x - o.x;
		float dz = z - o.z;
		return sqrtf( dx * dx + dz * dz );
	}

	float GetFacingAngle( const Vector3F &other ) const
	{
		float dx = other.x - x;
		float dz = other.z - z;
		return atan2f( dz, dx );
	}

	Vector3F ForwardVectorTo( const Vector3F &targetPosition ) const
	{
		Vector3F forward = targetPosition - *this;
		forward.Normalize();
		return forward;
	}

	float x, y, z;
};