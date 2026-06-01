
# RTMP server download:
https://github.com/bluenviron/mediamtx/releases
Download the release for the required platform

# Webcam access for the application under WSL
I'm using the WSL so I need to share the webcam in order for the GStreamer to get access to the dev/video0 i.e. camera 
Run this in powershell under administrator 
1. usbipd unbind --busid 2-13
2. usbipd bind --busid 2-13 --force
3. usbipd.exe attach --wsl --busid 2-13
