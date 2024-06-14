using Serilog;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace build_tool 
{
	class TextureBuilder 
	{
		/**
		 * @param textures The textures to build
		 * @param input_dir Root directory to use for making file paths relative
		 * @param output_dir Root directory assets will be placed in
		 */
		void BuildTextures(List<FileInfo> textures, DirectoryInfo input_dir, DirectoryInfo output_dir) 
		{
			foreach (var texture in textures)
			{
				Log.Information("Building texture {@TextureName}", texture.FullName);
				string newPath = String.Format("{0}{1}", output_dir, Utils.GetRelativePath(input_dir, texture));
				FileInfo output_file = new FileInfo(Path.ChangeExtension(newPath, ".sj_tex"));
				output_file.Directory.Create();

				string args = String.Format("{0} {1}", texture.FullName, output_file);

				Process textureBuilder = new Process();
				textureBuilder.StartInfo.FileName = BuildToolConfig.BuildersBinDir + "TextureBuilder/Release/TextureBuilder.exe";
				textureBuilder.StartInfo.Arguments = args;
				Log.Information("command line: {0} {1}", textureBuilder.StartInfo.FileName, args);
				textureBuilder.StartInfo.UseShellExecute = false;
				textureBuilder.StartInfo.RedirectStandardOutput = true;

				textureBuilder.Start();

				// Todo [NL] Remove this bullshit and allow processes to be spawned and error reported async
				textureBuilder.WaitForExit();

				if (textureBuilder.ExitCode != 0)
				{
					Log.Error("Texture Builder {@TextureName} Failed!", texture.Name);
				}
			}
		}

		public void BuildAll() 
		{
			Log.Information("Building engine textures");
			List<FileInfo> engine_textures = new List<FileInfo>();
			Utils.GlobFiles(BuildToolConfig.EngineAssetDir, extensions, engine_textures);			
			BuildTextures(engine_textures, BuildToolConfig.EngineAssetDir, BuildToolConfig.EngineDataDir);

			Log.Information("Building game textures");
			List<FileInfo> game_textures = new List<FileInfo>();
			Utils.GlobFiles(BuildToolConfig.GameAssetDir, extensions, game_textures);
			BuildTextures(game_textures, BuildToolConfig.GameAssetDir, BuildToolConfig.GameDataDir);
		}

		string[] extensions = new string[] { "*.jpg" };
	}

	class BuildTool 
	{
		static void Main(string[] args) 
		{
			BuildToolConfig.Init();

			TextureBuilder texture_builder = new TextureBuilder();
			texture_builder.BuildAll();

			Log.Information("Build Complete. Press any key to close...");
			Console.ResetColor();

			Console.ReadKey();
		}

	}
}
