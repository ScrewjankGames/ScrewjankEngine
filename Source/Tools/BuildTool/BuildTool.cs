using Serilog;
using System;
using System.Collections.Generic;

namespace build_tool 
{
	class BuildTool 
	{
		static void Main(string[] args) 
		{
			BuildToolConfig.Init();

			// Process Engine Assets


			Log.Information("Building Shaders");
			
			List<System.IO.FileInfo> vertex_shaders = new List<System.IO.FileInfo>();
			List<System.IO.FileInfo> fragment_shaders = new List<System.IO.FileInfo>();

			Utils.GlobFiles(BuildToolConfig.EngineAssetDir, "*.vert", vertex_shaders);
			Utils.GlobFiles(BuildToolConfig.EngineAssetDir, "*.frag", fragment_shaders);

			Log.Information("Vertex Shaders: {@VertexShaders}", string.Join(", ", vertex_shaders));
			Log.Information("Fragment Shaders: {@FragmentShaders}", string.Join(", ", fragment_shaders));

			Log.Information("Build Complete. Press any key to close...");
			Console.ResetColor();

			Console.ReadKey();
		}

	}
}
