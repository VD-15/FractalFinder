#include "Game.hpp"

#include "ValkyrieEngineCommon/Content.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glad/glad.h"
#include <chrono>
#include <stdexcept>
#include <iostream>
#include <algorithm>

using namespace game;

constexpr float defaultZoom = 2.f;

void LoadShader(const std::string& path, const std::string& alias)
{
	if (!Content<GLSLFile>::LoadContent(path, alias))
	{
		throw std::runtime_error("Failed to load shader: " + path);
	}
}

void CheckProgramError(UInt program)
{
	Int success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		Int logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		char* infoLog = new char[logLength];
		glGetProgramInfoLog(program, logLength, &logLength, infoLog);
		std::cout << "Failed to compile shader program:\n" << infoLog << std::endl;
		delete[] infoLog;
		throw std::runtime_error("Failed to compile shader program.");
	}
}

void CheckShaderError(UInt shader)
{
	Int success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		Int logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		char* infoLog = new char[logLength];
		glGetShaderInfoLog(shader, logLength, &logLength, infoLog);
		std::cout << "Failed to compile shader:\n" << infoLog << std::endl;
		delete[] infoLog;
		throw std::runtime_error("Failed to compile shader.");
	}
}

UInt CreateComputeProgram(const std::string& source)
{
	const char* cstr = Content<GLSLFile>::GetContent(source)->data.c_str();
	
	UInt program = glCreateShaderProgramv(GL_COMPUTE_SHADER,1, &cstr);
	CheckProgramError(program);

	return program;
}

UInt CreateShader(const std::string& source, UInt usage)
{
	const GLSLFile* glsl = Content<GLSLFile>::GetContent(source);
	auto cstr = glsl->data.c_str();
	UInt shader = glCreateShader(usage);
	glShaderSource(shader, 1, &cstr, nullptr);
	glCompileShader(shader);
	CheckShaderError(shader);
	return shader;
}

