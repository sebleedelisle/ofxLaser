#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
ADDON_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd -P)"
OF_ROOT="$(cd "${ADDON_ROOT}/../.." && pwd -P)"
TMP_ROOT=${TMPDIR:-/tmp}/ofxlaser-smoke
PARALLEL_JOBS=${JOBS:-4}
BUILD_CORE_TARGET=${BUILD_CORE_TARGET:-Release}
EXAMPLE_TARGET=${EXAMPLE_TARGET:-ReleaseNoOF}
SKIP_EXAMPLES=${SKIP_EXAMPLES:-}

if [[ ! -f "${OF_ROOT}/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk" ]]; then
    echo "Could not locate openFrameworks root from ${ADDON_ROOT}" >&2
    exit 1
fi

shopt -s nullglob
examples=("${ADDON_ROOT}"/example_*)

if (( ${#examples[@]} == 0 )); then
    echo "No examples found under ${ADDON_ROOT}" >&2
    exit 1
fi

mkdir -p "${TMP_ROOT}"

echo "Building openFrameworks ${BUILD_CORE_TARGET} once"
make -C "${OF_ROOT}/libs/openFrameworksCompiled/project" "${BUILD_CORE_TARGET}" -j"${PARALLEL_JOBS}"

for example_dir in "${examples[@]}"; do
    [[ -d "${example_dir}" ]] || continue
    [[ -f "${example_dir}/addons.make" ]] || continue

    example_name=$(basename "${example_dir}")
    build_dir="${TMP_ROOT}/${example_name}"

    if [[ " ${SKIP_EXAMPLES} " == *" ${example_name} "* ]]; then
        echo
        echo "== ${example_name} =="
        echo "Skipping ${example_name}"
        continue
    fi

    echo
    echo "== ${example_name} =="

    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"

    cp -R "${example_dir}/src" "${build_dir}/"
    cp "${example_dir}/addons.make" "${build_dir}/addons.make"

    if [[ -f "${example_dir}/of.entitlements" ]]; then
        cp "${example_dir}/of.entitlements" "${build_dir}/of.entitlements"
    fi

    cat > "${build_dir}/Makefile" <<'EOF'
ifneq ($(wildcard config.make),)
	include config.make
endif

ifndef OF_ROOT
	OF_ROOT=$(realpath ../../..)
endif

include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk
EOF

    cat > "${build_dir}/config.make" <<EOF
OF_ROOT = ${OF_ROOT}
PROJECT_ROOT = .
EOF

    if [[ "$(uname -s)" == "Darwin" ]]; then
        cat >> "${build_dir}/config.make" <<'EOF'
export MAC_OS_MIN_VERSION = 10.15
export MAC_OS_CPP_VER = -std=c++17
EOF
    fi

    make -C "${build_dir}" clean >/dev/null
    make -C "${build_dir}" "${EXAMPLE_TARGET}" -j"${PARALLEL_JOBS}"
done

echo
echo "All ofxLaser examples built successfully"
