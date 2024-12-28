FROM ubuntu:18.04

# Install dependencies
RUN apt-get update -yq && \
    apt-get install -y \
    software-properties-common \
    build-essential \
    git \
    tree \
    curl \
    python3.8 \
    libgl1-mesa-dev \
    libfontconfig1 \
    libxkbcommon-x11-0 \
    libdbus-1-3

RUN python3.8 --version 

# Install aqtinstall and Qt
RUN python3.8 -m pip3 install aqtinstall 
RUN python3.8 -m aqt install-qt linux desktop 5.15.2 gcc_64 \
    --outputdir /opt/Qt

ENV PATH="/opt/Qt/5.15.2/gcc_64/bin:${PATH}"
ENV QT_PLUGIN_PATH="/opt/Qt/5.15.2/gcc_64/plugins"
ENV QML_IMPORT_PATH="/opt/Qt/5.15.2/gcc_64/qml"
ENV QML2_IMPORT_PATH="/opt/Qt/5.15.2/gcc_64/qml"

# Copy any scripts you need
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
 