# Face Recognition Applications

This is a face recognition applications based on OpenCV. The 
development machine is Ubuntu Linux 14.04, but it should also work 
in Ubuntu Linux 12.04 LTS. The OpenCV version of development is 
OpenCV 2.4.9 (2014-04-25). To summarize, the best runtime 
environment, which is tested, will be as follows.

* __Operating System__: Ubuntu Linux 14.04
* __OpenCV Version__: OpenCV 2.4.9 (2014-04-25)


# Installation

In this section, the installation procedure will be discussed.

## OpenCV Installation

This collection of applications require OpenCV libraries. To 
install OpenCV, you can follow these steps.

1. Download OpenCV source code of version 2.4.9 for Linux from 

   `http://opencv.org/downloads.html`

2. Extract the downloaded package to your workspace. Notice that 
   these files must NOT be removed after installation, because 
   OpenCV application require the libraries from the installation 
   path. So you should find a good place for the extracted files.

3. Run the following command at the root path of the extracted 
   package to generate the makefile.

   `$ cmake .`

4. Then run the following command to compile the OpenCV source 
   codes.

   `$ make`

5. After compiling the source code, run the following command to 
   install the OpenCV. This command require administrator power.

   `$ sudo make install`

If there is no error, then you have successfully installed the 
OpenCV on your machine.

## Project Installation

This project only requires OpenCV and Linux environments as the 
runtime. After that, you just need to copy the entire project to 
your workspace.

The compiling instructions will be discussed in the next section.


# Compile and Run

In this section, the compiling and executing procedures of the 
involved applications will be discussed.

## Compiling Applications

All applications can be compiled to an executable program through 
the same way. The `generate.sh` script at the root path is 
provided as an easy tool to help compile the source codes to the 
corresponding executable programs.

### Query Applications

One source file in the `src/` directory represents an application 
and you can use the following commnad to query all applications 
available to be generated into executable program.

`./generate.sh`

After running the above command, the usage of this script in blue 
color and names of all available applications in yellow color 
will be printed on the screen.

Notice that the script should have execution property. If it does 
not, use the following command to make it executable.

`sudo chmod 777 generate.sh`

The above command gives the execution property to the script.

### Generate Applications

Use the following command to generate, or says, compile an 
application.

`./generate.sh <app>`

This argument `<app>` is the name of the application to compile. 
All available applications can be found via the method mentioned 
in the previous section.

Notice that the name of application does NOT include its file name 
extension.

For example, if the application to compile is `FaceCollection`, then 
run the following command.

`./generate.sh FaceCollection`

The letter case should be considered, and after running the above 
command, the corresponding executable program `FaceCollection.out` 
will be generated in the `release/` directory.

### FaceCollection

This application helps collect facial data for face recognition. 
Its usage is as follows.

`./FaceCollection.out <cascade> <data_path> <device_id>`

Where

- `<cascade>` is the path of the pre-trained Haar-cascade of for face 
  detection.

- `<data_path>` is the path to the face database.

- `<device_id>` is the video device identifier to capture video 
  stream from. Usually it is `0` if only one webcam connects to 
  the machine.

As the application launches, the user is required to type its name to 
the system.

It combines face detection and image corpping functions. A rectangle 
will occur on the video if any face is detected. And the user just 
need to press [space] on the keyboard to save the face image as an 
image file. Pressing [p] on the keyboard will inform the system to 
collect protrait.

### FaceRecognitionImage

This application performs face recognition from image file using 
OpenCV. The usage is as follows.

`./FaceRecognitionImage <cascade> <data_path> <in_image> <out_info> [<out_image>]`

Where 

- `<cascade>` is the path of the pre-trained Haar-cascade of for 
  face detection.

- `<data_path>` is the path to the face database.

- `<in_image>` is the path of the image file to perform face 
  recognition on.

- `<out_info>` is the expected path to put the face recognition 
  result. The output result is in JSON format.

- `<out_image>` is the expected path to put the output image file 
  after face recognition. A rectangle and corresponding tags will 
  occur if any face is detected. This argument is optional, and 
  if it is not provided, no output image will be generated.

### Name2Protraits

This application converts name of a person to its protraits.

`./Name2Protraits.out <data_path> <name> <out_protraits>`

Where

- `<data_path>` is the path to the face database.

- `<name>` is the target name to look for protraits.

- `<out_protraits>` is the expected path to put the paths to 
  protraits. The output result is in JSON format.


# Directory Structure

Below is the directory structure and corresponding introduction.

* `src/`: Source code directory. All the source codes of the 
  applications will be placed in this directory. Each souce code 
  file in this directory represent one application, or says, one 
  executable program.

* `build/`: Build directory. This directory includes all files of 
  compiling process. During compiling, temporary files generated 
  will be placed here. 

  - `./CMakeLists.txt.template`: CMakeLists template. It has the 
    commands for cmake, but it cannot be used directly. This 
    template file will be used by the `generate.sh` script to 
    create corresponding `CMakeLists.txt` file for different 
    applications.

* `release/`: Executable application release directory. The 
  compiled executable applications will be placed here after they 
  are compiled. Some data files will also be placed here, but 
  those files are not a part of the release applications.

* `generate.sh`: Application generation script. This script will 
  help generate different applications.

* `README.md`: Read me document. It is this document, which gives 
  introduction to the project and some instructions.


# Authorship

Below is the copyright announcement and the contact of the 
author.

* __Author__:  David Qiu
* __Email__:   david@davidqiu.com
* __Email__:   dicong.qiu@intel.com
* __Website__: http://www.davidqiu.com/

Copyright (C) 2014, David Qiu. All rights reserved.

Notice that there are some data files in the `release` directory 
and these files are eithor published on the Internet or collected 
by the author of this project.

For those published data files, the copyright belong to the 
original authors.

For those data files collected by the author of this project, the 
copyright belongs to the author of this project, and must NOT be 
used in any circumstance without approvement from the author of 
this project.



