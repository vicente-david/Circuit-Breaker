
#include "WavData.h"
#include <cstdio>
#include <cstring>
#include <string>

// load wav files
// very much taken from https://rastertek.com/gl4linuxtut56.html
WavData::WavData(std::string name, std::string path) {
	this->name = name;
	auto file = fopen(path.c_str(), "rb");
	if (file == NULL) {
		fprintf(stderr, "audio file at '%s' not found\n", path.c_str());
		return;
	}

	// read the header section of the wav
	RiffWaveHeader header;
	fread(&header, sizeof(header), 1, file);

	// this is a really confusing way of writing this if statement:
	// if (!(strcmp(header.chunkId, "RIFF") || strcmp(header.chunkId, "WAVE")))
	// you can't use strcmp though because it wont always be null terminated
	if (!(((header.chunkId[0] == 'R') && (header.chunkId[1] == 'I') &&
		   (header.chunkId[2] == 'F') && (header.chunkId[3] == 'F')) ||

		  ((header.format[0] == 'W') && (header.format[1] == 'A') &&
		   (header.format[2] == 'V') && (header.format[3] == 'E')))) {
		fprintf(stderr, "invalid header '%s' for wav file at '%s'\n",
				header.chunkId, path.c_str());
		return;
	}

	// find format chunk

	bool foundFormat = false;
	while (foundFormat == false) {
		// Read in the sub chunk header.
		int count = fread(&fmtData, sizeof(fmtData), 1, file);
		if(count!=1){
			printf("%dcouldn't find  header in file '%s'\n", count,
				   path.c_str());
			return;
		}


		// Determine if it is the fmt header.  If not then move to the end of
		// the chunk and read in the next one.
		if ((fmtData.subChunkId[0] == 'f') && (fmtData.subChunkId[1] == 'm') &&
			(fmtData.subChunkId[2] == 't') && (fmtData.subChunkId[3] == ' ')) {
			// if (strcmp(fmtData.subChunkId, "fmt ")) {
			foundFormat = true;
		} else {
			// fseek(file, fmtChunk.subChunkSize, SEEK_CUR);
		}
	}

	// read the format chunk
	// FmtChunk fmtData;
	// int count = fread(&fmtData, sizeof(fmtData), 1, file);
	// if (count != 1) {
	// 	throw std::runtime_error("invalid header when loading wav file");
	// }

	// Seek up to the next sub chunk.
	// fseek(file, subHeader.subChunkSize, SEEK_CUR);

	// find the actual data chunk
	// Read in the sub chunk headers until you find the data chunk.
	bool foundData = false;
	SubChunkHeader subHeader;
	while (foundData == false) {
		// Read in the sub chunk header.
		int count = fread(&subHeader, sizeof(subHeader), 1, file);
		if (count != 1) {
			printf("%dcouldn't find data header in file '%s'\n", count,
				   path.c_str());
			return;
		}
		// printf("id:%s\n", subHeader.subChunkId);
		// printf("h size:%d\n", subHeader.subChunkSize);

		// Determine if it is the data header.  If not then move to the end of
		// the chunk and read in the next one.
		if ((subHeader.subChunkId[0] == 'd') &&
			(subHeader.subChunkId[1] == 'a') &&
			(subHeader.subChunkId[2] == 't') &&
			(subHeader.subChunkId[3] == 'a')) {
			// if (strcmp(subHeader.subChunkId, "data")) {
			foundData = true;
		} else {
			fseek(file, subHeader.subChunkSize, SEEK_CUR);
		}
	}

	// Store the size of the data chunk.
	waveSize = subHeader.subChunkSize;

	waveData = new unsigned char[waveSize];

	// Read in the wave file data into the newly created buffer.
	int count = fread(waveData, 1, waveSize, file);
	if (count != waveSize) {
		fprintf(stderr, "couldn't load wav file at '%s'\n", path.c_str());
		fprintf(stderr, "(%d/%d) bytes loaded\n", count, waveSize);
		return;
	}

	// Close the file once done reading.
	fclose(file);
}
