#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
cd "${SCRIPT_DIR}/.."
ROOT_DIR="$(pwd)"
OUT="$ROOT_DIR/submission.zip"
BASE_COMMIT="9e0b8c5d366a864a0298efd122bb6504a1cb6bdb"
COMMIT_TARGET="commits.txt"
DIR_TARGETS=('bootblock' 'include' 'kernel' 'tools' 'user' 'scripts')
FILE_TARGETS=('CMakeLists.txt' $COMMIT_TARGET 'README')

git log "$BASE_COMMIT..HEAD" > $COMMIT_TARGET
zip -r "$OUT" ${DIR_TARGETS[@]} ${FILE_TARGETS[@]}
rm $COMMIT_TARGET
