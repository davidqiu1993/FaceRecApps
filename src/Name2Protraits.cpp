/**
 * Name to protrait paths conversion tool. This application 
 * helps convert a name to protrait paths from the face 
 * database.
 *
 * This application reads the input argument to look for the 
 * target name, and output a JSON format file indicating the 
 * paths of corresponding protraits.
 *
 * Author:  David Qiu.
 * Email:   david@davidqiu.com
 * Email:   dicong.qiu@intel.com
 * Website: http://www.davidqiu.com/
 *
 * Copyright (C) 2014, David Qiu. All rights reserved.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
using namespace std;


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
 * @brief
 *    Program entry of the application.
 */
int main(int argc, const char *argv[])
{
  stringstream ssInfo;

  // Check the arguments
  if (argc != 4) {
    cout << "usage: " << argv[0] << " <data_path> <name> <info_path>" << endl;
    cout << "\t <data_path> -- Path to the face database directory." << endl;
    cout << "\t <name> -- Name of the person to convert." << endl;
    cout << "\t <info_path> -- Path to the JSON file of protrait paths." << endl;
    exit(1);
  }

  // Get the arguments
  string dir_data = string(argv[1]);
  string name = string(argv[2]);
  string fn_info = string(argv[3]);

  // Obtain the subpaths and ensure existance
  string dir_protraits = dir_data + "/protraits";
  
  // Check if the path exists
  if (access(dir_protraits.c_str(), 0) == 0)
  {
    vector<string> items_protraits;
    vector<DirectoryItemType> types_protraits;
    
    // Traverse the directory
    traverseDirectory(dir_protraits, items_protraits, types_protraits);
    cout << "[INFO] Open protrait data directory \"" << dir_protraits << "\"." << endl;

    // Look for protrait directory to the corresponding name
    for (int i=0; i<items_protraits.size(); ++i)
    {
      if (name == items_protraits[i])
      {
        // Check if the item is a directory
        if (types_protraits[i] != DIRITEM_DIR) break;
        
        // Obtain the protrait directory path
        string dir_usrprotraits = dir_protraits + "/" + name;
        cout << "[INFO] Found user protrait directory \"" << dir_usrprotraits << "\". Protrait images: " << endl;

        // Load the file names
        vector<string> items_usrprotraits;
        vector<DirectoryItemType> types_usrprotraits;
        bool firstFileFlag = true;
        traverseDirectory(dir_usrprotraits, items_usrprotraits, types_usrprotraits);
        for (int j=0; j<items_usrprotraits.size(); ++j)
        {
          // Check item type and output result
          if (types_usrprotraits[j]==DIRITEM_FILE)
          {
            if (!firstFileFlag) ssInfo << ",";
            string protraitImagePath = dir_usrprotraits + "/" + items_usrprotraits[j];
            ssInfo << "\"" << protraitImagePath << "\"";
            cout << "\t- " << protraitImagePath << endl;
            firstFileFlag = false;
          }
        }

        // Skip the rest search
        break;
      }
    }
  }

  // Output the result
  ofstream ofsInfo;
  ofsInfo.open(fn_info.c_str());
  if (ofsInfo.is_open())
  {
    string strInfo;
    ssInfo >> strInfo;
    ofsInfo << "[" << strInfo << "]" << endl;
    ofsInfo.close();
    cout << "[INFO] Output result as file \"" << fn_info << "\"." << endl;
  }
  else
  {
    cerr << "[ERROR] Cannot open the file \"" << fn_info << "\"." << endl;
  }

  return 0;
}


