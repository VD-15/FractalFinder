#ifndef GAME_HPP
#define GAME_HPP

#include "ValkyrieEngine/ValkyrieEngine.hpp"
#include "ValkyrieEngineCommon/ValkyrieEngineCommon.hpp"
#include "ValkyrieEngineCommon/Content.hpp"
#include "VLFW/VLFW.hpp"
#include <chrono>

using namespace vlk;
using namespace vlfw;

struct Level
{
	UInt program;
	float zooms[4];
	Vector2 offsets[4];
};

#define NUM_LEVELS 12

namespace game
{
	class Game final :
		public EventListener<UpdateEvent>,
		public EventListener<VLFWMain::RenderWaitEvent>,
		public EventListener<PostUpdateEvent>
	{
		Window* window;
		UInt currentProgram;
		UInt fractalOutput;
		UInt quadProgram;
		UInt tricornProgram;
		UInt juliaProgram0;
		UInt juliaProgram1;
		UInt juliaProgram2;
		UInt mandelProgram;
		UInt burningProgram;
		UInt endTexture;
		UInt quadVAO;
		UInt computeVAO;

		UInt currentLevel;
		UInt numIterations;
		UInt previewTextures[4];
		bool foundImages[4];
		Color texColors[4];
		bool gameWon;

		float zoomValue;
		Vector2 dragStart;
		Vector2 viewOffset;

		Vector2 fullSize;
		Vector2 viewSize;
		Vector2 previewSize;

		Level levels[NUM_LEVELS];

		public:
		Game(Window* _window);
		~Game();
		void OnEvent(const UpdateEvent&) override;
		void OnEvent(const VLFWMain::RenderWaitEvent&) override;
		void OnEvent(const PostUpdateEvent&) override;

		void LoadLevel();
		void GeneratePreviews();
	};

	struct GLSLFile
	{
		std::string data;
	};
}

namespace vlk
{
	template <>
	game::GLSLFile* ConstructContent(const std::string& path);

	template <>
	void DestroyContent(game::GLSLFile* file);
}

#endif
