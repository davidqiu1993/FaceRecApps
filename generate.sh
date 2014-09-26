#!/bin/bash

printApplications()
{
  for f in ./src/*
  do
    fname=`basename $f`
    echo -e "\033[33m${fname%%.*}\033[0m"
  done
}

generateAllApps()
{
  for f in ../src/*
  do
    fname=`basename $f`
    appname=${fname%%.*}
    src_cmake=`sed -e "s/{app}/$appname/g" CMakeLists.txt.template`
    echo "$src_cmake" > CMakeLists.txt
    cmake .
    make
  done
}

if [ $# -lt 1 ] 
then
  echo -e "\033[36m[INFO] Please run $0 <app> to generate executable application. Available applications are:\033[0m"
  printApplications
  echo -e "\033[33m-a (all applications)\033[0m"
  exit 0
else
  echo -e "\033[36m[INFO] Your project is $1.\033[0m"
fi

cd build
if [ "$1" = "-a" ]
then
  generateAllApps
else
  src_cmake=`sed -e "s/{app}/$1/g" CMakeLists.txt.template`
  echo "$src_cmake" > CMakeLists.txt
  cmake .
  make
fi

