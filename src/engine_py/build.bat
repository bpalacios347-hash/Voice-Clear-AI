@echo off
echo Building Voice Clear AI Engine...
call venv\Scripts\activate
pyinstaller --onedir --noconsole --name VoiceClearEngine engine.py
echo Build complete!
