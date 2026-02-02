#pragma once

#include <string>
struct WavData {
  public:
	WavData(std::string name, std::string file);
	std::string  name;
	unsigned char *waveData;
	unsigned int waveSize;

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
	struct FmtChunk fmtData;

  private:
	struct RiffWaveHeader {
		char chunkId[4];
		unsigned int chunkSize;
		char format[4];
	};

	struct SubChunkHeader {
		char subChunkId[4];
		unsigned int subChunkSize;
	};

	unsigned int audioBufferId, audioSourceId;
};
