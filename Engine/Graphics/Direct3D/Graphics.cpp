// Includes
//=========

#include "../Graphics.h"

#include "Includes.h"
#include "../cConstantBuffer.h"
#include "../ConstantBufferFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Concurrency/cEvent.h>
#include <Engine/Time/Time.h>
#include <Engine/UserOutput/UserOutput.h>
#include <utility>
#include <map>
#include <vector>
#include "Engine/Math/cMatrix_transformation.h"

#include "../cGeometry.h"
#include "../cEffect.h"
#include "../GraphicDataHandler.h"
// Static Data Initialization
//===========================

namespace
{
	// Constant buffer object
	eae6320::Graphics::cConstantBuffer s_constantBuffer_frame(eae6320::Graphics::ConstantBufferTypes::Frame);
	eae6320::Graphics::cConstantBuffer s_constantBuffer_drawCall(eae6320::Graphics::ConstantBufferTypes::DrawCall);
	// Submission Data
	//----------------

	// This struct's data is populated at submission time;
	// it must cache whatever is necessary in order to render a frame
	struct sDataRequiredToRenderAFrame
	{
		eae6320::Graphics::ConstantBufferFormats::sFrame constantData_frame;
		std::vector<eae6320::Graphics::ConstantBufferFormats::sDrawCall> constantData_drawCallList;
		float backColor[4] = { 0,0,0,1.0f };
		std::vector<std::pair<eae6320::Graphics::cGeometry*, eae6320::Graphics::cEffect*>> s_RenderingGeometryList;
	};
	// In our class there will be two copies of the data required to render a frame:
	//	* One of them will be in the process of being populated by the data currently being submitted by the application loop thread
	//	* One of them will be fully populated and in the process of being rendered from in the render thread
	// (In other words, one is being produced while the other is being consumed)
	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
	auto* s_dataBeingSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
	auto* s_dataBeingRenderedByRenderThread = &s_dataRequiredToRenderAFrame[1];
	// The following two events work together to make sure that
	// the main/render thread and the application loop thread can work in parallel but stay in sync:
	// This event is signaled by the application loop thread when it has finished submitting render data for a frame
	// (the main/render thread waits for the signal)
	eae6320::Concurrency::cEvent s_whenAllDataHasBeenSubmittedFromApplicationThread;
	// This event is signaled by the main/render thread when it has swapped render data pointers.
	// This means that the renderer is now working with all the submitted data it needs to render the next frame,
	// and the application loop thread can start submitting data for the following frame
	// (the application loop thread waits for the signal)
	eae6320::Concurrency::cEvent s_whenDataForANewFrameCanBeSubmittedFromApplicationThread;


}

void eae6320::Graphics::SubmitElapsedTime(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_simulationTime)
{

	EAE6320_ASSERT(s_dataBeingSubmittedByApplicationThread);
	auto& constantData_frame = s_dataBeingSubmittedByApplicationThread->constantData_frame;
	constantData_frame.g_elapsedSecondCount_systemTime = i_elapsedSecondCount_systemTime;
	constantData_frame.g_elapsedSecondCount_simulationTime = i_elapsedSecondCount_simulationTime;
}

eae6320::cResult eae6320::Graphics::WaitUntilDataForANewFrameCanBeSubmitted(const unsigned int i_timeToWait_inMilliseconds)
{
	return Concurrency::WaitForEvent(s_whenDataForANewFrameCanBeSubmittedFromApplicationThread, i_timeToWait_inMilliseconds);
}

eae6320::cResult eae6320::Graphics::SignalThatAllDataForAFrameHasBeenSubmitted()
{
	return s_whenAllDataHasBeenSubmittedFromApplicationThread.Signal();
}

void eae6320::Graphics::SubmitBackColorDataFromApplicationThread(const float r, const float g, const float b, const float a)
{
	s_dataBeingSubmittedByApplicationThread->backColor[0] = r;
	s_dataBeingSubmittedByApplicationThread->backColor[1] = g;
	s_dataBeingSubmittedByApplicationThread->backColor[2] = b;
	s_dataBeingSubmittedByApplicationThread->backColor[3] = a;
}

void eae6320::Graphics::SubmitFrameRequiredMatrices(eae6320::Math::cMatrix_transformation i_worldToCamera, eae6320::Math::cMatrix_transformation i_cameraToProjected)
{
	auto& constantData_frame = s_dataBeingSubmittedByApplicationThread->constantData_frame;
	constantData_frame.g_transform_worldToCamera = i_worldToCamera;
	constantData_frame.g_transform_cameraToProjected = i_cameraToProjected;
}



void eae6320::Graphics::SubmitDrawCallRequiredMatrices(std::vector<eae6320::Math::cMatrix_transformation> i_localToWorldList)
{

	auto& constantData_drawCall = s_dataBeingSubmittedByApplicationThread->constantData_drawCallList;
	constantData_drawCall.clear();
	for (auto it = i_localToWorldList.begin(); it != i_localToWorldList.end(); ++it)
	{
		eae6320::Graphics::ConstantBufferFormats::sDrawCall tempDrawCall;
		tempDrawCall.g_transform_localToWorld = *it;
		constantData_drawCall.push_back(tempDrawCall);
	}

}

void eae6320::Graphics::UpdateGeometriesFromApplicationThread(std::vector<std::pair<Graphics::cGeometry*, Graphics::cEffect*>> renderingList)
{

	s_dataBeingSubmittedByApplicationThread->s_RenderingGeometryList = renderingList;
	for (auto it = s_dataBeingSubmittedByApplicationThread->s_RenderingGeometryList.begin(); it != s_dataBeingSubmittedByApplicationThread->s_RenderingGeometryList.end(); ++it)
	{
		(*it).first->IncrementReferenceCount();
		(*it).second->IncrementReferenceCount();
	}
}

