FROM ubuntu:16.04

ENV APPIMAGE_EXTRACT_AND_RUN=1

# Install dependencies
RUN apt-get -yq update
RUN apt-get -yqq install build-essential
RUN apt-get -yqq update
RUN apt-get -yqq install software-properties-common

RUN apt-add-repository -y ppa:ubuntu-toolchain-r/test && \
    #apt-add-repository -y ppa:ubuntu-sdk-team/ppa && \
    apt-add-repository -y ppa:git-core/ppa && \
    apt-add-repository -y ppa:beineri/opt-qt-5.15.2-xenial

RUN apt-get -yqq update

RUN apt-get -yqq --no-install-suggests --no-install-recommends install \
    git curl libfuse2 tree \
    qt515tools qt515base qt515multimedia qt515svg qt515wayland \
    bsdtar ffmpeg libgl1-mesa-dev \
    gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly gstreamer1.0-alsa gstreamer1.0-pulseaudio git

# Copy any scripts you need
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
 