UInt CreateGraphicsProgram(const std::string& vertexSource, const std::string& fragmentSource)
{
	UInt vertexShader = CreateShader(vertexSource, GL_VERTEX_SHADER);
	UInt fragmentShader = CreateShader(fragmentSource, GL_FRAGMENT_SHADER);

	UInt program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	CheckProgramError(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

Game::Game(Window* _window) :
	window(_window)
{
	Content<GLSLFile>::SetContentPrefix("res/");
	LoadShader("mandelbrot.glsl", "mandelbrot");
	LoadShader("julia0.glsl", "julia0");
	LoadShader("julia1.glsl", "julia1");
	LoadShader("julia2.glsl", "julia2");
	LoadShader("tricorn.glsl", "tricorn");
	LoadShader("burning.glsl", "burning");
	LoadShader("vertex.glsl", "vertex");
	LoadShader("fragment.glsl", "fragment");
	mandelProgram = CreateComputeProgram("mandelbrot");
	juliaProgram0 = CreateComputeProgram("julia0");
	juliaProgram1 = CreateComputeProgram("julia1");
	juliaProgram2 = CreateComputeProgram("julia2");
	tricornProgram = CreateComputeProgram("tricorn");
	burningProgram = CreateComputeProgram("burning");
	quadProgram = CreateGraphicsProgram("vertex", "fragment");

	auto windowSize = window->GetSize();
	fullSize = Vector2(windowSize[0], windowSize[1]);
	viewSize = Vector2(fullSize[0] * 0.8f, fullSize[1]);

	previewSize = Vector2(fullSize[0] - viewSize[0], viewSize[1] / 4.f);

	glGenTextures(1, &fractalOutput);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, fractalOutput);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA32F, 
		viewSize[0], 
		viewSize[1], 
		0, 
		GL_RGBA, 
		GL_FLOAT, 
		nullptr);
	glBindImageTexture(0, fractalOutput, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glGenTextures(4, previewTextures);
	for (UInt i = 0; i < 4; i++)
	{
		glActiveTexture(GL_TEXTURE1 + i);
		glBindTexture(GL_TEXTURE_2D, previewTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			GL_RGBA32F, 
			previewSize[0], 
			previewSize[1], 
			0, 
			GL_RGBA, 
			GL_FLOAT, 
			nullptr);
		glBindImageTexture(
			i + 1,
			previewTextures[i], 
			0, 
			false, 
			0, 
			GL_WRITE_ONLY, 
			GL_RGBA32F);
	}

	int width, height;
	unsigned char* endScreen = stbi_load("res/endscreen.png", &width, &height, nullptr, 4);

	glGenTextures(1, &endTexture);
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, endTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		endScreen);
	glBindImageTexture(
		5,
		endTexture, 
		0, 
		false, 
		0, 
		GL_READ_ONLY, 
		GL_RGBA);

	stbi_image_free(endScreen);

	struct Vertex { Vector2 pos; Vector2 uv; };
	
	/*Vertex vertices[3]
	{
		{ Vector2(0.f, 0.5f) },
		{ Vector2(-0.5f, -0.5f) },
		{ Vector2(0.5f, -0.5f) }
	};*/
	
	Vertex vertices[36]
	{
		{ Vector2(0.f,         0.f),         Vector2(0.f, 1.f) },
		{ Vector2(0.f,         viewSize[1]), Vector2(0.f, 0.f) },
		{ Vector2(viewSize[0], 0.f),         Vector2(1.f, 1.f) },

		{ Vector2(0.f,         viewSize[1]), Vector2(0.f, 0.f) },
		{ Vector2(viewSize[0], 0.f),         Vector2(1.f, 1.f) },
		{ Vector2(viewSize[0], viewSize[1]), Vector2(1.f, 0.f) },

		{ Vector2(viewSize[0], previewSize[1] * 0.f), Vector2(0.f, 1.f) },
		{ Vector2(viewSize[0], previewSize[1] * 1.f), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], previewSize[1] * 0.f), Vector2(1.f, 1.f) },

		{ Vector2(viewSize[0], previewSize[1] * 1.f), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], previewSize[1] * 0.f), Vector2(1.f, 1.f) },
		{ Vector2(fullSize[0], previewSize[1] * 1.f), Vector2(1.f, 0.f) },

		{ Vector2(viewSize[0], previewSize[1] * 1.f), Vector2(0.f, 1.f) },
		{ Vector2(viewSize[0], previewSize[1] * 2.f), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], previewSize[1] * 1.f), Vector2(1.f, 1.f) },

		{ Vector2(viewSize[0], previewSize[1] * 2.f), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], previewSize[1] * 1.f), Vector2(1.f, 1.f) },
		{ Vector2(fullSize[0], previewSize[1] * 2.f), Vector2(1.f, 0.f) },

		{ Vector2(viewSize[0], previewSize[1] * 2.f), Vector2(0.f, 1.f) },
		{ Vector2(viewSize[0], previewSize[1] * 3.f), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], previewSize[1] * 2.f), Vector2(1.f, 1.f) },

		{ Vector2(viewSize[0], previewSize[1] * 3.f), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], previewSize[1] * 2.f), Vector2(1.f, 1.f) },
		{ Vector2(fullSize[0], previewSize[1] * 3.f), Vector2(1.f, 0.f) },

		{ Vector2(viewSize[0], previewSize[1] * 3.f), Vector2(0.f, 1.f) },
		{ Vector2(viewSize[0], previewSize[1] * 4.f), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], previewSize[1] * 3.f), Vector2(1.f, 1.f) },

		{ Vector2(viewSize[0], previewSize[1] * 4.f), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], previewSize[1] * 3.f), Vector2(1.f, 1.f) },
		{ Vector2(fullSize[0], previewSize[1] * 4.f), Vector2(1.f, 0.f) },

		{ Vector2(0.f,         0.f),         Vector2(0.f, 1.f) },
		{ Vector2(0.f,         fullSize[1]), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], 0.f),         Vector2(1.f, 1.f) },

		{ Vector2(0.f,         fullSize[1]), Vector2(0.f, 0.f) },
		{ Vector2(fullSize[0], 0.f),         Vector2(1.f, 1.f) },
		{ Vector2(fullSize[0], fullSize[1]), Vector2(1.f, 0.f) },
	};

	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	UInt vertexBuffer = 0;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);
	glEnableVertexAttribArray(1);
	glUseProgram(quadProgram);

	glGenVertexArrays(1, &computeVAO);
	glBindVertexArray(computeVAO);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glUseProgram(mandelProgram);

	zoomValue = defaultZoom;

	glClearColor(0.f, 0.f, 0.f, 0.f);
	numIterations = 0;

	levels[0] =
	{
		mandelProgram,
		{
			0.00539102f,
			0.00485192f,
			0.0556257f,
			0.0215505f,
		},
		{
			Vector2(-0.56226f, -0.642735f),
			Vector2(-0.1283f, -0.988242f),
			Vector2(-0.0584823f, 0.660361f),
			Vector2(-0.862101f, -0.258372),
		
		}
	};

	levels[1] =
	{
		tricornProgram,
		{
			0.00676278f,
			0.00927678f,
			0.00547786f,
			0.0500631f,
		},
		{
			Vector2(0.743174f, -0.930051f),
			Vector2(-1.47725f, 0.0f),
			Vector2(-1.20453f, -0.079302f),
			Vector2(0.228252f, -0.529966f),
		}
	};

	levels[2] =
	{
		burningProgram,
		{
			0.00154711f,
			0.000739977f,
			0.00212224f,
			0.0114528f,
		},
		{
			Vector2(0.970566f, -1.68122f),
			Vector2(-1.57553f, -0.0369697f),
			Vector2(-1.86087f, -0.000532295f),
			Vector2(-0.969854f, -0.989513f),
		}
	};

	levels[3] =
	{
		juliaProgram1,
		{
			0.14358f,
			0.0556257f,
			0.0405511f,
			0.0405511f,
		},
		{
			Vector2(0.523753f, -0.188956f),
			Vector2(-0.509669f, -0.0752877f),
			Vector2(-0.075375f, 0.584517f),
			Vector2(0.225403f, 1.02556f),
		}
	};

	levels[4] =
	{
		juliaProgram2,
		{
			0.19222f,
			0.0405511f,
			0.0157103f,
			0.0618063f,
		},
		{
			Vector2(0.744528f, -0.276319f),
			Vector2(-1.08141f, -0.45105f),
			Vector2(0.282748f, 0.680276f),
			Vector2(-0.689748f, -0.255823f),
		}
	};

	levels[5] = 
	{
		juliaProgram0,
		{
			0.0295617f,
			0.076304f,
			0.0399335f,
			0.0215505
		},
		{
			Vector2(-0.551516f, 0.108391f),
			Vector2(0.907088f, -0.284514f),
			Vector2(-0.0433468f, 0.818537f),
			Vector2(0.0308237f, -0.0408021f)
		}
	};
	
	levels[6] =
	{
		mandelProgram,
		{
			0.00834911f,
			0.000485498f,
			0.000232213f,
			0.00323461f,
		},
		{
			Vector2(0.3187f, -0.0321924f),
			Vector2(-1.76648f, -0.0417347f),
			Vector2(-1.02001f, 0.367522f),
			Vector2(-0.398024f, -0.681524f),
		
		}
	};

	levels[7] =
	{
		tricornProgram,
		{
			0.000111066f,
			0.00547789f,
			0.00202005f,
			0.00013712f,
		},
		{
			Vector2(0.409404f, -1.1384f),
			Vector2(-1.25785f, -0.0921809f),
			Vector2(0.596074f, 1.10252f),
			Vector2(0.767101f, -1.31569f),
		}
	};

	levels[8] =
	{
		burningProgram,
		{
			0.0127253f,
			0.000665978f,
			0.000599382f,
			0.000393255f,
		},
		{
			Vector2(0.480201f, -1.14648f),
			Vector2(0.375798f, 0.0866547f),
			Vector2(-1.76489f, -0.0300707f),
			Vector2(-1.56364f, -0.000174844f),
		}
	};

	levels[9] =
	{
		juliaProgram1,
		{
			0.0157103f,
			0.00323462f,
			0.00608653f,
			0.0141394f,
		},
		{
			Vector2(0.231377f, 0.587359f),
			Vector2(-0.486454f, 1.0161f),
			Vector2(-0.50049f, -0.750328f),
			Vector2(-0.132677f, -0.069827f),
		}
	};

	levels[10] =
	{
		juliaProgram2,
		{
			0.00154711f,
			0.00927678f,
			0.000739975f,
			0.00191001f,
		},
		{
			Vector2(-1.25244f, -0.567376f),
			Vector2(0.558084f, 0.580386f),
			Vector2(0.950295f, -0.322775f),
			Vector2(-1.02154f, 0.270105f),
		}
	};

	levels[11] = 
	{
		juliaProgram0,
		{
			0.0618063f,
			0.04055110f,
			0.00154712f,
			0.0060865f,
		},
		{
			Vector2(-0.325299f, 0.554382f),
			Vector2(0.375164f, -0.3630710f),
			Vector2(-1.54658f, 0.11131f),
			Vector2(-1.44190f, 0.1496010f)
		}
	};

	gameWon = false;
	currentLevel = 0;
	LoadLevel();
	GeneratePreviews();
}

