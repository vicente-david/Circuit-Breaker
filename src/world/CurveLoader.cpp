#include "CurveLoader.h"
#include <glm/glm.hpp>

// open a file
// assume it's .obj
// assume every object is a curve until proven not
// assume a curve is something that starts with "v" and contains only "v" and "l"
// if any other fields are detected ex: "f" then it is not a curve
// "o" means object, so after an object has been parsed, then reset and parse the next object (assuming curve once again)
std::vector<TrackCurve> CurveLoader::loadCurve(const std::string path) {

	std::vector<TrackCurve> curves;
	// open file for reading (input file stream)
	std::ifstream trackFile(path);
	
	if (!trackFile.is_open()) std::cout << "Failed to open file: " << path << std::endl;

	std::string currentLine;

	bool isCurve = true; // default to true, assume anything that has v and l in obj file are curves 

	TrackCurve trackCurve;

	// reads .obj line by line
	while (std::getline(trackFile, currentLine)) {
		// automatically split line by white spaces
		std::istringstream stringElements(currentLine);
		std::string prefix;
		// stream the string elements 
		stringElements >> prefix;
		if (prefix == "o") {
			// if empty or a curve, then reset without adding the trackCurve to list of curves
			if (!(trackCurve.curvePoints.empty() || !isCurve)) {
				// if trackpoints are non empty and it is a curve
				trackCurve.curvatures = calculateCurveAngles(trackCurve.curvePoints);
				curves.push_back(trackCurve);
			}
			trackCurve.curvePoints.clear(); // new vector time
			isCurve = true; // assume curve 
		}else if(!isCurve) { // if we're not reseting using o, then do nothing
			continue;
		}
		else if (prefix == "v") { // vertices are ordered automatically in obj format (assumption)
			float x, y, z;
			stringElements >> x >> y >> z;
			trackCurve.curvePoints.push_back(glm::vec3(x,y,z));
		}
		else if (prefix != "l") { // l is necessary to detect the curve, if l isn't detected and the above cases fail, then it's not a curve
			isCurve = false;
			// note, this a very strong statement, the istringstream trims whitespaces 
			// but obj files can have comments
			// prefixed with #
			// so if there's something not loading properly, it might be because of this
		}

	}

	// if eof but no o
	if (!(trackCurve.curvePoints.empty() || !isCurve)) {
		// if trackpoints are non empty and it is a curve
		trackCurve.curvatures = calculateCurveAngles(trackCurve.curvePoints);
		curves.push_back(trackCurve);
		
	}
	
	

	trackFile.close();

	return curves;
}


// Calculate angle at each point in the curve to predict how much a spark must turn to continue along track
std::vector<float> CurveLoader::calculateCurveAngles(std::vector<glm::vec3> curvePoints) {
	std::vector<float> angles;
	
	for (int i = 0; i < curvePoints.size(); i++) {
		glm::vec3 point = curvePoints.at(i);

		// direction to target
		glm::vec3 behind = curvePoints.at((i - 8) % curvePoints.size());
		behind = point - behind;
		glm::vec2 dirIn(behind.x, behind.z);
		// direction leaving target
		glm::vec3 ahead = curvePoints.at((i + 6) % curvePoints.size());
		ahead = ahead - point;
		glm::vec2 dirOut(ahead.x, ahead.z);

		float curvature;

		if (glm::length(dirIn) * glm::length(dirOut) < 1e-2) {
			curvature = 0.0f;
		}
		else {
			curvature = glm::dot(dirIn, dirOut) / (glm::length(dirIn) * glm::length(dirOut));
			curvature = glm::clamp(curvature, -1.0f, 1.0f);
			curvature = glm::acos(curvature); // curvature 0 = straight, 1 = curve
		}
		
		angles.push_back(curvature);
	}

	return angles;
	
}
