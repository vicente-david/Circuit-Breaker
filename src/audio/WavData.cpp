
#include "WavData.h"
#include "AudioEngine.h"
#include <AL/al.h>
#include <cstdio>
#include <cstring>
#include <string>

// load wav files
// very much taken from https://rastertek.com/gl4linuxtut56.html
WavData::WavData(std::string path) {
	// this->name = name;
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
	FmtChunk fmtData;

	while (foundFormat == false) {
		// Read in the sub chunk header.
		int count = fread(&fmtData, sizeof(fmtData), 1, file);
		if (count != 1) {
			fprintf(stderr, "%dcouldn't find  header in file '%s'\n", count,
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

	// parse format
	// only mono audio works in 3d
	if (fmtData.channels == 1) {
		is3D = true;
		if (fmtData.bitsPerSample == 8) {
			format = AL_FORMAT_MONO8;
		} else if (fmtData.bitsPerSample == 16) {
			format = AL_FORMAT_MONO16;
		} else {
			fprintf(stderr, "couldn't get format for wav file at '%s'\n",
					path.c_str());
			fprintf(stderr, "only 8/16 bit/sample allowed (got %d)\n",
					fmtData.bitsPerSample);
			return;
		}
	} else {
		is3D = false;
		if (fmtData.bitsPerSample == 8) {
			format = AL_FORMAT_STEREO8;
		} else if (fmtData.bitsPerSample == 16) {
			format = AL_FORMAT_STEREO16;
		} else {
			fprintf(stderr, "couldn't get format for wav file at '%s'\n",
					path.c_str());
			fprintf(stderr, "only 8/16 bit/sample allowed (got %d)\n",
					fmtData.bitsPerSample);
			return;
		}
	}
	sampleRate = fmtData.sampleRate;

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

		// Determine if it is the data header.  If not then move to the end
		// of the chunk and read in the next one.
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
	dataSize = subHeader.subChunkSize;

	auto waveData = new unsigned char[dataSize];
	// waveData = new unsigned char[dataSize];

	// Read in the wave file data into the newly created buffer.
	int count = fread(waveData, 1, dataSize, file);
	if (count != dataSize) {
		fprintf(stderr, "couldn't load wav file at '%s'\n", path.c_str());
		fprintf(stderr, "(%d/%d) bytes loaded\n", count, dataSize);
		return;
	}

	// Close the file once done reading.
	fclose(file);

	alGenBuffers((ALuint)1, &buffer);
	AudioEngine::checkALErrors("creating audio bufer for " + path);
	alBufferData(buffer, format, waveData, dataSize, sampleRate);
}

ALuint WavData::createSource() {
	ALuint channel;

	alGenSources((ALuint)1, &channel);
	if (!AudioEngine::checkALErrors("creating channel")) {
		return -1;
	}
	alSourcei(channel, AL_BUFFER, buffer);
	AudioEngine::checkALErrors("sending audio bufer");
	if (!loop) {
		alSourcei(channel, AL_LOOPING, AL_FALSE);
		AudioEngine::checkALErrors("setting no looping");
	}else{
		alSourcei(channel, AL_LOOPING, AL_TRUE);
		AudioEngine::checkALErrors("setting yes looping");
	}
	return channel;
}
