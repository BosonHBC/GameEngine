#include <string>
#include "cEffect.h"
#include "Engine/Results/Results.h"
#include "Engine/ScopeGuard/cScopeGuard.h"
#include "GraphicDataHandler.h"
#include "cShader.h"

namespace eae6320 {
	namespace Graphics {

		eae6320::cResult cEffect::InitializeShadingData(const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const uint8_t& i_renderState)
		{
			auto result = Results::Success;

			if (!DataHandler::s_renderState.IsValid())
				if (!(result = cRenderState::s_manager.Load(i_renderState, DataHandler::s_renderState)))
				{
					EAE6320_ASSERTF(false, "Can't initialize shading data without render state");
				}

			if (!(result = cShader::s_manager.Load(i_vertexShaderPath,
				s_vertexShader, ShaderTypes::Vertex)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without vertex shader");
			}
			if (!(result = cShader::s_manager.Load(i_fragmentShaderPath,
				s_fragmentShader, ShaderTypes::Fragment)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without fragment shader");
			}

			if (result)
				CreateProgram();

			return result;
		}


		void cEffect::BindShadingData()
		{

			{
				EAE6320_ASSERT(m_programID != 0);
				glUseProgram(m_programID);
				EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
			}
			// Render state
			{
				EAE6320_ASSERT(eae6320::Graphics::DataHandler::s_renderState);
				auto* const renderState = cRenderState::s_manager.Get(eae6320::Graphics::DataHandler::s_renderState);
				EAE6320_ASSERT(renderState);
				renderState->Bind();
			}
		}

		eae6320::cResult cEffect::CleanUp()
		{
			auto result = Results::Success;
			if ((m_programID) != 0)
			{
				glDeleteProgram(m_programID);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					if (result)
					{
						result = eae6320::Results::Failure;
					}
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
				}
				m_programID = 0;
			}

			if (s_vertexShader)
			{
				const auto result_vertexShader = cShader::s_manager.Release(s_vertexShader);
				if (!result_vertexShader)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_vertexShader;
					}
				}
			}
			if (s_fragmentShader)
			{
				const auto result_fragmentShader = cShader::s_manager.Release(s_fragmentShader);
				if (!result_fragmentShader)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_fragmentShader;
					}
				}
			}


			return result;
		}
		eae6320::cResult cEffect::CreateProgram()
		{
			auto result = Results::Success;
			unsigned int m_tempID;
			eae6320::cScopeGuard scopeGuard_program([&result, &m_tempID]
				{
					if (!result)
					{
						if (m_tempID != 0)
						{
							glDeleteProgram(m_tempID);
							const auto errorCode = glGetError();
							if (errorCode != GL_NO_ERROR)
							{
								EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
								eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
									reinterpret_cast<const char*>(gluErrorString(errorCode)));
							}
							m_tempID = 0;
						}
					}
				});
			m_programID = m_tempID;
			{
				m_programID = glCreateProgram();
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to create a program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
				else if (m_programID == 0)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERT(false);
					eae6320::Logging::OutputError("OpenGL failed to create a program");
					return result;
				}
			}
			// Attach the shaders to the program
			{
				// Vertex
				{
					glAttachShader(m_programID, eae6320::Graphics::cShader::s_manager.Get(s_vertexShader)->m_shaderId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to attach the vertex shader to the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				// Fragment
				{
					glAttachShader(m_programID, eae6320::Graphics::cShader::s_manager.Get(s_fragmentShader)->m_shaderId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to attach the fragment shader to the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
			}
			// Link the program
			{
				glLinkProgram(m_programID);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					// Get link info
					// (this won't be used unless linking fails
					// but it can be useful to look at when debugging)
					std::string linkInfo;
					{
						GLint infoSize;
						glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &infoSize);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							struct sLogInfo
							{
								GLchar* memory;
								sLogInfo(const size_t i_size) { memory = reinterpret_cast<GLchar*>(malloc(i_size)); }
								~sLogInfo() { if (memory) free(memory); }
							} info(static_cast<size_t>(infoSize));
							constexpr GLsizei* const dontReturnLength = nullptr;
							glGetProgramInfoLog(m_programID, static_cast<GLsizei>(infoSize), dontReturnLength, info.memory);
							const auto errorCode = glGetError();
							if (errorCode == GL_NO_ERROR)
							{
								linkInfo = info.memory;
							}
							else
							{
								result = eae6320::Results::Failure;
								EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
								eae6320::Logging::OutputError("OpenGL failed to get link info of the program: %s",
									reinterpret_cast<const char*>(gluErrorString(errorCode)));
								return result;
							}
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to get the length of the program link info: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					// Check to see if there were link errors
					GLint didLinkingSucceed;
					{
						glGetProgramiv(m_programID, GL_LINK_STATUS, &didLinkingSucceed);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							if (didLinkingSucceed == GL_FALSE)
							{
								result = eae6320::Results::Failure;
								EAE6320_ASSERTF(false, linkInfo.c_str());
								eae6320::Logging::OutputError("The program failed to link: %s",
									linkInfo.c_str());
								return result;
							}
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to find out if linking of the program succeeded: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to link the program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			return result;
		}
	}
}