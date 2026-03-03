// takes a obj file, locates all objects that contain only the "v" and "l" property
// there is another unsupported format which defines a literal curve object
// rather than line segments, this parser will fail to detect that type of curve


// this may need to be adjusted depending how branching paths work
// currently if you model the branching paths as separate curves you just get different TrackCurve structs

#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <glm/vec3.hpp>

// defines the curve of a track (assuming blender .obj file)
struct TrackCurve {
	std::vector<glm::vec3> curvePoints; // ordered list of points
	// curvePoints.front() defines the start of the curve
	// curvePoints.back() define the end of the curve

};

class CurveLoader {
public:
	// returns a list of trackcurves in order of .obj appearance
	static std::vector<TrackCurve> loadCurve(const std::string path);

};