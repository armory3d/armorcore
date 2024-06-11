#!/usr/bin/env bash

. `dirname "$0"`/Tools/platform.sh
MAKE="`dirname "$0"`/Tools/bin/$KINC_PLATFORM/kmake"
exec $MAKE `dirname "$0"`/Tools/make.js "$@"
