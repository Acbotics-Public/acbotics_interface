#!/usr/bin/env bash

TAG_COMMIT=$(git rev-list --abbrev-commit --tags --max-count=1)
TAG=$(git describe --abbrev=0 --tags ${TAG_COMMIT} 2>/dev/null || true)
COMMIT=$(git rev-parse --short HEAD)
DATE=$(git log -1 --format=%cd --date=format:"%Y%m%d")
GIT_VERSION=${TAG#*v}
GIT_VERSION=${GIT_VERSION:-"none"}

if [[ "${COMMIT}" != "${TAG_COMMIT}" ]]; then
    GIT_VERSION=${GIT_VERSION}-next-${COMMIT}-${DATE}
fi

if [[ $(git status --porcelain) != "" ]]; then
    GIT_VERSION=${GIT_VERSION}-dirty
fi

echo $GIT_VERSION
