#!/bin/sh
# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

## Note about shellcheck SC2086
## "Double-quote arguments to prevent globbing and word-splitting"
## https://github.com/koalaman/shellcheck/wiki/SC2086
## In few cases the $XXX_PARAMS argument is NOT double-quoted
## on purpose: it might be empty, and it might have several
## distinct arguments to be passed to other scripts (e.g. "-i -t -D").
##
## As this is done purposfully, and the content of $XXX_PARAMS
## is closely controlled, an explicit 'shellcheck disable SC2086'
## was added to the relevant lines.

set -u

die()
{
  base=$(basename "$0")
  echo "$base: error: $*" >&2
  exit 1
}

show_help_and_exit()
{
  base=$(basename "$0")
  echo "CacheLib dependencies builder

usage: $base [-BdhiIjStv] NAME

options:
  -B    skip build step
        (default is to build with cmake & make)
  -d    build with DEBUG configuration
        (default is RELEASE with debug information)
  -h    This help screen
  -i    install after build using 'sudo make install'
        (default is to build but not install)
  -I    Add ittapi to your build
        (VTune profiling abilities)
  -j    build using all available CPUs ('make -j')
        (default is to use single CPU)
  -S    skip git-clone/git-pull step
        (default is to get the latest source)
  -t    build tests
        (default is to skip tests if supported by the package)
  -v    verbose build

NAME: the dependency to build supported values are:
  zstd
  googlelog, googleflags, googletest,
  fmt, sparsemap,
  folly, fizz, wangle, fbthrift, ittapi,
  cachelib

  "
  exit 0
}

###############################
## Parse Commandline arguments
###############################
install=
build=yes
source=yes
debug_build=
build_tests=
show_help=
many_jobs=
add_itt=
verbose=
while getopts :BSIdhijtv param
do
  case $param in
    i) install=yes ;;
    B) build= ;;
    I) add_itt=yes ;;
    S) source= ;;
    h) show_help=yes ;;
    d) debug_build=yes ;;
    v) verbose=yes ;;
    j) many_jobs=yes ;;
    t) build_tests=yes ;;
    ?) die "unknown option. See -h for help."
  esac
done
test -n "$show_help" && show_help_and_exit;
shift $((OPTIND-1))

test "$#" -eq 0 \
    && die "missing dependancy name to build. See -h for help"



######################################
## Check which dependecy was requested
######################################

external_git_clone=
external_git_branch=
external_git_tag=
update_submodules=
cmake_custom_params=