Game::~Game()
{

}

constexpr inline float MyClamp(float val, float min, float max)
{
	return std::max(std::min(val, max), min);
}

constexpr inline float MyLerp(float a, float b, float t)
{
	return a + t * (b - a);
}

void Game::OnEvent(const UpdateEvent&)
{
	if (Keyboard::IsKeyPressed(Key::Escape))
	{
		window->SetCloseFlag();
	}
	
	if (gameWon) return;

	float scroll = Mouse::GetScrollDelta().Y();
	zoomValue *= Pow(0.9f, scroll);

	//TODO: adjust offset when zooming so the screen stays centered
	//frame height == 2 * zoom
	
	if (Keyboard::IsKeyPressed(Key::Num1)) viewOffset = levels[currentLevel].offsets[0];
	if (Keyboard::IsKeyPressed(Key::Num2)) viewOffset = levels[currentLevel].offsets[1];
	if (Keyboard::IsKeyPressed(Key::Num3)) viewOffset = levels[currentLevel].offsets[2];
	if (Keyboard::IsKeyPressed(Key::Num4)) viewOffset = levels[currentLevel].offsets[3];

	// Reset zoom value
	if (Mouse::IsButtonDown(MouseButton::Middle))
	{
		zoomValue = defaultZoom;
		viewOffset = Vector2();
	}

	if (Mouse::IsButtonDown(MouseButton::Right))
	{
		// Amount we need to move
		auto v = Mouse::GetMouseDelta();
		
		viewOffset += Vector2(v[0] * (2.f * zoomValue / viewSize[0]),
		                      v[1] * (2.f * zoomValue / viewSize[1]));
	}

	if (Mouse::IsButtonPressed(MouseButton::Left))
	{
		Vector2 mouse(Mouse::GetMousePos());
		Vector2 world =  Vector2(mouse[0] * (2.f * zoomValue / viewSize[0]),
		                         mouse[1] * (2.f * zoomValue / viewSize[1]));
		world[0] -= zoomValue;
		world[1] -= zoomValue;
		world += viewOffset;

		for (UInt i = 0; i < 4; i++)
		{
			if (Vector2::Distance(world, levels[currentLevel].offsets[i]) <= levels[currentLevel].zooms[i] / 2.f)
			{
				std::cout << "Found image: " << i << std::endl;
				foundImages[i] = true;
			}
		}
	}

	bool foundAll = true;

	for (UInt i = 0; i < 4; i++)
	{
		if (foundImages[i])
		{

			for (UInt j = 0; j < 3; j++)
			{
				if (texColors[i][j] > 0.f)
				texColors[i][j] -= 0.05f;
			}
		}
		else
		{
			foundAll = false;
		}
	}

	if (foundAll)
	{
		// start fade out
		numIterations--;

		if (numIterations < 1)
		{
			zoomValue = defaultZoom;
			viewOffset = Vector2();
			currentLevel++;

			if (currentLevel == NUM_LEVELS)
			{
				currentLevel = 0;
				gameWon = true;
			}
			LoadLevel();
			GeneratePreviews();
		}
	}
	else if (numIterations < 60)
	{
		numIterations++;
	}

	if (false)
	{
		Vector2 mouse(Mouse::GetMousePos());
		Vector2 world =  Vector2(mouse[0] * (2.f * zoomValue / viewSize[0]),
		                         mouse[1] * (2.f * zoomValue / viewSize[1]));
		world[0] -= zoomValue;
		world[1] -= zoomValue;
		world += viewOffset;
		std::cout << "Offset: " << viewOffset[0] << ", " << viewOffset[1] << std::endl;
		std::cout << "Mouse pos: " << world[0] << ", " << world[1] <<
		"\nZoom Value: " << zoomValue << "\n";
	}
}

