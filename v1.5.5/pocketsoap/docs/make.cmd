@echo off
call settings.cmd
del psdocs\*.* /q
if exist DocGen.log del DocGen.log

wizrun.exe /q %PATHTODOCGEN%\docgenwizard.gxw bAutoGen=true projectName=psdocs projectDirectory="%PATHTOPS%\docs\psdocs" idlFile="%PATHTOPS%\psoap_core\psoap.idl" xmlFile="%PATHTOPS%\docs\ps.xml"

if exist DocGen.log type DocGen.log
