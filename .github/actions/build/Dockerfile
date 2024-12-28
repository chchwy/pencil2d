FROM ubuntu:18.04

# Install dependencies
RUN apt-get update -yq
RUN apt-get install -yq --no-install-suggests --no-install-recommends \
    software-properties-common \
    build-essential \
    git curl libfuse2 tree \
    qt515tools qt515base qt515multimedia qt515svg qt515wayland \
    bsdtar ffmpeg libgl1-mesa-dev \
    gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly gstreamer1.0-alsa gstreamer1.0-pulseaudio git

# Copy any scripts you need
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
 