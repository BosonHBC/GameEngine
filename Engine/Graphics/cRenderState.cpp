// Includes
//=========

#include "cRenderState.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <new>

// Static Data Initialization
//===========================

eae6320::Assets::cManager<eae6320::Graphics::cRenderState, uint8_t> eae6320::Graphics::cRenderState::s_manager;

// Interface
//==========

// Access
//-------

bool eae6320::Graphics::cRenderState::IsAlphaTransparencyEnabled() const
{
	return RenderStates::IsAlphaTransparencyEnabled( m_bits );
}

bool eae6320::Graphics::cRenderState::IsDepthTestingEnabled() const
{
	return RenderStates::IsDepthTestingEnabled( m_bits );
}

bool eae6320::Graphics::cRenderState::IsDepthWritingEnabled() const
{
	return RenderStates::IsDepthWritingEnabled( m_bits );
}

bool eae6320::Graphics::cRenderState::ShouldBothTriangleSidesBeDrawn() const
{
	return RenderStates::ShouldBothTriangleSidesBeDrawn( m_bits );
}

uint8_t eae6320::Graphics::cRenderState::GetRenderStateBits() const
{
	return m_bits;
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cRenderState::Load( const uint8_t i_renderStateBits, cRenderState*& o_renderState )
{
	auto result = Results::Success;

	EAE6320_ASSERT( o_renderState == nullptr );

	cRenderState* newRenderState = nullptr;
	cScopeGuard scopeGuard( [&o_renderState, &result, &newRenderState]
		{
			if ( result )
			{
				EAE6320_ASSERT( newRenderState != nullptr );
				o_renderState = newRenderState;
			}
			else
			{
				if ( newRenderState )
				{
					newRenderState->DecrementReferenceCount();
					newRenderState = nullptr;
				}
				o_renderState = nullptr;
			}
		} );

	// Allocate a new render state
	{
		newRenderState = new (std::nothrow) cRenderState( i_renderStateBits );
		if ( !newRenderState )
		{
			result = Results::OutOfMemory;
			EAE6320_ASSERTF( false, "Couldn't allocate memory for a render state" );
			Logging::OutputError( "Failed to allocate memory for a render state" );
			return result;
		}
	}
	// Initialize the platform-specific render state
	if ( !( result = newRenderState->Initialize() ) )
	{
		EAE6320_ASSERTF( false, "Initialization of new render state failed" );
		return result;
	}

	return result;
}

// Implementation
//===============

// Initialization / Clean Up
//--------------------------

eae6320::Graphics::cRenderState::cRenderState( const uint8_t i_renderStateBits )
	:
	m_bits( i_renderStateBits )
{

}

eae6320::Graphics::cRenderState::~cRenderState()
{
	EAE6320_ASSERT( m_referenceCount == 0 );
	const auto result = CleanUp();
	EAE6320_ASSERT( result );
}
