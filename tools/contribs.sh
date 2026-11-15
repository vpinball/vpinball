#!/bin/bash

# Prints the contributor list for each credited repository, ordered by commit count,
# in the format used by the Credits section of the mobile apps.
#
# Uses GITHUB_TOKEN if set, otherwise the token from `gh auth token`.

TOKEN="${GITHUB_TOKEN:-$(gh auth token 2>/dev/null)}"

if [ -z "${TOKEN}" ]; then
    echo "error: set GITHUB_TOKEN or run 'gh auth login'" >&2
    exit 1
fi

ALIASES=(
    "Andre Michi=andremichi"
    "rockfordroeNG=RockfordRoe"
)

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

apply_aliases() {
    local args=()
    for alias in "${ALIASES[@]}"; do
        args+=(-e "s/^${alias%%=*}\$/${alias#*=}/")
    done
    sed "${args[@]}"
}

generate_commit_report() {
    REPO="$1"
    BRANCH="$2"

    echo "------"
    echo "REPO: ${REPO}"
    echo "BRANCH: ${BRANCH}"

    page=1
    : > "${TMP_DIR}/all.txt"

    while : ; do
        url="https://api.github.com/repos/${REPO}/commits?sha=${BRANCH}&per_page=100&page=${page}"
        status=$(curl -s -w '%{http_code}' -H "Authorization: Bearer ${TOKEN}" "${url}" --output "${TMP_DIR}/page.json")

        if [ "${status}" != "200" ]; then
            echo "error: ${REPO} page ${page} returned HTTP ${status}: $(jq -r '.message? // empty' "${TMP_DIR}/page.json" 2>/dev/null)" >&2
            exit 1
        fi

        count=$(jq 'length' "${TMP_DIR}/page.json")
        if [ "${count}" = "0" ]; then
            break
        fi

        jq -r '.[] | (.author.login // .commit.author.name)' "${TMP_DIR}/page.json" >> "${TMP_DIR}/all.txt"

        ((page++))
    done

    grep -v -e '\[bot\]$' -e '^No Author$' "${TMP_DIR}/all.txt" \
        | apply_aliases \
        | sort | uniq -c | sort -k1,1rn -k2 -f \
        | sed 's/^ *[0-9]* //' \
        | paste -sd ',' - | sed 's/,/, /g'

    echo
}

generate_commit_report "vpinball/vpinball" "master"
generate_commit_report "vpinball/pinmame" "master"
generate_commit_report "vpinball/libaltsound" "master"
generate_commit_report "vpinball/libdmdutil" "master"
generate_commit_report "PPUC/libzedmd" "main"
generate_commit_report "PPUC/libserum" "main"
generate_commit_report "vpinball/libdof" "master"
generate_commit_report "PPUC/libvni" "main"
generate_commit_report "vpinball/libwinevbs" "master"
