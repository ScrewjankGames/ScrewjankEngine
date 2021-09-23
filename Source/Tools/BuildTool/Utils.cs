using System.Collections.Generic;

namespace build_tool 
{
	class Utils 
	{
		public static void GlobFiles(System.IO.DirectoryInfo root, string pattern, List<System.IO.FileInfo> out_files)
		{
			out_files.AddRange(root.GetFiles(pattern));

			System.IO.DirectoryInfo[] subdirectories = root.GetDirectories();

			foreach (var dir in subdirectories)
			{
				GlobFiles(dir, pattern, out_files);
			}

			return;
		}

	}
}