rem @echo off
call settings.cmd
del proxydocs\*.* /q
if exist DocGen.log del DocGen.log

wizrun.exe /q %PATHTODOCGEN%\docgenwizard.gxw projectName=proxydocs projectDirectory="%PATHTOPS%\docs\proxydocs" idlFile="%PATHTOPS%\psProxy_Core\psProxy.idl" xmlFile="%PATHTOPS%\docs\proxy.xml"


if exist DocGen.log type DocGen.log
