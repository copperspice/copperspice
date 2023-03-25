# Script to build a rpm package for CopperSpice
#
set -e

#  Step 1 : Protect user from themselves
#
if [ ! -d "/etc/rpmdevtools" ]; then
    if [ ! "$(grep -i 'REDHAT' /etc/*release)" ]; then
        echo "This script can only be run on REDHAT based distribution with rpmdevtools installed"
        exit 1
    fi
fi

echo "MUST BE RUN FROM ROOT OF PROJECT DIRECTORY TREE"
echo ""
echo "YOU MUST HAVE RUN rpmdev-setuptree one time from the command line prior to running this."
echo "that creates a permanent $HOME/rpmbuild directory tree. You only ever need to run"
echo "that command once."
echo ""
echo "This script ASSUMES it can create and use copperspice_rpm_build"
echo "directory one level up from where this script is being run. If directory"
echo "exist it will be deleted and recreated. The only reason the build directory is"
echo "created is so cmake can populate variable values in the copperspice.spec file."
echo ""
echo "Script also ASSUMES you are running from the root of the Git project directory"
echo "where all source is in a directory named src at the same level as this file."
echo ""
echo "You must have ninja and a valid build environment. "
echo ""
echo "After creating fresh directory this script will run the cmake"
echo "command to populate the copperspice_rpm_build directory. After that it will use the "
echo "rpmbuild command to actually create the RPM package."
echo ""

#  Step 2 : Establish fresh clean directories
#
echo "*** Establishing fresh directories"
SCRIPT_DIR="$PWD"
BUILD_DIR="$SCRIPT_DIR/../copperspice_rpm_build"
INSTALL_DIR="/usr"

echo "SCRIPT_DIR                $SCRIPT_DIR"
echo "BUILD_DIR                 $BUILD_DIR"
echo "INSTALL_DIR               $INSTALL_DIR"

#  nuke the directories we will use if they already exist
#
if [ -d "$BUILD_DIR" ]; then
  rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

#  save working directory just in case
#
pushd `pwd`

#  Step 3 : Prepare build directory
#           We do this just to get copperspice.spec with all of the CMake variables filled in.
#
echo "*** Prepping build directory"
cd "$BUILD_DIR"
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release \
      -DBUILDING_RPM=ON \
      -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
      -DCMAKE_SKIP_BUILD_RPATH=TRUE \
      "$SCRIPT_DIR"

#  Step 4 : build copperpice and generate the RPM
#
rpmbuild -ba copperspice.spec

echo ""
echo "If this completed without errors you will find the RPM in {$HOME}/rpmbuild/RPMS"
echo ""

popd


set -e

exit 0
