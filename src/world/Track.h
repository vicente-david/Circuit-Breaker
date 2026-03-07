#include "CurveLoader.h"
#include "../graphics/Model.h"
#include <vector>

class Track {

public:

	Track(char* path) {
		model = Model(path);
		paths = CurveLoader::loadCurve(path);
	}
	Model model;
	std::vector<TrackCurve> paths;

};