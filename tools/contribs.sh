#!/bin/bash

# Prints the contributor list for each credited repository, ordered by commit count,
# in the format used by the Credits section of the mobile apps.
#
# Uses GITHUB_TOKEN if set, otherwise the token from `gh auth token`.

TOKEN="${GITHUB_TOKEN:-$(gh auth token 2>/dev/null)}"

generate_commit_report() {
    REPO="$1"
    BRANCH="$2"

    echo "------"
    echo "REPO: ${REPO}"
    echo "BRANCH: ${BRANCH}"

    page=1
    : > tmp-contribs-all.txt

    while : ; do
        url="https://api.github.com/repos/${REPO}/commits?sha=${BRANCH}&per_page=100&page=${page}"
        curl -s -H "Authorization: Bearer ${TOKEN}" "${url}" --output tmp-contribs-page.json

        count=$(jq 'if type == "array" then length else 0 end' tmp-contribs-page.json)
        if [ "${count}" = "0" ]; then
            break
        fi

        jq -r '.[] | (.author.login // .commit.author.name)' tmp-contribs-page.json >> tmp-contribs-all.txt

        ((page++))
    done

    grep -v -e '\[bot\]$' -e '^No Author$' tmp-contribs-all.txt \
        | sort | uniq -c | sort -rn \
        | sed 's/^ *[0-9]* //' \
        | paste -sd ',' - | sed 's/,/, /g'

    rm -f tmp-contribs-all.txt tmp-contribs-page.json

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
