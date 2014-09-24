/**
 * Face collection application. This application helps collect
 * facial data for face recognition automatically.
 *
 * As the application launches, the user is required to type 
 * its name to the system.
 *
 * It combines face detection and image corpping functions. A 
 * rectangle will occur on the video if any face is detected. 
 * And the user just need to press [space] on the keyboard to 
 * save the face image as an image file. Pressing [p] on the 
 * keyboard will inform the system to collect protrait.
 *
 * Author:  David Qiu.
 * Email:   david@davidqiu.com
 * Email:   dicong.qiu@intel.com
 * Website: http://www.davidqiu.com/
 *
 * Copyright (C) 2014, David Qiu. All rights reserved.
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
  bool saveFaceFlag = false;
  bool saveProtraitFlag = false;
  int saveImageCount = 0;
  bool exitAppFlag = false;
  string expectName;

  // Check the arguments
  if (argc != 4) {
    cout << "usage: " << argv[0] << " <cascade> <data_path> <device_id>" << endl;
    cout << "\t <cascade> -- Path to the Haar Cascade for face detection." << endl;
    cout << "\t <data_path> -- Path to the face database directory." << endl;
    cout << "\t <device_id> -- The webcam device id to grab frames from." << endl;
    exit(1);
  }

  // Get the arguments
  string fn_cascade = string(argv[1]);
  string dir_data = string(argv[2]);
  int deviceId = atoi(argv[3]);

  // Obtain the subpaths and ensure existance
  string dir_faces = dir_data + "/faces";
  string dir_protraits = dir_data + "/protraits";
  system( (string("mkdir -p ") + dir_faces).c_str() );
  system( (string("mkdir -p ") + dir_protraits).c_str() );

  // Request for the expect name of the user and ensure existance
  cout << "Please type the name of current user. (No space)" << endl;
  cout << "NAME: ";
  cin >> expectName;
  string dir_usrfaces = dir_faces + "/" + expectName;
  string dir_usrprotraits = dir_protraits + "/" + expectName;
  system( (string("mkdir -p ") + dir_usrfaces).c_str() );
  system( (string("mkdir -p ") + dir_usrprotraits).c_str() );
  
  // Load the face database
  vector<Mat> images;
  vector<int> labels;
  map<int, string> names;
  map<string, int> name2label;
  try
  {
    loadFaceData(dir_data, images, labels, names);
  }
  catch (cv::Exception& e)
  {
    cerr <<  "[Error] Failed to load the face data. Reason: " << e.msg << endl;
    exit(1);
  }
  for (map<int, string>::iterator it=names.begin(); it!=names.end(); ++it)
  {
    name2label.insert( pair<string, int>(it->second, it->first) );
  }

  // Get the standard face image size
  int im_width = images[0].cols;
  int im_height = images[0].rows;

  // Create and train a face recognizer
  Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
  model->train(images, labels);

  // Create and train a face detecter
  CascadeClassifier haar_cascade;
  haar_cascade.load(fn_cascade);

  // Open the video capture device
  VideoCapture cap(deviceId);
  if(!cap.isOpened()) {
      cerr << "Capture Device ID " << deviceId << "cannot be opened." << endl;
      return -1;
  }

  // Get the frame inversed scale factor
  Mat frame;
  cap >> frame;
  double invfscale_x = ((double)frame.cols)/((double)STD_DETECT_FRAME_WIDTH);
  double invfscale_y = ((double)frame.rows)/((double)STD_DETECT_FRAME_HEIGHT);

  // Process frames from the video stream
  for(;;)
  {
    // Obtain the next frame from the video stream
    cap >> frame;

    // Clone the current frame
    Mat original = frame.clone();

    // Resize the frame
    Mat original_resized;
    cv::resize(original, original_resized, Size(STD_DETECT_FRAME_WIDTH, STD_DETECT_FRAME_HEIGHT), 1.0, 1.0, INTER_CUBIC);
    
    // Convert to grayscale
    Mat gray;
    cvtColor(original_resized, gray, CV_BGR2GRAY);

    // Find the faces in the frame
    vector< Rect_<int> > faces;
    haar_cascade.detectMultiScale(gray, faces);

    // Process all the faces detected
    for(int i = 0; i < faces.size(); i++)
    {
      // Obtain the current face
      Rect face_i = faces[i];
      Rect face_i_original(
        (double)face_i.x * invfscale_x,
        (double)face_i.y * invfscale_y,
        (double)face_i.width * invfscale_x,
        (double)face_i.height * invfscale_y
      );

      // Crop the face from the image
      Mat face = gray(face_i);

      // Resize the face image for recognition
      Mat face_resized;
      cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);

      // Perform prediction
      double confidence = 0.0;
      int prediction = -1;
      model->predict(face_resized, prediction, confidence);

      // Check if save face image
      if (saveFaceFlag)
      {
        // Construct the file name
        stringstream ssFilename;
        string strFilename;
        ssFilename << dir_usrfaces << "/";
        ssFilename << time(NULL) << "_" << saveImageCount << ".jpg";
        ssFilename >> strFilename;

        // Save image as file
        imwrite(strFilename.c_str(), face_resized);
        saveImageCount++; 
        cout << "[INFO] Image saved as \"" << strFilename << "\"" << endl;

        // Append the saved face to runtime
        int usrLabel = -1;
        if (name2label.find(expectName)!=name2label.end()) usrLabel = name2label[expectName];
        images.push_back(face_resized);
        labels.push_back(usrLabel);
        names.insert( pair<int, string>(usrLabel, expectName) );
        name2label.insert( pair<string, int>(expectName, usrLabel) );
        model->train(images, labels);

        // Reset the save face flag
        saveFaceFlag = false;
      }
      
      // Check if save protrait image
      if (saveProtraitFlag)
      {
        // Obtain the protrait image
        Mat protrait = original_resized(face_i);
        Mat protrait_resize;
        cv::resize(protrait, protrait_resize, Size(STD_PROTRAIT_SIZE, STD_PROTRAIT_SIZE), 1.0, 1.0, INTER_CUBIC);
        
        // Construct the file name
        stringstream ssFilename;
        string strFilename;
        ssFilename << dir_usrprotraits << "/";
        ssFilename << time(NULL) << "_" << saveImageCount << ".jpg";
        ssFilename >> strFilename;
        
        // Save image as file
        imwrite(strFilename.c_str(), protrait_resize);
        saveImageCount++; 
        cout << "[INFO] Image saved as \"" << strFilename << "\"" << endl;

        // Reset the save protrait flag
        saveProtraitFlag = false;
      }

      // Tag the face with rectangle
      rectangle(original, face_i_original, CV_RGB(0, 255,0), 1);

      // Put information above the rectangle
      string strName = names[prediction];
      string box_text = format("Prediction = %s [%lf]", strName.c_str(), confidence);
      int pos_x = std::max(face_i_original.tl().x - 10, 0);
      int pos_y = std::max(face_i_original.tl().y - 10, 0);
      putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
    }

    // Show the result
    imshow("face_collection", original);

    // Detect keyboard events
    char key = (char) waitKey(1);
    switch (key)
    {
        case 27: exitAppFlag=true; break;
        case 'p': saveProtraitFlag = true; break;
        case ' ': saveFaceFlag=true; break;
    }
    if (exitAppFlag) break;
  }

  return 0;
}


