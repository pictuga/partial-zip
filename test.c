#include <stdio.h>
#include "common.h"
#include "partial/partial.h"

void callback(ZipInfo* info, CDFile* file, size_t progress) {
	int percentDone = progress * 100/file->compressedSize;
	fprintf(stderr, "Getting: %d%%\n", percentDone);
}

int main(int argc, char* argv[]) {
	
	if (argc < 2) {
		printf("partialzip <zipfile> <extract> [<outfile>]\r\n");
		return -1;
	}
	
	int len = strlen(argv[0]);
	char* extract = argv[2], fname[len+7];
	char* outfile;
	
	if(argc >= 3)
	{
		outfile = argv[2];
	
		if (argc >= 4)
		{
			outfile = argv[3];	
		}
	}
	
	if (strstr(argv[1], "http://") == NULL || strstr(argv[1], "file://") == NULL)
	{
		strcpy(fname, "file://");
	}

	strcpy(fname, argv[1]);
	
	ZipInfo* info = PartialZipInit(fname);
	if(!info)
	{
		printf("Cannot find %s\n", fname);
		return 0;
	}

	PartialZipSetProgressCallback(info, callback);

	if(argc >= 3)
	{
		CDFile* file = PartialZipFindFile(info, extract);
		if(!file)
		{
			printf("Cannot find %s in %s\n", extract, fname);
			return 0;
		}

		unsigned char* data = PartialZipGetFile(info, file);
		int dataLen = file->size; 

		PartialZipRelease(info);

		data = realloc(data, dataLen + 1);
		data[dataLen] = '\0';
	
		if(argc == 4 && strlen(outfile) == 1 && outfile[0] == '-')
		{
			printf("%s\n", data);
		}
		else
		{
			FILE* out;
			out = fopen(outfile, "w");
	
			if (out == NULL)
			{
				printf("Failed to open file");
				exit(-1);
			}

			int done = 0;
			done = fwrite(data, sizeof(char), dataLen, out);
	
			fclose(out);
		}

		free(data);
	}
	else
	{
		PartialZipListFiles(info);
	}

	return 0;
}

