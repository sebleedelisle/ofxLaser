#!/bin/zsh

set -euo pipefail

SCRIPT_DIR=${0:A:h}
ADDON_ROOT=${SCRIPT_DIR:h}
OF_ROOT=${ADDON_ROOT:h:h}
TMP_ROOT=${TMPDIR:-/tmp}/ofxlaser-smoke
PARALLEL_JOBS=${JOBS:-4}

if [[ ! -f "${OF_ROOT}/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk" ]]; then
    echo "Could not locate openFrameworks root from ${ADDON_ROOT}" >&2
    exit 1
fi

examples=("${ADDON_ROOT}"/example_*)

if (( ${#examples[@]} == 0 )); then
    echo "No examples found under ${ADDON_ROOT}" >&2
    exit 1
fi

mkdir -p "${TMP_ROOT}"

echo "Building openFrameworks Release once"
make -C "${OF_ROOT}/libs/openFrameworksCompiled/project" Release

for example_dir in "${examples[@]}"; do
    example_name=${example_dir:t}
    build_dir="${TMP_ROOT}/${example_name}"

    echo
    echo "== ${example_name} =="

    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"

    ln -s "${example_dir}/src" "${build_dir}/src"
    ln -s "${example_dir}/addons.make" "${build_dir}/addons.make"

    if [[ -f "${example_dir}/of.entitlements" ]]; then
        ln -s "${example_dir}/of.entitlements" "${build_dir}/of.entitlements"
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
export MAC_OS_MIN_VERSION = 10.15
export MAC_OS_CPP_VER = -std=c++17
PROJECT_ROOT = .
EOF

    make -C "${build_dir}" clean >/dev/null
    make -C "${build_dir}" ReleaseNoOF -j"${PARALLEL_JOBS}"
done

echo
echo "All ofxLaser examples built successfully"
