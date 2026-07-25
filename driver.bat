@echo off
setlocal
dotnet run "%~dp0scripts\driver\spectra-bootstrap-driver.cs" -- %*
exit /b %errorlevel%
