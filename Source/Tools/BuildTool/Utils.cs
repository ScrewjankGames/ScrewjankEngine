using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace build_tool 
{
	class Utils 
	{
		public static void GlobFiles(DirectoryInfo root, IEnumerable<string> patterns, List<FileInfo> out_files)
		{
			foreach (string pattern in patterns) 
			{
				out_files.AddRange(root.GetFiles(pattern));
			}

			DirectoryInfo[] subdirectories = root.GetDirectories();

			foreach (var dir in subdirectories)
			{
				GlobFiles(dir, patterns, out_files);
			}
		}

        public static string CleanPath(string path) 
        {
            return path.Replace("\\", "/");
        }

		public static string GetRelativePath(DirectoryInfo dir, FileInfo file) 
		{
			string dir_path = CleanPath(dir.FullName);
			string file_path = CleanPath(file.FullName);

			if (!file_path.Contains(dir_path))
			{
				throw new ArgumentException("File is not in directory");
			}

			string relative_path = file_path.Remove(0, dir_path.Length);
			return relative_path;
		}
    }
}