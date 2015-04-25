#!/bin/bash
TAG=`git describe --tags --long`
BRANCH=`git rev-parse --abbrev-ref HEAD`

if [ "${BRANCH}" != "master" ]; then
    echo "${TAG}-${BRANCH}"
else
    echo "${TAG}"
fi