case "$1" in
  googlelog)
    NAME=glog
    REPO=https://github.com/google/glog
    REPODIR=cachelib/external/$NAME
    SRCDIR=$REPODIR
    external_git_clone=yes
    external_git_tag="v0.5.0"
    cmake_custom_params="-DBUILD_SHARED_LIBS=ON"
    ;;

  googleflags)
    NAME=gflags
    REPO=https://github.com/gflags/gflags
    REPODIR=cachelib/external/$NAME
    SRCDIR=$REPODIR
    external_git_clone=yes
    external_git_tag="v2.2.2"
    cmake_custom_params="-DGFLAGS_BUILD_SHARED_LIBS=YES"
    if test "$build_tests" = "yes" ; then
        cmake_custom_params="$cmake_custom_params -DGFLAGS_BUILD_TESTING=YES"
    else
        cmake_custom_params="$cmake_custom_params -DGFLAGS_BUILD_TESTING=NO"
    fi
    ;;

  googletest)
    NAME=googletest
    REPO=https://github.com/google/googletest.git
    REPODIR=cachelib/external/$NAME
    SRCDIR=$REPODIR
    # Work-around: build googletest with DEBUG information
    # results in CMake problems in detecting the library.
    # See https://gitlab.kitware.com/cmake/cmake/-/issues/17799
    # Disable debug build, even if requested.
    if test "$debug_build" ; then
        echo "Note: disabling DEBUG mode for googletest build"
        debug_build=
    fi
    cmake_custom_params="-DBUILD_SHARED_LIBS=ON"
    external_git_clone=yes
    ;;

  fmt)
    NAME=fmt
    REPO=https://github.com/fmtlib/fmt.git
    REPODIR=cachelib/external/$NAME
    SRCDIR=$REPODIR
    external_git_clone=yes
    cmake_custom_params="-DBUILD_SHARED_LIBS=ON"
    if test "$build_tests" = "yes" ; then
        cmake_custom_params="$cmake_custom_params -DFMT_TEST=YES"
    else
        cmake_custom_params="$cmake_custom_params -DFMT_TEST=NO"
    fi
    ;;

  zstd)
    NAME=zstd
    REPO=https://github.com/facebook/zstd
    REPODIR=cachelib/external/$NAME
    SRCDIR=$REPODIR/build/cmake
    external_git_clone=yes
    external_git_branch=release
    if test "$build_tests" = "yes" ; then
        cmake_custom_params="-DZSTD_BUILD_TESTS=ON"
    else
        cmake_custom_params="-DZSTD_BUILD_TESTS=OFF"
    fi
    ;;

  sparsemap)
    NAME=sparsemap
    REPO=https://github.com/Tessil/sparse-map.git
    REPODIR=cachelib/external/$NAME
    SRCDIR=$REPODIR
    external_git_clone=yes
    ;;

  folly)
    NAME=folly
    SRCDIR=cachelib/external/$NAME
    update_submodules=yes
    cmake_custom_params="-DBUILD_SHARED_LIBS=ON"
    if test "$build_tests" = "yes" ; then
        cmake_custom_params="$cmake_custom_params -DBUILD_TESTS=ON"
    else
        cmake_custom_params="$cmake_custom_params -DBUILD_TESTS=OFF"
    fi
    ;;

  fizz)
    NAME=fizz
    SRCDIR=cachelib/external/$NAME/$NAME
    update_submodules=yes
    cmake_custom_params="-DBUILD_SHARED_LIBS=ON"
    if test "$build_tests" = "yes" ; then
        cmake_custom_params="$cmake_custom_params -DBUILD_TESTS=ON"
    else
        cmake_custom_params="$cmake_custom_params -DBUILD_TESTS=OFF"
    fi
    ;;

  wangle)
    NAME=wangle
    SRCDIR=cachelib/external/$NAME/$NAME
    update_submodules=yes
    cmake_custom_params="-DBUILD_SHARED_LIBS=ON"
    if test "$build_tests" = "yes" ; then
        cmake_custom_params="$cmake_custom_params -DBUILD_TESTS=ON"
    else
        cmake_custom_params="$cmake_custom_params -DBUILD_TESTS=OFF"
    fi
    ;;

  fbthrift)
    NAME=fbthrift
    SRCDIR=cachelib/external/$NAME
    update_submodules=yes
    cmake_custom_params="-DBUILD_SHARED_LIBS=ON"
    ;;

  ittapi)
    NAME=ittapi
    REPO=https://github.com/intel/ittapi.git
    REPODIR=cachelib/external/$NAME
    SRCDIR=$REPODIR
    external_git_clone=yes
    ;;

  cachelib)
    NAME=cachelib
    SRCDIR=cachelib
    if test "$build_tests" = "yes" ; then
        cmake_custom_params="$cmake_custom_params -DBUILD_TESTS=ON"
    else
        cmake_custom_params="$cmake_custom_params -DBUILD_TESTS=OFF"
    fi
    ;;

  *) die "unknown dependency '$1'. See -h for help."
esac

####################################
## Calculate cmake/make parameters
####################################

CMAKE_PARAMS="$cmake_custom_params"
test "$debug_build" \
  && CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_BUILD_TYPE=Debug" \
  || CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_BUILD_TYPE=RelWithDebInfo"


MAKE_PARAMS=
test "$verbose" && MAKE_PARAMS="$MAKE_PARAMS VERBOSE=YES"

JOBS=$(nproc --ignore 1)
test "$many_jobs" && MAKE_PARAMS="$MAKE_PARAMS -j$JOBS"




####################################
# Real work starts here
####################################


dir=$(dirname "$0")
cd "$dir/.." || die "failed to change-dir into $dir/.."
test -d cachelib || die "expected 'cachelib' directory not found in $PWD"


