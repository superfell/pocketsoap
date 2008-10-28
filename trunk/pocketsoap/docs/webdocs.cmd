rd /S /Q web
mkdir web

xcopy master\*.htm* web\master /S /I 
xcopy master\*.css web\master /S /I 
xcopy master\*.gif web\master /S /I 
xcopy proxydocs\*.htm* web\proxydocs /S /I 
xcopy proxydocs\*.css web\proxydocs /S /I 
xcopy proxydocs\*.gif web\proxydocs /S /I 
xcopy psdocs\*.htm* web\psdocs /S /I 
xcopy psdocs\*.css web\psdocs /S /I 
xcopy psdocs\*.gif web\psdocs /S /I 
xcopy attachments\*.htm* web\attachments /S /I 
xcopy attachments\*.css web\attachments /S /I 
xcopy attachments\*.gif web\attachments /S /I 
copy webdocs\*.htm web 

rem fix .chm links
cd web
for /R %%f in ( *.htm* ) do call ..\sedone.cmd %%f
cd ..


