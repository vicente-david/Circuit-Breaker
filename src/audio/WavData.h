#pragma once

#include <AL/al.h>
#include <cstdio>
#include <string>
class WavData {
  public:
	WavData() {
		printf("this shouldn't be called\n");
	}
	WavData(std::string file);
	// WavData(std::string name, std::string file);
	// std::string  name;
	// unsigned char *waveData;
	unsigned int dataSize;

	bool is3D;
	bool loop;
	ALenum format;
	ALuint buffer;
	int sampleRate;

	ALuint createSource();

  private:
	struct FmtChunk {
		char subChunkId[4];
		unsigned int subChunkSize;
		short int format;
		short int channels;
		unsigned int sampleRate;
		unsigned int byteRate;
		short int blockAlighn;
		short int bitsPerSample;
	};
	struct RiffWaveHeader {
		char chunkId[4];
		unsigned int chunkSize;
		char format[4];
	};

	struct SubChunkHeader {
		char subChunkId[4];
		unsigned int subChunkSize;
	};
};
