
# RTMP server download:
https://github.com/bluenviron/mediamtx/releases
Download the release for the required platform

# Webcam access for the application under WSL
I'm using the WSL so I need to share the webcam in order for the GStreamer to get access to the dev/video0 i.e. camera 
Run this in powershell under administrator 
1. usbipd unbind --busid 2-13
2. usbipd bind --busid 2-13 --force
3. usbipd.exe attach --wsl --busid 2-13


# run from cmd to start the stream with RTMP with a test video source
gst-launch-1.0 videotestsrc ! videoconvert ! x264enc tune=zerolatency ! flvmux streamable=true ! rtmpsink location="rtmp://0.0.0.0/stream live=1"

# run from cmd to start the stream with RTMP with a test video from webcam
gst-launch-1.0 v4l2src device=/dev/video0 ! image/jpeg,width=640,height=480,framerate=30/1 ! jpegdec ! videoconvert ! x264enc tune=zerolatency bitrate=1500 ! flvmux streamable=true ! rtmpsink location="rtmp://0.0.0.0/stream live=1"

http://localhost:8889/stream/ - to see the stream in browser 
rtmp://localhost/stream - to watch stream in VLC

# HOW to start RTMP server
podman run --rm -it --network=host docker.io/bluenviron/mediamtx
