#pragma once

#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <filesystem>
#include <sstream>
#include <iomanip>

std::string GetCrashDumpFilename();
bool WriteMinidump( EXCEPTION_POINTERS *pExceptionPointers, const std::string &dumpPath );
LONG WINAPI GlobalCrashHandler( EXCEPTION_POINTERS *pExceptionPointers );
void SetupCrashHandler();