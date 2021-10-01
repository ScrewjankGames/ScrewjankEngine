using Serilog;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace build_tool 
{
	class ShaderBuilder 
	{
		/**
		 * @param shaders The shaders to build
		 * @param input_dir Root directory to use for making file paths relative
		 * @param output_dir Root directory assets will be placed in
		 */
		void BuildShaders(List<FileInfo> shaders, DirectoryInfo input_dir, DirectoryInfo output_dir) 
		{
			foreach (var shader in shaders)
			{
				Log.Information("Building shader {@ShaderName}", shader.Name);
				
				FileInfo output_file = new FileInfo(String.Format("{0}{1}", output_dir, Utils.GetRelativePath(input_dir, shader)));
				output_file.Directory.Create();

				string args = String.Format("{0} -o {1}.spv", shader.FullName, output_file);

				Process glslc = new Process();
				glslc.StartInfo.FileName = "glslc.exe";
				glslc.StartInfo.Arguments = args;
				glslc.StartInfo.UseShellExecute = false;
				glslc.StartInfo.RedirectStandardOutput = true;

				glslc.Start();

				// Todo [NL] Remove this bullshit and allow processes to be spawned and error reported async
				glslc.WaitForExit();

				if (glslc.ExitCode != 0)
				{
					Log.Error("Shader Compilation {@ShaderName} Failed!", shader.Name);
				}
			}
		}

		public void BuildAll() 
		{
			Log.Information("Building engine shaders");
			List<FileInfo> engine_shaders = new List<FileInfo>();
			Utils.GlobFiles(BuildToolConfig.EngineAssetDir, extensions, engine_shaders);			
			BuildShaders(engine_shaders, BuildToolConfig.EngineAssetDir, BuildToolConfig.EngineDataDir);

			Log.Information("Building game shaders");
			List<FileInfo> game_shaders = new List<FileInfo>();
			Utils.GlobFiles(BuildToolConfig.GameAssetDir, extensions, game_shaders);
			BuildShaders(game_shaders, BuildToolConfig.GameAssetDir, BuildToolConfig.GameDataDir);
		}

		string[] extensions = new string[] { "*.vert", "*.frag" };
	}

	class BuildTool 
	{
		static void Main(string[] args) 
		{
			BuildToolConfig.Init();

			ShaderBuilder shader_builder = new ShaderBuilder();
			shader_builder.BuildAll();

			Log.Information("Build Complete. Press any key to close...");
			Console.ResetColor();

			Console.ReadKey();
		}

	}
}
