#!/usr/bin/env bash
#
# Fast local deploy for obs-plugin-countdown-adu (macOS).
#
# Builds ONLY the plugin target and copies + re-signs the resulting .plugin
# into the already-built OBS.app bundle, instead of rebuilding the whole
# `obs-studio` target (which is slow because Xcode re-embeds every plugin).
#
# Usage:
#   plugins/obs-plugin-countdown-adu/deploy-macos.sh [Release|Debug]
#
# Defaults to Release (the configuration your OBS.app is built with).
# Override the signing identity with CODESIGN_IDENT if needed.

set -euo pipefail

# Resolve repo root (this script lives in plugins/obs-plugin-countdown-adu/).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

CONFIG="${1:-Release}"
BUILD_DIR="${REPO_ROOT}/build_macos"
IDENTITY="${CODESIGN_IDENT:-OBS Dev}"
PLUGIN="obs-plugin-countdown-adu"

SRC="${BUILD_DIR}/plugins/${PLUGIN}/${CONFIG}/${PLUGIN}.plugin"
DEST_DIR="${BUILD_DIR}/frontend/${CONFIG}/OBS.app/Contents/PlugIns"

echo "==> Building ${PLUGIN} (${CONFIG})"
cmake --build "${BUILD_DIR}" --target "${PLUGIN}" --config "${CONFIG}"

if [[ ! -d "${SRC}" ]]; then
  echo "error: built plugin not found at ${SRC}" >&2
  exit 1
fi
if [[ ! -d "${DEST_DIR}" ]]; then
  echo "error: OBS.app PlugIns dir not found at ${DEST_DIR}" >&2
  echo "       (build the ${CONFIG} app once with: cmake --build ${BUILD_DIR} --target obs-studio --config ${CONFIG})" >&2
  exit 1
fi

echo "==> Copying into OBS.app"
rm -rf "${DEST_DIR}/${PLUGIN}.plugin"
cp -R "${SRC}" "${DEST_DIR}/"

echo "==> Re-signing with identity: ${IDENTITY}"
codesign --force --sign "${IDENTITY}" --timestamp=none --options runtime "${DEST_DIR}/${PLUGIN}.plugin"

echo "==> Done. Restart OBS to load the updated plugin."
