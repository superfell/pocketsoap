@echo off
@echo generating docs for attachments library
call settings.cmd
del attachments\*.* /q
if exist DocGen.log del DocGen.log

wizrun.exe /q %PATHTODOCGEN%\docgenwizard.gxw projectName=attachments projectDirectory="%PATHTOPS%\docs\attachments" idlFile="%PATHTOPS%\attachments_core\Attachments.idl" xmlFile="%PATHTOPS%\docs\attachments.xml"

cd attachments
rem fix links
for /R %%f in ( *.html ) do call ..\sedst.cmd %%f

cd..

if exist DocGen.log type DocGen.log
