#pragma once

#include <string>
class Sound {
  public:

	void load(std::string file);
	void unload();
	void play();
	void stop();

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

	struct FmtChunk {
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned int sampleRate;
		unsigned int bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
	};

	unsigned int audioBufferId, audioSourceId;
	unsigned char *waveData;
	unsigned int waveSize;
};
