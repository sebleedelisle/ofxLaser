#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
ADDON_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd -P)"
OF_ROOT="${OF_ROOT:-$(cd "${ADDON_ROOT}/../.." && pwd -P)}"
PROJECT_GENERATOR="${PROJECT_GENERATOR:-${PG_BIN:-projectGenerator}}"
PG_EXAMPLE="${PG_EXAMPLE:-example_HelloLaser}"
PG_PROJECT_NAME="${PG_PROJECT_NAME:-ofxLaserProjectGeneratorSmoke}"
TMP_ROOT="${TMPDIR:-/tmp}/ofxlaser-projectgenerator-smoke"
PARALLEL_JOBS="${JOBS:-4}"

detect_platform() {
    case "$(uname -s)" in
        Darwin)
            echo "osx"
            ;;
        Linux)
            echo "linux64"
            ;;
        MINGW*|MSYS*|CYGWIN*)
            echo "msys2"
            ;;
        *)
            echo "Unsupported host platform: $(uname -s)" >&2
            exit 1
            ;;
    esac
}

PG_PLATFORM="${PG_PLATFORM:-$(detect_platform)}"

resolve_executable() {
    local executable="$1"

    if [[ "${executable}" == */* || "${executable}" == *\\* ]]; then
        printf '%s\n' "${executable}"
        return
    fi

    command -v "${executable}"
}

PG_BIN_PATH="$(resolve_executable "${PROJECT_GENERATOR}")"
if [[ ! -f "${PG_BIN_PATH}" ]]; then
    echo "Could not find projectGenerator executable: ${PROJECT_GENERATOR}" >&2
    exit 1
fi

if [[ ! -f "${OF_ROOT}/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk" ]]; then
    echo "Could not locate openFrameworks root from ${OF_ROOT}" >&2
    exit 1
fi

if [[ ! -d "${ADDON_ROOT}/${PG_EXAMPLE}/src" ]]; then
    echo "Could not locate example source: ${ADDON_ROOT}/${PG_EXAMPLE}/src" >&2
    exit 1
fi

BUILD_ROOT="${TMP_ROOT}/${PG_PLATFORM}/${PG_PROJECT_NAME}"
rm -rf "${BUILD_ROOT}"
mkdir -p "${BUILD_ROOT}"

cp -R "${ADDON_ROOT}/${PG_EXAMPLE}/src" "${BUILD_ROOT}/"
cp "${ADDON_ROOT}/${PG_EXAMPLE}/addons.make" "${BUILD_ROOT}/addons.make"

if [[ -f "${ADDON_ROOT}/${PG_EXAMPLE}/of.entitlements" ]]; then
    cp "${ADDON_ROOT}/${PG_EXAMPLE}/of.entitlements" "${BUILD_ROOT}/of.entitlements"
fi

OF_ROOT_FOR_PG="${OF_ROOT}"
BUILD_ROOT_FOR_PG="${BUILD_ROOT}"
if command -v cygpath >/dev/null 2>&1; then
    OF_ROOT_FOR_PG="$(cygpath -m "${OF_ROOT}")"
    BUILD_ROOT_FOR_PG="$(cygpath -m "${BUILD_ROOT}")"
fi

echo "Running projectGenerator for ${PG_PLATFORM}"
"${PG_BIN_PATH}" -o"${OF_ROOT_FOR_PG}" -p"${PG_PLATFORM}" -a"ofxLaser" "${BUILD_ROOT_FOR_PG}"

case "${PG_PLATFORM}" in
    osx)
        XCODE_PROJECT="${BUILD_ROOT}/${PG_PROJECT_NAME}.xcodeproj"
        if [[ ! -d "${XCODE_PROJECT}" ]]; then
            echo "Expected Xcode project not found: ${XCODE_PROJECT}" >&2
            exit 1
        fi
        xcodebuild \
            -project "${XCODE_PROJECT}" \
            -target "${PG_PROJECT_NAME}" \
            -configuration "${PG_BUILD_TARGET:-Release}" \
            -quiet \
            ONLY_ACTIVE_ARCH=YES \
            ARCHS="$(uname -m)" \
            CODE_SIGN_IDENTITY=-
        ;;
    linux*|msys2)
        if [[ ! -f "${BUILD_ROOT}/Makefile" ]]; then
            echo "Expected Makefile not found: ${BUILD_ROOT}/Makefile" >&2
            exit 1
        fi
        make -C "${BUILD_ROOT}" "${PG_BUILD_TARGET:-${EXAMPLE_TARGET:-ReleaseNoOF}}" -j"${PARALLEL_JOBS}"
        ;;
    *)
        echo "Generation completed for ${PG_PLATFORM}; no build command configured"
        ;;
esac

echo "projectGenerator smoke test passed for ${PG_PLATFORM}"
