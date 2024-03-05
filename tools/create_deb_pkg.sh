#!/bin/bash

VERSION=0.1
PATCH=1

# Check qseed application is built
if [ ! -f ../build/qseed ]; then
    echo "Cannot find qseed application! First build the application."
    exit 1
fi

# Delete previously built package
rm -f qseed_$VERSION-$PATCH.deb

# Copy qseed application
mkdir -p qseed_$VERSION-$PATCH/usr/local/bin
cp ../build/qseed qseed_$VERSION-$PATCH/usr/local/bin

# Copy qseed configuration file
mkdir -p qseed_$VERSION-$PATCH/etc/qseed
cp qseed_config.yml qseed_$VERSION-$PATCH/etc/qseed

# Copy debian control file
mkdir -p qseed_$VERSION-$PATCH/DEBIAN
cp control qseed_$VERSION-$PATCH/DEBIAN

# Create debian package
dpkg-deb --build qseed_$VERSION-$PATCH

# Clean up folders
rm -rf qseed_$VERSION-$PATCH
