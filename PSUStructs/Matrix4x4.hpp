#pragma once

#include <array>
#include <iostream>

class Matrix4x4 {
public:
    // Matrix data stored in row-major order (for DirectX style)
    float m[ 4 ][ 4 ];

    Matrix4x4()
    {
        // Initialize to identity matrix
        Identity();
    }

    // Initialize matrix to Identity
    void Identity()
    {
        for( int row = 0; row < 4; ++row )
        {
            for( int col = 0; col < 4; ++col )
            {
                m[ row ][ col ] = ( row == col ) ? 1.0f : 0.0f;
            }
        }
    }

    // Multiply matrix with another Matrix4x4
    Matrix4x4 operator*( const Matrix4x4 &other ) const
    {
        Matrix4x4 result;
        for( int row = 0; row < 4; ++row )
        {
            for( int col = 0; col < 4; ++col )
            {
                result.m[ row ][ col ] = 0.0f;
                for( int k = 0; k < 4; ++k )
                {
                    result.m[ row ][ col ] += m[ row ][ k ] * other.m[ k ][ col ];
                }
            }
        }
        return result;
    }

    // Multiply matrix with a 3D vector (Assuming W = 1.0)
    Vector3F Multiply( const Vector3F &vec ) const
    {
        float x = vec.x * m[ 0 ][ 0 ] + vec.y * m[ 1 ][ 0 ] + vec.z * m[ 2 ][ 0 ] + m[ 3 ][ 0 ];
        float y = vec.x * m[ 0 ][ 1 ] + vec.y * m[ 1 ][ 1 ] + vec.z * m[ 2 ][ 1 ] + m[ 3 ][ 1 ];
        float z = vec.x * m[ 0 ][ 2 ] + vec.y * m[ 1 ][ 2 ] + vec.z * m[ 2 ][ 2 ] + m[ 3 ][ 2 ];
        float w = vec.x * m[ 0 ][ 3 ] + vec.y * m[ 1 ][ 3 ] + vec.z * m[ 2 ][ 3 ] + m[ 3 ][ 3 ];

        if( w != 0.0f )
        {
            x /= w;
            y /= w;
            z /= w;
        }

        return Vector3F( x, y, z );
    }
};
