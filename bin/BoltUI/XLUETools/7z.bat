@echo off
@set PACK=".\7z.exe"
@%PACK% a -tzip -mx0 ".\skin.zip" ".\skin\*"

@pause