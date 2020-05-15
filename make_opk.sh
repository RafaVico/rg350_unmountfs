#!/bin/sh

OPK_NAME=unmountfs.opk

echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=UnmountFS OPK
Comment=Unmount opks from /mnt
Exec=rg350unmountfs
Terminal=false
Type=Application
StartupNotify=true
Icon=rg350unmountfs
Categories=applications;
Comment[es]=Desmonta opks de /mnt
EOF

# create opk
FLIST="data"
FLIST="${FLIST} rg350unmountfs"
FLIST="${FLIST} rg350unmountfs.png"
FLIST="${FLIST} default.gcw0.desktop"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop

