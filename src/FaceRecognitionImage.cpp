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

#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
using namespace cv;
using namespace std;


const int STD_PROTRAIT_SIZE       = 256;
const int STD_FACE_REC_SIZE       = 64;
const int STD_DETECT_FRAME_WIDTH  = 320;
const int STD_DETECT_FRAME_HEIGHT = 240;


/**
 * @brief
 *   Type of directory item.
 */ 
enum DirectoryItemType
{
  DIRITEM_OTHER = 0,  // Other directory item type
  DIRITEM_FILE = 1,   // Normal file
  DIRITEM_DIR = 2     // Directory item
};


/**
 * @param dirpath Path to the directory.
 * @param items Item names from the directory
 * @param types Directory types of corresponding items.
 * @return An int indicating the operation state. "0" indicates 
 *         success and negative numbers indicate failure.
 *
 * @brief
 *    Traverse a directory and obtain all the items. Hidden files 
 *    will be ignored.
 */
int traverseDirectory(string dirpath, vector<string>& items, vector<DirectoryItemType>& types)
{
  DIR* dp;
  struct dirent* dirp;
  struct stat st;

  // Clear the vectors
  items.clear();
  types.clear();

  // Open dirent directory
  if ((dp = opendir(dirpath.c_str())) == NULL)
  {
    cerr << "[ERROR] traverseDirectory(string, vector<string>&, vector<DirectoryItemType>&): " 
         << "Cannot open the directory " << dirpath << "." << endl;
    return -1;
  }

  // Read all files in this dir
  while ((dirp = readdir(dp)) != NULL)
  {
    // Ignore hidden files
    if (dirp->d_name[0] == '.') continue;

    // Obtain the full name of the item
    string fullname = dirpath + string("/") + string(dirp->d_name);

    // Obtain the item status
    if (stat(fullname.c_str(), &st) == -1)
    {
      cerr << "[ERROR] traverseDirectory(string, vector<string>&, vector<DirectoryItemType>&): " 
           << "Cannot obtain the status of the item " << fullname << "." << endl;
      return -1;
    }

    // Obtain the item type
    DirectoryItemType itemType = DIRITEM_OTHER;
    if (S_ISREG(st.st_mode)) itemType = DIRITEM_FILE;
    if (S_ISDIR(st.st_mode)) itemType = DIRITEM_DIR;
    
    // Push to the vectors
    items.push_back(dirp->d_name);
    types.push_back(itemType);
  }

  return 0;
}


/**
 * @param datapath Path to face database directory.
 * @param images Array to store face image data.
 * @param labels Array to store corresponding labels for face images.
 * @param names Mapping from label to name of the face.
 *
 * @brief
 *    Load face data from face database directory.
 */
void loadFaceData(const string& datapath, vector<Mat>& images, vector<int>& labels, map<int, string>& names)
{
  vector<string> items_data;
  vector<DirectoryItemType> types_data;
  vector<string> items_face;
  vector<DirectoryItemType> types_face;
  
  // Clear the result containers
  images.clear();
  labels.clear();
  names.clear();

  // Obtain the face data path
  string faceDataPath = datapath + "/faces";

  // Traverse the data directory
  if (traverseDirectory(faceDataPath, items_data, types_data) < 0)
  {
    string error_message = "Cannot read the face database directory " + faceDataPath + ".";
    cerr << "[ERROR] loadFaceData(const string&, vector<Mat>&, vector<int>&, map<int, string>&): "
         << error_message << endl;
    CV_Error(CV_StsBadArg, error_message);
  }

  // Inform the loading state
  cout << "[INFO] Open face data directory \"" << faceDataPath << "\". Now loading: " << endl;
  
  // Traverse each face directory
  for (int i=0; i<items_data.size(); ++i)
  {
    // Check if the current item is a directory
    if ( !(types_data[i]==DIRITEM_DIR) ) continue;

    // Obtain the face image directory path
    string faceImagePath = faceDataPath + "/" + items_data[i];

    // Traverse the face image directory
    if (traverseDirectory(faceImagePath, items_face, types_face) < 0)
    {
      string error_message = "Cannot read the face image directory " + faceImagePath + ".";
      cerr << "[ERROR] loadFaceData(const string&, vector<Mat>&, map<int, string>&): "
           << error_message << endl;
      CV_Error(CV_StsBadArg, error_message);
    }

    // Inform the loading state
    cout << "\t- " << items_data[i] << " [" << i + 1 << "/" << items_data.size() << "]" << endl;

    // Push data to result containers
    for (int j=0; j<items_face.size(); ++j)
    {
      // Check if the current item is a normal file
      if ( !(types_face[j]==DIRITEM_FILE) ) continue;

      // Obtain the image path
      string imagePath = faceImagePath + "/" + items_face[j];
      
      // Read the image and push to the result containers
      Mat img_original = imread(imagePath, 0);
      Mat img_resized;
      cv::resize(img_original, img_resized, Size(STD_FACE_REC_SIZE, STD_FACE_REC_SIZE), 1.0, 1.0, INTER_CUBIC);
      images.push_back(img_resized);
      labels.push_back(i);
      names.insert( pair<int, string>(i, items_data[i]) );
      
      // Inform the loading state
      cout << "\t\t- " << items_face[j] << endl;
    }
  }
}


/**
 * @brief
 *    Program entry of the application.
 */
int main(int argc, const char *argv[])
{
  // Check for valid command line arguments
  if (argc < 5)
  {
    cout << "usage: " << argv[0] << " <cascade> <data_path> <in_image> <out_info> [<out_image>]" << endl;
    cout << "\t <cascade>   -- Path to the Haar Cascade for face detection." << endl;
    cout << "\t <data_path> -- Path to the face database." << endl;
    cout << "\t <in_image>  -- Input image to process face recognition." << endl;
    cout << "\t <out_info>  -- Output information of the face recognition result." << endl;
    cout << "\t <out_image> -- Output image of the face recognition result. (optional)" << endl;
    exit(1);
  }

  // Read the program arguments
  string fn_cascade = string(argv[1]);
  string dir_data = string(argv[2]);
  string fn_inimage = string(argv[3]);
  string fn_outinfo = string(argv[4]);
  string fn_outimage = "";
  bool has_outimage = false;
  if (argc > 5) {
    fn_outimage = string(argv[5]);
    has_outimage = true;
  }

  // Obtain the subpaths and ensure existance
  string dir_faces = dir_data + "/faces";
  if ( !(access(dir_faces.c_str(), 0)==0) )
  {
    cerr << "[ERROR] The path to face database does not exist." << endl;
    exit(1);
  }
  
  // Load the face database
  vector<Mat> images;
  vector<int> labels;
  map<int, string> names;
  try
  {
    loadFaceData(dir_data, images, labels, names);
  }
  catch (cv::Exception& e)
  {
    cerr << "[Error] Failed to load the face data. Reason: " << e.msg << endl;
    exit(1);
  }
  cout << "[INFO] Face database loaded." << endl;

  // Get the standard face image size
  int im_width = images[0].cols;
  int im_height = images[0].rows;
  cout << "[INFO] Standard face image size is " << im_width << "*" << im_height << endl;

  // Create and train a face recognizer
  Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
  model->train(images, labels);
  cout << "[INFO] Face recognizer trained." << endl;

  // Create and train a face detecter
  CascadeClassifier haar_cascade;
  haar_cascade.load(fn_cascade);
  cout << "[INFO] Face Haar-Like cascade trained." << endl;

  // Load the input image from file
  Mat original = imread(fn_inimage);
  cout << "[INFO] Load input image to process." << endl;

  // Convert the original image to grayscale:
  Mat gray;
  cvtColor(original, gray, CV_BGR2GRAY);

  // Find the faces from the original image
  vector< Rect_<int> > faces;
  haar_cascade.detectMultiScale(gray, faces);
  cout << "[INFO] " << faces.size() << " faces detected. Faces are:" << ((faces.size())?(""):(" (NO DATA).")) << endl;

  // Recognize all the faces found
  stringstream ssInfo;
  for(int i = 0; i < faces.size(); i++)
  {
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
    if (has_outimage)
    {
      // Tag the face with a rectangle
      rectangle(original, face_i, CV_RGB(0, 255,0), 1);
      
      // Put the prediction information above the rectangle
      int pos_x = std::max(face_i.tl().x - 10, 0);
      int pos_y = std::max(face_i.tl().y - 10, 0);
      string box_text = format("%s [%.2lf]", strName.c_str(), confidence);
      putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
    }

    // Output the recognition information
    if (i>0) ssInfo << ",";
    ssInfo << "{"
           << "\"prediction\":\"" << strName << "\","
           << "\"confidence\":" << confidence << ","
           << "\"position\":" << "{"
                              << "\"x\":" << face_i.tl().x << ","
                              << "\"y\":" << face_i.tl().y
                              << "},"
           << "\"size\":" << "{"
                          << "\"width\":" << face_i.size().width << ","
                          << "\"height\":" << face_i.size().height
                          << "}"
           << "}";

    // Inform the face recognition result
    cout << "\t- " << strName << " [" << confidence << "]" << endl;
  }

  // Output the recognition result as file
  ofstream ofsInfo;
  ofsInfo.open(fn_outinfo.c_str());
  if (ofsInfo.is_open())
  {
    string strInfo;
    ssInfo >> strInfo;
    ofsInfo << "[" << strInfo << "]" << endl;
    ofsInfo.close();
    cout << "[INFO] Output the information file as \"" << fn_outinfo << "\"" << endl;
  }
  else
  {
    cerr << "[ERROR] Cannot open the file \"" << fn_outinfo << "\"." << endl;
    exit(1);
  }

  // Ouput the recognition result image
  if (has_outimage)
  {
    imwrite(fn_outimage.c_str(), original);
    cout << "[INFO] Output the processed image as \"" << fn_outimage << "\"" << endl;
  }

  return 0;
}

