using Serilog;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;

namespace build_tool 
{
	abstract class GlobBuilder
	{
		public abstract IEnumerable<string> Extensions { get; }
		public abstract string BuilderName { get; }

		public abstract string BuilderExePath { get; }

		public abstract string OutputExtension { get; }

		bool BuildItem(FileInfo item, DirectoryInfo input_dir, DirectoryInfo output_dir)
		{
			Log.Information("{@builderName} building {@itemName}", BuilderName, item.Name);

			string newPath = String.Format("{0}{1}", output_dir, Utils.GetRelativePath(input_dir, item));
			FileInfo output_file = new FileInfo(Path.ChangeExtension(newPath, OutputExtension));
			output_file.Directory.Create();

			string args = String.Format("{0} {1}", item.FullName, output_file);

			Process builder = new Process();
			builder.StartInfo.FileName = BuildToolConfig.BuildersBinDir + BuilderExePath;
			builder.StartInfo.Arguments = args;
			Log.Information("command line: {0} {1}", builder.StartInfo.FileName, args);
			builder.StartInfo.UseShellExecute = false;
			builder.StartInfo.RedirectStandardOutput = true;

			builder.Start();

			// Todo [NL] Remove this bullshit and allow processes to be spawned and error reported async
			builder.WaitForExit();

			if (builder.ExitCode != 0)
			{
				Log.Error("{@builderName} {@itemName} Failed!", BuilderName, item.Name);
				return false;
			}

			return true;
		}

		public void BuildAll()
		{
			List<FileInfo> engine_items = new List<FileInfo>();
			Utils.GlobFiles(BuildToolConfig.EngineAssetDir, Extensions, engine_items);

			List<FileInfo> game_items = new List<FileInfo>();
			Utils.GlobFiles(BuildToolConfig.GameAssetDir, Extensions, game_items);

			foreach (FileInfo item in engine_items)
			{
				bool success = BuildItem(item, BuildToolConfig.EngineAssetDir, BuildToolConfig.EngineDataDir);
				if (!success)
				{
					Log.Error("Failed to build item {itemName}. Build aborted", item.FullName);
					return;
				}
			}

			foreach (FileInfo item in game_items)
			{
				bool success = BuildItem(item, BuildToolConfig.GameAssetDir, BuildToolConfig.GameDataDir);
				if (!success)
				{
					Log.Error("Failed to build item {itemName}. Build aborted", item.FullName);
					return;
				}
			}
		}
	}

	class TextureBuilder : GlobBuilder
	{
		public override IEnumerable<string> Extensions => new string[] { "*.png", "*.jpg" };

		public override string BuilderName => "Texture Builder";

		public override string BuilderExePath => "TextureBuilder/Release/TextureBuilder.exe";

		public override string OutputExtension => ".sj_tex";
	}

	class ModelBuilder : GlobBuilder
	{
		public override IEnumerable<string> Extensions => new string[] { "*.obj" };

		public override string BuilderName => "Model Builder";

		public override string BuilderExePath => "ModelBuilder/Release/ModelBuilder.exe";

		public override string OutputExtension => ".sj_mesh";
	}

	class SceneBuilder : GlobBuilder 
	{
		public override IEnumerable<string> Extensions => new string[] { "*.scene" };

		public override string BuilderName => "Scene Builder";

		public override string BuilderExePath => "SceneBuilder/Release/SceneBuilder.exe";

		public override string OutputExtension => ".sj_scene";
	}

	class BuildTool 
	{
		static void Main(string[] args) 
		{
			BuildToolConfig.Init();

			TextureBuilder texture_builder = new TextureBuilder();
			texture_builder.BuildAll();

			ModelBuilder model_builder = new ModelBuilder();
			model_builder.BuildAll();

			SceneBuilder scene_builder = new SceneBuilder();
			scene_builder.BuildAll();

			Log.Information("Build Complete. Press any key to close...");
			Console.ResetColor();

			Console.ReadKey();
		}

	}
}
