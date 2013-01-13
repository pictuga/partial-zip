#include <stdio.h>
#include <sys/stat.h>
#include "common.h"
#include "partial/partial.h"

int isDir(const char* dir)
{
	struct stat s;
	stat(dir, &s);
	return (s.st_mode & S_IFDIR);
}

void callback(ZipInfo* info, CDFile* file, size_t progress) {
	int percentDone = progress * 100/file->compressedSize;
	fprintf(stderr, "Getting: %d%%\n", percentDone);
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("partialzip <zipfile> [<pattern> [-]]\r\n");
		return -1;
	}
	
	char* pattern = argv[2];
	char* fname;
	
	if (strstr(argv[1], "http://") == NULL && strstr(argv[1], "file://") == NULL)
	{
		fname = malloc( sizeof(char) * (strlen(argv[1]) + 7) );
		strcpy(fname, "file://");
	}
	else
	{
		fname = malloc( sizeof(char) * strlen(argv[1]) );
	}

	strcat(fname, argv[1]);
	
	ZipInfo* info = PartialZipInit(fname);
	if(!info)
	{
		printf("Cannot find %s\n", fname);
		return 0;
	}

	free(fname);

	PartialZipSetProgressCallback(info, callback);

	if(argc >= 3)
	{
		size_t size = 0;

		CDFile** files = PartialZipFindPattern(info, pattern, &size);
		if(!files)
		{
			printf("Cannot find %s in file.\n", pattern);
			return 0;
		}

		unsigned int i;
		for(i = 0; i < size; i++)
		{
			char* cur = (char*) files[i];
			char* curFileName = cur + sizeof(CDFile);
			char* myFileName = (char*) malloc(files[i]->lenFileName + 1);
			memcpy(myFileName, curFileName, files[i]->lenFileName);
			myFileName[files[i]->lenFileName] = '\0';

			if(myFileName[files[i]->lenFileName - 1] == '/')
			{
				if(!isDir(myFileName))
				{
					mkdir(myFileName, 0755);
				}
			}
			else
			{
				//FIXME create parent dirs

				unsigned char* data = PartialZipGetFile(info, files[i]);

				int dataLen = files[i]->size;

				data = realloc(data, dataLen + 1);
				data[dataLen] = '\0';

				if(argc == 4 && strlen(argv[3]) == 1 && argv[3][0] == '-')
				{
					if(size > 1 && i != 0)
						printf("\n");
					if(size > 1)
						printf("====%s====\n", myFileName);
					printf("%s\n", data);
				}
				else
				{
					FILE* out;
					out = fopen(myFileName, "w");
			
					if (out == NULL)
					{
						printf("Failed to open local file %s for write.\n", myFileName);
						exit(-1);
					}

					int done = 0;
					done = fwrite(data, sizeof(char), dataLen, out);
			
					fclose(out);
				}

				free(myFileName);
				free(data);
			}
		}
		
		PartialZipRelease(info);
		free(files);
	}
	else
	{
		PartialZipListFiles(info, 0);
	}

	return 0;
}

