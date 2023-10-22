// This content is part of test.h
// Platform specific functions

#ifdef _WIN32
#include <windows.h>

int _listFiles(char* path, char** output)
{
  char basePath[MAX_PATH];
	char wildPath[MAX_PATH];
	int fileCount = 0;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	strcpy(basePath, path);
  if(path[strlen(path-1)] != '/')
    strcat(basePath, "/");
  strcpy(wildPath, basePath);
	strcat(wildPath,"*");

	int basePathLength = strlen(basePath);
	hFind = FindFirstFile(wildPath, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	  return 0;

	do
	{
	  if(strcmp(".", ffd.cFileName) != 0 && strcmp("..", ffd.cFileName) != 0)
	  {
      if(output)
      {
        output[fileCount] = malloc(basePathLength + strlen(ffd.cFileName) + 2);
	      strcpy(output[fileCount], basePath);
        strcat(output[fileCount], ffd.cFileName);
      }
      fileCount++;
	  }
	}
	while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);

	return fileCount;
}
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool _isDirectory(char* path)
{
  struct stat stats;
  if(stat(path, &stats) == 0)
  {
    if(S_ISDIR(stats.st_mode)) return true;
  }
  return false;
}

int _listFiles(char* path, char** output)
{
  int fileCount = 0;
  DIR *dir;
	struct dirent *ent;
  int pathLength = strlen(path);

	if ((dir = opendir (path)) != 0) 
  {
	  while ((ent = readdir (dir)) != 0)
    {
      if(strcmp(".", ent->d_name) != 0 && strcmp("..", ent->d_name) != 0)
      {
        if(output)
        {
          output[fileCount] = (char*)malloc(pathLength + strlen(ent->d_name) + 2);
          strcpy(output[fileCount], path);
          if(path[pathLength-1] != '\\' && path[pathLength-1] != '/')
            strcat(output[fileCount], "/");
          strcat(output[fileCount], ent->d_name);
        }
        fileCount++; 
      }
	  }
	  closedir (dir);
	}
  
  return fileCount;
}
#endif