void Game::OnEvent(const VLFWMain::RenderWaitEvent&)
{
	if (window->GetCloseFlag()) return;
	glFinish();
}

void Game::OnEvent(const PostUpdateEvent&)
{
	if (window->GetCloseFlag()) return;

	auto ifb = window->GetFramebufferSize();
	Vector2 fb(ifb[0], ifb[1]);

	float r = fb[0];
	float l = 0.f;
	float t = fb[1];
	float b = 0.f;
	Matrix3 ortho(
		2.f / (r - l), 0.f, -(r + l) / (r - l),
		0.f, 2.f / (t - b), -(t + b) / (t - b),
		0.f, 0.f, 1.f);

	if (!gameWon)
	{
		glBindVertexArray(computeVAO);
		glUseProgram(levels[currentLevel].program);
		glUniform1i(0, 0); // Bind default texture
		glUniform1i(1, numIterations); // 60 iterations
		glUniform1f(2, zoomValue); // Use member zoom
		glUniform2f(3, viewOffset[0], viewOffset[1]); // Use member offset
		glDispatchCompute(viewSize[0], viewSize[1], 1); // Dispatch view size

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(quadVAO);
		glUseProgram(quadProgram);
		glUniform1i(0, 0); // Bind default texture
		glUniformMatrix3fv(1, 1, true, &ortho[0][0]);
		glUniform4f(2, 1.f, 1.f, 1.f, 1.f);

		// Draw big viewport
		glDrawArrays(GL_TRIANGLES, 0, 6);

		for (UInt i = 0; i < 4; i++)
		{
			glUniform1i(0, i + 1);
			glUniform4fv(2, 1, texColors[i].Data());
			glDrawArrays(GL_TRIANGLES, 6 * (i + 1), 6);
		}
	}
	else
	{
		static Color c = Color(0.f, 0.f, 0.f, 1.f);

		for (UInt i = 0; i < 3; i++)
		{
			if (c[i] < 1.f)
				c[i] += 0.01f;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(quadVAO);
		glUseProgram(quadProgram);
		glUniform1i(0, 5); // Bind default texture
		glUniformMatrix3fv(1, 1, true, &ortho[0][0]);
		glUniform4fv(2, 1, &c[0]);

		// Draw full screen
		glDrawArrays(GL_TRIANGLES, 30, 6);
	}
}

void Game::LoadLevel()
{
	currentProgram = levels[currentLevel].program;
	for (UInt i = 0; i < 4; i++)
	{
		foundImages[i] = false;
		texColors[i] = Color(1.f, 1.f, 1.f, 1.f);
	}
}

void Game::GeneratePreviews()
{
	glBindVertexArray(computeVAO);
	glUseProgram(levels[currentLevel].program);

	for (UInt i = 0; i < 4; i++)
	{
		glUniform1i(0, i + 1); // Bind default texture
		glUniform1i(1, 60); // more iterations
		glUniform1f(2, levels[currentLevel].zooms[i]); // Use member zoom
		glUniform2f(3, 
			levels[currentLevel].offsets[i][0], 
			levels[currentLevel].offsets[i][1]); // Use member offset
		glDispatchCompute(previewSize[0], previewSize[1], 1); // Dispatch view size

		foundImages[i] = false;
		texColors[i] = Color(1.f, 1.f, 1.f, 1.f);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

template <>
GLSLFile* vlk::ConstructContent(const std::string& path)
{
	std::ifstream file(path);
	std::cout << "Loading file: " << path << std::endl;

	// path parameter may not point to a valid file
	if (!file.good()) return nullptr;

	// Construct instance of MyContent
	GLSLFile* content = new GLSLFile();

	// Read file contents into object
	std::string temp;
	while (std::getline(file, temp))
	{
		content->data += (temp + "\n");
	}

	return content;
}

template <>
void vlk::DestroyContent(GLSLFile* file)
{
	delete file;
}
