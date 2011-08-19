#!/bin/sh
cat <<EOF
[Desktop Entry]
Version=0.20
Type=Application
Name=OpenDCP
GenericName=Blog editor
Comment=OpenDCP Digital Cinema Package Creator
tryExec=$1/bin/opendcp
Exec=$1/bin/opendcp &
Categories=Application;AudioVideo;Video;
Icon=$1/share/icons/opendcp.png
EOF
