/*
	The two functions in this file are how Maya will interact with the plug-in
*/

// Includes
//=========

#include "cMayaGeometryExporter.h"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MString.h>

// Static Data Initialization
//===========================

namespace
{
	// This will be displayed in Maya's dropdown list of available export formats
	constexpr auto* const s_pluginName = "BOSON HUANG's EAE6320 Geometry Format"
#ifdef _DEBUG
		" -- DEBUG"
#endif
	;
}

// Entry Point
//============

__declspec(dllexport) MStatus initializePlugin( MObject io_object )
{
	// Create a plugin function set
	MFnPlugin plugin( io_object );

	// Register the exporter
	MStatus status;
	{
		constexpr auto* const noIcon = "none";
		status = plugin.registerFileTranslator( s_pluginName, noIcon,
			// This function is what Maya should call to create a new instance of the geometry exporter
			eae6320::cMayaGeometryExporter::Create );
		if ( !status )
		{
			MGlobal::displayError( MString( "Failed to register geometry exporter: " ) + status.errorString() );
		}
	}
    return status;
}

__declspec(dllexport) MStatus uninitializePlugin( MObject io_object )
{
	// Create a plugin function set
	MFnPlugin plugin( io_object );

	// Unregister the exporter
	MStatus status;
	{
		status = plugin.deregisterFileTranslator( s_pluginName );
		if ( !status )
		{
			MGlobal::displayError( MString( "Failed to deregister geometry exporter: " ) + status.errorString() );
		}
	}
    return status;
}
