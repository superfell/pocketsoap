@echo off

:: Cleanup
if exist foo\. rd /s /q foo & md foo
if exist DocGen.log del DocGen.log

:: Run!
start /wait WizRun.exe ..\DocGenWizard.gxw

:: Show errors
if exist DocGen.log xce.exe DocGen.log
