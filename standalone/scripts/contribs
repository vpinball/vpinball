#!/bin/bash

GITHUB_TOKEN=""

generate_commit_report() {
    REPO="$1"
    BRANCH="$2"

    echo "------"
    echo "REPO: ${REPO}"
    echo "BRANCH: ${BRANCH}"

    page=1
    all_logins=()

    while : ; do
        url="https://api.github.com/repos/$REPO/commits?sha=$BRANCH&per_page=100&page=$page"
        curl -s -H "Authorization: token $GITHUB_TOKEN" "$url" --output caches.json

        logins=$(jq -r 'map(if .author.login == null then .commit.author.name else .author.login end) | .[]' caches.json)

        if [ -z "$logins" ]; then
            break
        fi

        all_logins+=($logins)

        ((page++))
    done

    rm -f tmp-counts-all.txt

    for login in "${all_logins[@]}"; do
        echo $login >> tmp-counts-all.txt
    done

    awk '{counts[$0]++} END {for (user in counts) print counts[user], user}' tmp-counts-all.txt | sort -rn > tmp-counts-login.txt

    rm -f "tmp-counts-all.txt"

    awk '{print $2}' tmp-counts-login.txt | tr '\n' ',' | sed 's/,/, /g; s/, $//'

    rm -f tmp-counts-login.txt
    
    echo
}

generate_commit_report "vpinball/libaltsound" "master"
generate_commit_report "jsm174/libdof" "master"
generate_commit_report "zesinger/libserum" "main"
generate_commit_report "ppuc/libzedmd" "main"
generate_commit_report "vpinball/libdmdutil" "master"
generate_commit_report "vpinball/vpinball" "10.8.1"
generate_commit_report "vpinball/pinmame" "master"
