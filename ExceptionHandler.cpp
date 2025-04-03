
#include "ExceptionHandler.h"

#pragma comment(lib, "Dbghelp.lib")

// Create a timestamped filename
std::string GetCrashDumpFilename()
{
    SYSTEMTIME st;
    GetLocalTime( &st );

    std::ostringstream oss;
    oss << "Crash_"
        << std::setfill( '0' ) << std::setw( 4 ) << st.wYear
        << std::setw( 2 ) << st.wMonth
        << std::setw( 2 ) << st.wDay << "_"
        << std::setw( 2 ) << st.wHour
        << std::setw( 2 ) << st.wMinute
        << std::setw( 2 ) << st.wSecond << ".dmp";

    return oss.str();
}

// Write the crash dump file
bool WriteMinidump( EXCEPTION_POINTERS *pExceptionPointers, const std::string &dumpPath )
{
    HANDLE hFile = CreateFileA( dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
    if( hFile == INVALID_HANDLE_VALUE )
        return false;

    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = pExceptionPointers;
    mdei.ClientPointers = FALSE;

    BOOL success = MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpWithDataSegs,
        &mdei,
        nullptr,
        nullptr
    );

    CloseHandle( hFile );
    return success;
}

// The crash handler function
LONG WINAPI GlobalCrashHandler( EXCEPTION_POINTERS *pExceptionPointers )
{
    int response = MessageBoxA( nullptr,
                                "Oops! PSUEx detected a crash in the game.\n\nWould you like to save a crash report to help us figure out what went wrong?",
                                "Game Crash Detected",
                                MB_YESNO | MB_ICONERROR | MB_SYSTEMMODAL );

    if( response == IDYES )
    {
        char exePath[ MAX_PATH ];
        GetModuleFileNameA( nullptr, exePath, MAX_PATH );
        std::filesystem::path dumpDir = std::filesystem::path( exePath ).parent_path();
        std::string dumpName = GetCrashDumpFilename();
        std::string dumpPath = ( dumpDir / dumpName ).string();

        if( WriteMinidump( pExceptionPointers, dumpPath ) )
        {
            std::string msg = "Crash report saved to:\n" + dumpPath;
            MessageBoxA( nullptr, msg.c_str(), "Crash Report Saved", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL );

            // Open the folder where the dump was saved
            ShellExecuteA( nullptr, "open", dumpDir.string().c_str(), nullptr, nullptr, SW_SHOWDEFAULT );
        }
        else
        {
            MessageBoxA( nullptr, "Failed to save crash report.", "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL );
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

// Call this during DLL initialization
void SetupCrashHandler()
{
    SetUnhandledExceptionFilter( GlobalCrashHandler );
}