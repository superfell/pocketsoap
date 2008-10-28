rem @echo off
@echo Source Package builder for pocketXMLRPC 1.1
@echo .

rd /s /q src_tmp
mkdir src_tmp
cd src_tmp
cvs export -r %1 3rdparty
cvs export -r %1 common
cvs export -r %1 pocketHTTP
cvs export -r %1 pocketXMLRPC

"c:\Program Files\winzip\WZZIP.EXE" -a -r -p src.zip *.*

cd ..
copy src_tmp\src.zip src-%1.zip
