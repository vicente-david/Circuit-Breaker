
#include "Sound.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>

// load wav files
// very much taken from https://rastertek.com/gl4linuxtut56.html
void Sound::load(std::string path) {

	auto file = fopen(path.c_str(), "rb");
	if (file == NULL) {
		fprintf(stderr, "audio file at '%s' not found\n", path.c_str());
		return;
	}

	// read the header section of the wav
	RiffWaveHeader header;
	fread(&header, sizeof(header), 1, file);
	if (!(strcmp(header.chunkId, "RIFF") || strcmp(header.chunkId, "WAVE"))) {
		fprintf(stderr, "invalid header '%s' for wav file at '%s'\n",
				header.chunkId, path.c_str());
		return;
	}
	printf("head id:%s\n", header.chunkId);

	// find format chunk
	SubChunkHeader subHeader;

	bool foundFormat = false;
	while (foundFormat == false) {
		// Read in the sub chunk header.
		fread(&subHeader, sizeof(subHeader), 1, file);

		printf("(1)h size:%d\n", subHeader.subChunkSize);
		// Determine if it is the fmt header.  If not then move to the end of
		// the chunk and read in the next one.
		// if ((subHeader.subChunkId[0] == 'f') &&
		// 	(subHeader.subChunkId[1] == 'm') &&
		// 	(subHeader.subChunkId[2] == 't') &&
		// 	(subHeader.subChunkId[3] == ' ')) {
		if (strcmp(subHeader.subChunkId, "fmt ")) {
			foundFormat = true;
		} else {
			fseek(file, subHeader.subChunkSize, SEEK_CUR);
		}
	}
	printf("id:%s size:%d\n", subHeader.subChunkId, subHeader.subChunkSize);

	// read the format chunk
	FmtChunk fmtData;
	int count = fread(&fmtData, sizeof(fmtData), 1, file);
	if (count != 1) {
		throw std::runtime_error("invalid header when loading wav file");
	}

	// Seek up to the next sub chunk.
	// fseek(file, subHeader.subChunkSize, SEEK_CUR);

	// find the actual data chunk
	// Read in the sub chunk headers until you find the data chunk.
	bool foundData = false;
	while (foundData == false) {
		// Read in the sub chunk header.
		count = fread(&subHeader, sizeof(subHeader), 1, file);
		if (count != 1) {
			printf("%dcouldn't find data header in file '%s'\n", count,
				   path.c_str());
			return;
		}
		printf("id:%s\n", subHeader.subChunkId);
		// printf("h size:%d\n", subHeader.subChunkSize);

		// Determine if it is the data header.  If not then move to the end of
		// the chunk and read in the next one.
		// if ((subHeader.subChunkId[0] == 'd') &&
		// 	(subHeader.subChunkId[1] == 'a') &&
		// 	(subHeader.subChunkId[2] == 't') &&
		// 	(subHeader.subChunkId[3] == 'a')) {
		if (strcmp(subHeader.subChunkId, "data")) {
			foundData = true;
		} else {
			fseek(file, subHeader.subChunkSize, SEEK_CUR);
		}
	}

	// Store the size of the data chunk.
	waveSize = subHeader.subChunkSize;

	waveData = new unsigned char[waveSize];

	// Read in the wave file data into the newly created buffer.
	count = fread(waveData, 1, waveSize, file);
	if (count != waveSize) {
		fprintf(stderr, "couldn't load wav file at '%s'\n", path.c_str());
		fprintf(stderr, "(%d/%d) bytes loaded\n", count, waveSize);
		return;
	}

	// Close the file once done reading.
	fclose(file);
}