void eae6320::Graphics::CreateDataFromApplicationThread(std::vector<std::pair<Graphics::cGeometry*, Graphics::cEffect*>> renderingList)
{
	s_dataBeingSubmittedByApplicationThread->s_RenderingGeometryList = renderingList;
	s_dataBeingRenderedByRenderThread->s_RenderingGeometryList = renderingList;

}
// Render
//-------

void eae6320::Graphics::RenderFrame()
{
	// Wait for the application loop to submit data to be rendered
	{
		const auto result = Concurrency::WaitForEvent(s_whenAllDataHasBeenSubmittedFromApplicationThread);
		if (result)
		{
			// Switch the render data pointers so that
			// the data that the application just submitted becomes the data that will now be rendered
			std::swap(s_dataBeingSubmittedByApplicationThread, s_dataBeingRenderedByRenderThread);

			// Once the pointers have been swapped the application loop can submit new data
			const auto result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Signal();
			if (!result)
			{
				EAE6320_ASSERTF(false, "Couldn't signal that new graphics data can be submitted");
				Logging::OutputError("Failed to signal that new render data can be submitted");
				UserOutput::Print("The renderer failed to signal to the application that new graphics data can be submitted."
					" The application is probably in a bad state and should be exited");
				return;
			}
		}
		else
		{
			EAE6320_ASSERTF(false, "Waiting for the graphics data to be submitted failed");
			Logging::OutputError("Waiting for the application loop to submit data to be rendered failed");
			UserOutput::Print("The renderer failed to wait for the application to submit data to be rendered."
				" The application is probably in a bad state and should be exited");
			return;
		}
	}


	EAE6320_ASSERT(s_dataBeingRenderedByRenderThread);
	eae6320::Graphics::DataHandler::SetClearColor(s_dataBeingRenderedByRenderThread->backColor[0], s_dataBeingRenderedByRenderThread->backColor[1], s_dataBeingRenderedByRenderThread->backColor[2], s_dataBeingRenderedByRenderThread->backColor[3]);
	eae6320::Graphics::DataHandler::ClearColor();
	// Update the frame constant buffer
	{
		// Copy the data from the system memory that the application owns to GPU memory
		auto& constantData_frame = s_dataBeingRenderedByRenderThread->constantData_frame;
		s_constantBuffer_frame.Update(&constantData_frame);


	}
	// Bind the shading data and draw geometry

	for (uint16_t i = 0; i < s_dataBeingRenderedByRenderThread->s_RenderingGeometryList.size(); ++i)
	{

		auto& constantData_drawCall = s_dataBeingRenderedByRenderThread->constantData_drawCallList;
		s_constantBuffer_drawCall.Update(&constantData_drawCall[i]);
		s_dataBeingRenderedByRenderThread->s_RenderingGeometryList[i].second->BindShadingData();
		s_dataBeingRenderedByRenderThread->s_RenderingGeometryList[i].first->DrawGeometry();

	}

	eae6320::Graphics::DataHandler::SwapBuffer();
	// Once everything has been drawn the data that was submitted for this frame
	// should be cleaned up and cleared.

	// Decrease the reference since the render thread has finish rendering for this frame
	for (auto it = s_dataBeingRenderedByRenderThread->s_RenderingGeometryList.begin(); it != s_dataBeingRenderedByRenderThread->s_RenderingGeometryList.end(); ++it)
	{
		(*it).first->DecrementReferenceCount();
		(*it).second->DecrementReferenceCount();
	}
	// so that the struct can be re-used (i.e. so that data for a new frame can be submitted to it)
	{
		// (At this point in the class there isn't anything that needs to be cleaned up)
	}
}



// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::Initialize(const sInitializationParameters& i_initializationParameters)
{
	auto result = Results::Success;
	result = eae6320::Graphics::DataHandler::InitializeGlobalData(i_initializationParameters, s_constantBuffer_frame, s_constantBuffer_drawCall, s_whenAllDataHasBeenSubmittedFromApplicationThread, s_whenDataForANewFrameCanBeSubmittedFromApplicationThread);

	return result;
}

eae6320::cResult eae6320::Graphics::CleanUp()
{
	auto result = Results::Success;
	for (auto it = s_dataBeingRenderedByRenderThread->s_RenderingGeometryList.begin(); it != s_dataBeingRenderedByRenderThread->s_RenderingGeometryList.end(); ++it) {
		// Delete effect reference
		(*it).second->DecrementReferenceCount();
		(*it).second = nullptr;
	}
	{
		const auto result_constantBuffer_frame = s_constantBuffer_frame.CleanUp();
		if (!result_constantBuffer_frame)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_constantBuffer_frame;
			}
		}
	}
	{
		const auto result_constantBuffer_drawCall = s_constantBuffer_drawCall.CleanUp();
		if (!result_constantBuffer_drawCall)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_constantBuffer_drawCall;
			}
		}
	}
	for (auto it = s_dataBeingRenderedByRenderThread->s_RenderingGeometryList.begin(); it != s_dataBeingRenderedByRenderThread->s_RenderingGeometryList.end(); ++it)
	{
		// Delete geometry reference
		(*it).first->DecrementReferenceCount();
		(*it).first = nullptr;
	}

	eae6320::Graphics::DataHandler::CleanUpGlobalData();

	return result;
}
