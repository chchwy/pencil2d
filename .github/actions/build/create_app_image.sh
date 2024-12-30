
$IS_RELEASE = "false"


create_package_linux() {
  echo "::group::Set up AppImage contents"
  make install INSTALL_ROOT="${PWD}/Pencil2D"
  echo "::endgroup::"

  echo "::group::Create AppImage"
  # "Downgrade" the desktop entry to version 1.0
  sed -i "/^Keywords\(\[[a-zA-Z_.@]\+\]\)\?=/d;/^Version=/cVersion=1.0" \
    Pencil2D/usr/share/applications/org.pencil2d.Pencil2D.desktop
  install -Dm755 /usr/bin/ffmpeg Pencil2D/usr/plugins/ffmpeg
  install -Dm755 "/usr/lib/x86_64-linux-gnu/gstreamer1.0/gstreamer-1.0/gst-plugin-scanner" \
    "Pencil2D/usr/lib/gstreamer1.0/gstreamer-1.0/gst-plugin-scanner"
  local gst_executables="-executable=Pencil2D/usr/lib/gstreamer1.0/gstreamer-1.0/gst-plugin-scanner"
  for plugin in adpcmdec alsa app audioconvert audioparsers audioresample \
      autodetect coreelements gsm id3demux jack mpg123 mulaw playback \
      pulse typefindfunctions wavparse apetag; do
    install -Dm755 "/usr/lib/x86_64-linux-gnu/gstreamer-1.0/libgst${plugin}.so" \
      "Pencil2D/usr/lib/gstreamer-1.0/libgst${plugin}.so"
    gst_executables="${gst_executables} -executable=Pencil2D/usr/lib/gstreamer-1.0/libgst${plugin}.so"
  done
  curl -fsSLO https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
  chmod 755 linuxdeployqt-continuous-x86_64.AppImage
  local update_info="" # Currently no appimageupdate support for nightly builds
  if [ $IS_RELEASE = "true" ]; then
    update_info="-updateinformation=gh-releases-zsync|${GITHUB_REPOSITORY/\//|}|latest|pencil2d-linux-amd64-*.AppImage.zsync"
  fi
  LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/lib/x86_64-linux-gnu/pulseaudio" \
    ./linuxdeployqt-continuous-x86_64.AppImage \
    Pencil2D/usr/share/applications/org.pencil2d.Pencil2D.desktop \
    -executable=Pencil2D/usr/plugins/ffmpeg \
    ${gst_executables} \
    -extra-plugins=platforms/libqwayland-egl.so,platforms/libqwayland-generic.so,\
platforms/libqwayland-xcomposite-egl.so,platforms/libqwayland-xcomposite-glx.so,\
wayland-decoration-client,wayland-graphics-integration-client,wayland-shell-integration \
    ${update_info} \
    -appimage
  local qtsuffix="-qt${INPUT_QT}"
  local output_name="pencil2d${qtsuffix/-qt5/}-linux-$3"
  mv Pencil2D*.AppImage "$output_name.AppImage"
  mv Pencil2D*.AppImage.zsync "$output_name.AppImage.zsync" \
    && sed -i '1,/^$/s/^\(Filename\|URL\): .*$/\1: '"$output_name.AppImage/" "$output_name.AppImage.zsync" \
    || true
  echo "output-basename=$output_name" >> "${GITHUB_OUTPUT}"
  echo "::endgroup::"
}

create_package_linux "" "" "nightly" 