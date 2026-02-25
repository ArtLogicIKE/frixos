@echo off
cd \espressif\source\frixos
date /t
time /t
echo pushing to v%1
copy build\frixos.bin c:\espressif\source\frixos-web\www\revE%1.bin /b/y > nul
copy spiffs\* c:\espressif\source\frixos-web\www\%1\ /b/y > nul
