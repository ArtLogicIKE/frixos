@echo off
pushd
cd \espressif\source\frixos
date /t
time /t
echo pushing to v%1
copy build\frixos.bin c:\espressif\source\frixos-web\www\revE%1.bin /b/y
cd spiffs
del files.txt
dir /l /os /b > ..\files.txt
copy ..\files.txt .\
echo re-created files.txt
copy * c:\espressif\source\frixos-web\www\%1\ /b/y > nul
echo copied spiffs
popd
