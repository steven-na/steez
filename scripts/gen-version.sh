#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

build_type="${1:-release}"
out="src/version_gen.h"
tmp="$(mktemp)"

tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
version="${tag#v}"
major="${version%%.*}"
rest="${version#*.}"
minor="${rest%%.*}"
patch="${rest#*.}"
patch="${patch%%-*}"

commit=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
if ! git diff --quiet 2>/dev/null || ! git diff --cached --quiet 2>/dev/null; then
    commit="${commit}-dirty"
fi

cat > "$tmp" <<EOF
#pragma once

#define STEEZ_VERSION_MAJOR $major
#define STEEZ_VERSION_MINOR $minor
#define STEEZ_VERSION_PATCH $patch
#define STEEZ_VERSION_STRING "$version"
#define STEEZ_GIT_COMMIT "$commit"
#define STEEZ_BUILD_DATE "$(date +%Y-%m-%d)"
#define STEEZ_BUILD_TIME "$(date +%H:%M:%S)"
#define STEEZ_BUILD_TYPE "$build_type"
EOF

if ! cmp -s "$tmp" "$out" 2>/dev/null; then
    mv "$tmp" "$out"
else
    rm "$tmp"
fi
