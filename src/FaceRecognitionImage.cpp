/**
* Face recognition from image file using OpenCV.
*
* Author:  David Qiu.
* Email:   david@davidqiu.com
* Email:   dicong.qiu@intel.com
* Website: http://www.davidqiu.com/
*
* Copyright (C) 2014, David Qiu. All rights reserved.
* 
* This piece of code is modified from the video stream face recognition 
* toturial of OpenCV.
*/

/*
* Copyright (c) 2011. Philipp Wagner <bytefish[at]gmx[dot]de>.
* Released to public domain under terms of the BSD Simplified license.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of the organization nor the names of its contributors
*     may be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*   See <http://www.opensource.org/licenses/bsd-license>
*/

#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>

using namespace cv;
using namespace std;

const int STD_FACE_WIDTH  = 64;
const int STD_FACE_HEIGHT = 64;


static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, map<int, string>& names, char separator = ';') {
  std::ifstream file(filename.c_str(), ifstream::in);
  if (!file) {
    string error_message = "No valid input file was given, please check the given filename.";
    CV_Error(CV_StsBadArg, error_message);
  }
  string line, path, classlabel, classname;
  while (getline(file, line)) {
    stringstream liness(line);
    getline(liness, path, separator);
    getline(liness, classlabel, separator);
    getline(liness, classname);
    if(!path.empty() && !classlabel.empty() && !classname.empty()) {
      Mat face_resized;
      cv::resize(imread(path,0), face_resized, Size(STD_FACE_WIDTH, STD_FACE_HEIGHT), 1.0, 1.0, INTER_CUBIC);
      images.push_back(face_resized);
      labels.push_back(atoi(classlabel.c_str()));
      names.insert( pair<int, string>(atoi(classlabel.c_str()), classname) );
    }
  }
}

int main(int argc, const char *argv[]) {
  // Check for valid command line arguments
  if (argc < 5) {
    cout << "usage: " << argv[0] << " <cascade> <csv> <in_image> <out_info> [<out_image>]" << endl;
    cout << "\t <cascade>   -- Path to the Haar Cascade for face detection." << endl;
    cout << "\t <csv>       -- Path to the CSV file with the face database." << endl;
    cout << "\t <in_image>  -- Input image to process face recognition." << endl;
    cout << "\t <out_info>  -- Output information of the face recognition result." << endl;
    cout << "\t <out_image> -- Output image of the face recognition result. (optional)" << endl;
    exit(1);
  }

  // Read the program arguments
  string fn_haar = string(argv[1]);
  string fn_csv = string(argv[2]);
  string fn_inimage = string(argv[3]);
  string fn_outinfo = string(argv[4]);
  string fn_outimage = "";
  bool has_outimage = false;
  if (argc > 5) {
    fn_outimage = string(argv[5]);
    has_outimage = true;
  }

  // Open the output information file
  FILE* outinfo = fopen(fn_outinfo.c_str(), "w");
  if (outinfo == NULL) {
    cerr << "Error opening file \"" << fn_outinfo << "\"." << endl;
    exit(1);
  }

  // Load data from the face database
  vector<Mat> images;
  vector<int> labels;
  map<int, string> names;
  try {
    read_csv(fn_csv, images, labels, names);
  } catch (cv::Exception& e) {
    cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
    exit(1);
  }

  // Get the standard size of the images (same size required by fisherface algorithm)
  int im_width = images[0].cols;
  int im_height = images[0].rows;

  // Train a face recognizer model (face recognition)
  Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
  model->train(images, labels);

  // Train a cascade classifier model (face detection)
  CascadeClassifier haar_cascade;
  haar_cascade.load(fn_haar);

  // Load the input image from file
  Mat original = imread(fn_inimage);

  // Convert the original image to grayscale:
  Mat gray;
  cvtColor(original, gray, CV_BGR2GRAY);

  // Find the faces from the original image
  vector< Rect_<int> > faces;
  haar_cascade.detectMultiScale(gray, faces);

  // Recognize all the faces found
  fprintf(outinfo, "[");
  for(int i = 0; i < faces.size(); i++) {
    // Obtain the current face to process
    Rect face_i = faces[i];

    // Crop the face from the image
    Mat face = gray(face_i);

    // Resizing the face image for recognition
    Mat face_resized;
    cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);

    // Perform the recognition prediction
    double confidence = 0.0;
    int prediction = -1;
    model->predict(face_resized, prediction, confidence);
    string strName = names[prediction];
     
    // Check if has output image
    if (has_outimage) {
      // Tag the face with a rectangle
      rectangle(original, face_i, CV_RGB(0, 255,0), 1);
      
      // Put the prediction information above the rectangle
      int pos_x = std::max(face_i.tl().x - 10, 0);
      int pos_y = std::max(face_i.tl().y - 10, 0);
      string box_text = format("%s [%lf]", strName.c_str(), confidence);
      putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
    }

    // Output the recognition information
    if (i>0) fprintf(outinfo, ",");
    fprintf(outinfo, "{\"prediction\":\"%s\",\"confidence\":%lf,\"pos_x\":%d,\"pos_y\":%d}", 
      strName.c_str(), 
      confidence, 
      face_i.tl().x,
      face_i.tl().y);
  }
  fprintf(outinfo, "]\n");

  // Ouput the recognition result image
  if (has_outimage) imwrite(fn_outimage.c_str(), original);
  
  return 0;
}

