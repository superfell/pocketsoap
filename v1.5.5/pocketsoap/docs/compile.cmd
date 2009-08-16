@echo Off
@echo Compiling HTML Help
call settings.cmd

PUSHD %HTMLHELP%
hhc.exe %PATHTOPS%\docs\psdocs\psdocs.hhp

hhc.exe %PATHTOPS%\docs\proxyDocs\proxyDocs.hhp

hhc.exe %PATHTOPS%\docs\attachments\attachments.hhp

POPD
copy psdocs\psdocs.chm master
copy proxydocs\proxydocs.chm master
copy attachments\attachments.chm master

PUSHD %HTMLHELP%
hhc.exe %PATHTOPS%\docs\master\master.hhp

POPD
copy master\*.chm redist


@echo done !
