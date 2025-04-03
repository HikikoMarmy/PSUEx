#pragma once

#include <vector>

namespace PSUHook
{
	typedef void ( *MemoryPatchFunction )( void );

	class PatchRegistry
	{
	public:
		PatchRegistry() = default;
		virtual ~PatchRegistry() = default;
		virtual void ApplyPatch() = 0;

		static bool ApplyAll()
		{
			try
			{
				for( const auto &patch : get_patch_list() )
				{
					patch();
				}
			}
			catch( std::runtime_error e )
			{
				printf( "Failed to apply patches.\n" );
				printf( "Error: %s\n", e.what() );
				return false;
			}

			return true;
		}

		static bool RegisterPatch( const MemoryPatchFunction _patch )
		{
			get_patch_list().push_back( _patch );
			return true;
		}

	private:
		static std::vector< MemoryPatchFunction > &get_patch_list()
		{
			static std::vector< MemoryPatchFunction > patched;
			return patched;
		}
	};

	// Register a patch
	#define REGISTER_PATCH( _patch ) static const bool _registered = PatchRegistry::RegisterPatch( &_patch );
}


// Include all patches here
#include "Patch/ResolutionPatch.hpp"
#include "Patch/MinimapPatch.hpp"
#include "Patch/NoVideoPatch.hpp"
#include "Patch/FastStartPatch.hpp"
#include "Patch/ProfilerPatch.hpp"
#include "Patch/FastGrindingPatch.hpp"
#include "Patch/TextSmoothingPatch.hpp"
#include "Patch/V1TextureFilter.hpp"
#include "Patch/ViewDistancePatch.hpp"