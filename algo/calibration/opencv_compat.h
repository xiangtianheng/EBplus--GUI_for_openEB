#ifndef GUI_ALGO_CALIBRATION_OPENCV_COMPAT_H
#define GUI_ALGO_CALIBRATION_OPENCV_COMPAT_H

#include <opencv2/core/version.hpp>

// OpenCV 5 split calibration, pattern detection, and point geometry.
#if CV_VERSION_MAJOR >= 5
#include <opencv2/calib.hpp>
#include <opencv2/geometry.hpp>
#include <opencv2/objdetect.hpp>
#else
#include <opencv2/calib3d.hpp>
#endif

#endif
