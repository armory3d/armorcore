#!/usr/bin/env bash

. `dirname "$0"`/tools/platform.sh
MAKE="`dirname "$0"`/tools/bin/$KINC_PLATFORM/kmake"
exec $MAKE `dirname "$0"`/tools/make.js "$@"