# After ensuring we are in the correct directory, set the installation prefix"
PREFIX="$PWD/opt/cachelib/"
CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_INSTALL_PREFIX=$PREFIX"
CMAKE_PREFIX_PATH="$PREFIX/lib/cmake:$PREFIX/lib64/cmake:$PREFIX/lib:$PREFIX/lib64:$PREFIX:${CMAKE_PREFIX_PATH:-}"
export CMAKE_PREFIX_PATH
PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PREFIX/lib64/pkgconfig:${PKG_CONFIG_PATH:-}"
export PKG_CONFIG_PATH
LD_LIBRARY_PATH="$PREFIX/lib:$PREFIX/lib64:${LD_LIBRARY_PATH:-}"
export LD_LIBRARY_PATH
PATH="$PREFIX/bin:$PATH"
export PATH

##
## Update the latest source code
##

if test "$source" ; then

  if test "$external_git_clone" ; then

    # This is an external (non-facebook) project, clone/pull it.
    if test -d "$SRCDIR" ; then
      # cloned repository already exists, update it, unless we're on a specific tag
      ( cd "$SRCDIR" && git fetch --all ) \
        || die "failed to fetch git repository for '$NAME' in '$SRCDIR'"
    else
      # Clone new repository directory
      git clone "$REPO" "$REPODIR" \
        || die "failed to clone git repository $REPO to '$REPODIR'"
    fi


    # switch to specific branch/tag if needed
    if test "$external_git_branch" ; then
        ( cd "$REPODIR" \
           && git checkout --force "origin/$external_git_branch" ) \
           || die "failed to checkout branch $external_git_branch in $REPODIR"
    elif test "$external_git_tag" ; then
        ( cd "$REPODIR" \
           && git checkout --force "$external_git_tag" ) \
           || die "failed to checkout tag $external_git_tag in $REPODIR"
    fi

  fi

  if test "$update_submodules" ; then
    ./contrib/update-submodules.sh || die "failed to update git-submodules"
  fi
fi


## If we do not build or install (only download the source code),
## exit now.
if test -z "$build" && test -z "$install" ; then
  echo "Source for '$NAME' downloaded/updated"
  exit 0
fi


# The build directory is needed for both build and installation
mkdir -p "build-$NAME" || die "failed to create build-$NAME directory"
cd "build-$NAME" || die "'cd' failed"

# Grab the location of the build-ittapi folder
if test "$add_itt" ; then
  RED='\033[1;31m'
  GREEN='\033[1;32m'
  NC='\033[0m' # No Color
  ITTDIR=$(find / -type d -name build-ittapi 2> /dev/null)
  [ -d "$ITTDIR" ] && echo -e "${GREEN}Directory '$ITTDIR' exists.${NC}" || echo -e "${RED}Error: Directory '$ITTDIR' does not exists yet.${NC}"
fi

##
## Build
##
if test "$build" ; then
  # If add_itt is true and we are in the build-ittapi folder; we will then create the .a library files.
  if test "$add_itt" = yes && [ "$PWD" = "$ITTDIR" ] ; then
    python3 ../cachelib/external/ittapi/buildall.py -c
    cd ../build-ittapi
    cp ../cachelib/external/ittapi/CMakeLists.txt .
    cp -R ../cachelib/external/ittapi/include .
    python3 ../cachelib/external/ittapi/buildall.py -d -v
    cp build_linux/32/bin/libittnotify.a ../opt/cachelib/lib
    cp build_linux/64/bin/libittnotify.a ../opt/cachelib/lib64
  # else we will continue making the other build files.
  elif [ ! "$PWD" = "$ITTDIR" ] ; then
    # shellcheck disable=SC2086
    cmake $CMAKE_PARAMS "../$SRCDIR" || die "cmake failed on $SRCDIR"
    # shellcheck disable=SC2086
    nice make $MAKE_PARAMS || die "make failed"
  fi
fi

## If no install requested, exit now
if test -z "install" ; then
  echo "'$NAME' is now built in build-$NAME (source in $REPODIR)"
  exit 0
fi


##
## Install
##

if test "$install" ; then
  if [ ! "$PWD" = "$ITTDIR" ] ; then
    # shellcheck disable=SC2086
    make $MAKE_PARAMS install || die "make install failed"
  fi
fi

echo "'$NAME' is now installed"
