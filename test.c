#include <stdio.h>
#include "common.h"
#include "partial/partial.h"

void callback(ZipInfo* info, CDFile* file, size_t progress) {
	int percentDone = progress * 100/file->compressedSize;
	fprintf(stderr, "Getting: %d%%\n", percentDone);
}

int main(int argc, char* argv[]) {
	
	if (argc < 2) {
		printf("partialzip <zipfile> [<pattern> [<outfile>]]\r\n");
		return -1;
	}
	
	int len = strlen(argv[0]);
	char* pattern = argv[2], fname[len+7];
	char* outfile;
	
	if(argc >= 3)
	{
		outfile = argv[2];
	
		if (argc >= 4)
		{
			outfile = argv[3];	
		}
	}
	
	if (strstr(argv[1], "http://") == NULL && strstr(argv[1], "file://") == NULL)
	{
		strcpy(fname, "file://");
	}

	strcat(fname, argv[1]);
	
	ZipInfo* info = PartialZipInit(fname);
	if(!info)
	{
		printf("Cannot find %s\n", fname);
		return 0;
	}

	PartialZipSetProgressCallback(info, callback);

	if(argc >= 3)
	{
		size_t size = 0;

		CDFile** files = PartialZipFindPattern(info, pattern, &size);
		if(!files)
		{
			printf("Cannot find %s in %s\n", pattern, fname);
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
				//skip for now
				//FIXME create them in near future
			}
			else
			{
				unsigned char* data = PartialZipGetFile(info, files[i]);

				int dataLen = files[i]->size;

				data = realloc(data, dataLen + 1);
				data[dataLen] = '\0';

				if(argc == 4 && strlen(outfile) == 1 && outfile[0] == '-')
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
					if(size == 0)
						out = fopen(outfile, "w");
					else
						out = fopen(myFileName, "w");
			
					if (out == NULL)
					{
						printf("Failed to open file\n");
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

