@echo off
@echo Source Package builder for WSDL 2.0
@echo .

if exist (wsdl2.0.1-src.zip) del wsdl2.0.1-src.zip
"c:\program files\winzip\wzzip.exe" -a -r -P wsdl2.0.1-src.zip @src_list.txt -x@exclude_list.txt
