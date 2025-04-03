#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <array>
#include <span>
#include <cstddef>
#include <psapi.h>

class PSUMemory {
private:
    static inline uintptr_t baseAddress;

public:
    static PSUMemory &Get()
    {
        static PSUMemory instance;
        return instance;
    }

    PSUMemory()
    {
        baseAddress = ( uintptr_t )GetMainModuleBaseAddress();
    }

    PSUMemory( const PSUMemory & ) = delete;
    PSUMemory &operator=( const PSUMemory & ) = delete;

    template<typename... Ts>
    static std::array<std::byte, sizeof...( Ts )> MakeByteArray( Ts&&... args )
    {
        return { std::byte( std::forward<Ts>( args ) )... };
    }

    template <typename T>
    static T ReadMemory( uintptr_t address )
    {
        T *ptr = reinterpret_cast< T * >( baseAddress + address );
        if( !IsValidPointer( ptr ) )
        {
            printf( "Invalid Pointer Read: 0x%08X\n", static_cast< unsigned int >( baseAddress + address ) );
            return T{};
        }
        return *ptr;
    }

    template <typename T>
    static T ReadAbsoluteMemory( uintptr_t address )
    {
        T *ptr = reinterpret_cast< T * >( address );
        if( !IsValidPointer( ptr ) )
        {
            printf( "Invalid Absolute Pointer Read: 0x%08X\n", static_cast< unsigned int >( address ) );
            return T{};
        }
        return *ptr;
    }

    template <typename T = void>
    static T *ReadPtr( uintptr_t address )
    {
        return reinterpret_cast< T * >( baseAddress + address );
    }

    template <typename T = void>
    static T *ReadAbsolutePtr( uintptr_t address )
    {
        return reinterpret_cast< T * >( address );
    }

    template <typename T>
    static void WriteMemory( uintptr_t address, T value )
    {
        T *ptr = reinterpret_cast< T * >( baseAddress + address );
        if( IsValidPointer( ptr ) )
        {
            *ptr = value;
        }
        else
        {
            printf( "Invalid Pointer Write: 0x%08X\n", static_cast< unsigned int >( baseAddress + address ) );
        }
    }

    static void WriteMemory( uintptr_t address, std::span<const uint8_t> value )
    {
        uint8_t *ptr = reinterpret_cast< uint8_t * >( baseAddress + address );
        if( IsValidPointer( ptr ) )
        {
            std::copy( value.begin(), value.end(), ptr );
        }
        else
        {
            printf( "Invalid Memory Block Write: 0x%08X\n", static_cast< unsigned int >( baseAddress + address ) );
        }
    }

    template <typename T>
    static void WriteAbsoluteMemory( uintptr_t address, T value )
    {
        T *ptr = reinterpret_cast< T * >( address );
        if( IsValidPointer( ptr ) )
        {
            *ptr = value;
        }
        else
        {
            printf( "Invalid Absolute Pointer Write: 0x%08X\n", static_cast< unsigned int >( address ) );
        }
    }

    static void WriteAbsoluteMemory( uintptr_t address, std::span<const uint8_t> value )
    {
        uint8_t *ptr = reinterpret_cast< uint8_t * >( address );
        if( IsValidPointer( ptr ) )
        {
            std::copy( value.begin(), value.end(), ptr );
        }
        else
        {
            printf( "Invalid Absolute Memory Block Write: 0x%08X\n", static_cast< unsigned int >( address ) );
        }
    }

private:
    static bool IsValidPointer( const void *ptr )
    {
        MEMORY_BASIC_INFORMATION mbi;
        if( VirtualQuery( ptr, &mbi, sizeof( mbi ) ) == 0 )
        {
            return false;
        }

        if( mbi.State != MEM_COMMIT )
        {
            return false;
        }

        DWORD protect = mbi.Protect;
        if( protect & ( PAGE_NOACCESS | PAGE_GUARD ) )
        {
            return false;
        }

        return true;
    }

    uintptr_t GetMainModuleBaseAddress()
    {
        HMODULE hMods[ 1024 ];
        HANDLE hProcess = GetCurrentProcess();
        DWORD cbNeeded;

        if( EnumProcessModules( hProcess, hMods, sizeof( hMods ), &cbNeeded ) )
        {
            // The first module in the array is the base address of the main module (the game itself)
            uintptr_t baseAddress = reinterpret_cast< uintptr_t >( hMods[ 0 ] );
            printf( "Detected Base Address: 0x%08X\n", static_cast< unsigned int >( baseAddress ) );
            return baseAddress;
        }
        else
        {
            printf( "Failed to enumerate process modules\n" );
            return 0;
        }
    }
};

template<typename T>
class MemoryValue {
private:
    uintptr_t baseAddress;
    uintptr_t offset;

    bool IsValidPointer( T *ptr ) const
    {
        MEMORY_BASIC_INFORMATION mbi{};
        if( VirtualQuery( ptr, &mbi, sizeof( mbi ) ) )
        {
            return ( mbi.State == MEM_COMMIT ) && !( mbi.Protect & PAGE_NOACCESS );
        }
        return false;
    }

public:
    // Default constructor
    MemoryValue() : baseAddress( 0 ), offset( 0 )
    {
    }

    // Initialization method to provide base address
    template<typename C>
    void Init( C *baseObj, uintptr_t baseAddr, MemoryValue<T> C:: *member )
    {
        baseAddress = baseAddr;
        offset = reinterpret_cast< uintptr_t >( &( baseObj->*member ) ) - reinterpret_cast< uintptr_t >( baseObj );
    }

    // Assignment operator (write to memory)
    MemoryValue &operator=( const T &value )
    {
        T *ptr = reinterpret_cast< T * >( baseAddress + offset );
        if( IsValidPointer( ptr ) )
        {
            *ptr = value;
        }
        return *this;
    }

    // Type cast operator (read from memory)
    operator T() const
    {
        T *ptr = reinterpret_cast< T * >( baseAddress + offset );
        if( IsValidPointer( ptr ) )
        {
            return *ptr;
        }
        return T(); // Return default value if invalid
    }
};