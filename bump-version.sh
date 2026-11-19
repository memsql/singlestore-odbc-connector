#!/bin/bash

# Script to bump version for SingleStore ODBC Connector
# Usage: ./bump-version.sh <new_version>          Step 1: open a version bump PR
#        ./bump-version.sh --tag <new_version>    Step 2: tag master after the PR is merged
# Example: ./bump-version.sh 1.2.2
#          ./bump-version.sh 1.2.3-beta
#          ./bump-version.sh --tag 1.2.2
#
# `master` is protected, so the version bump goes through a pull request.
# Step 1 updates CMakeLists.txt on a new branch, pushes it, and opens a PR.
# Step 2 (after the PR is merged) tags master and pushes the tag, which
# triggers the Publish installers workflow.
# See RELEASING.md for the full release process.

set -euo pipefail

TAG_ONLY=false
if [ "${1:-}" = "--tag" ]; then
    TAG_ONLY=true
    shift
fi

if [ $# -ne 1 ]; then
    echo "Usage: $0 <new_version>          # open a version bump PR"
    echo "       $0 --tag <new_version>    # tag master after the PR is merged"
    echo "Example: $0 1.2.2"
    echo "         $0 1.3.0-rc1"
    exit 1
fi

NEW_VERSION=$1

# Validate version format (X.Y.Z or X.Y.Z-suffix)
if ! [[ $NEW_VERSION =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9]+)?$ ]]; then
    echo "Error: Version must be in format X.Y.Z or X.Y.Z-suffix (e.g., 1.2.2 or 1.2.3-beta)"
    exit 1
fi

# Split version into base and quality/suffix
if [[ $NEW_VERSION =~ ^([0-9]+\.[0-9]+\.[0-9]+)(-(.+))?$ ]]; then
    BASE_VERSION="${BASH_REMATCH[1]}"
    QUALITY="${BASH_REMATCH[3]}"
else
    echo "Error: Failed to parse version"
    exit 1
fi

# Default quality to "ga" if not specified
if [ -z "$QUALITY" ]; then
    QUALITY="ga"
fi

# Split base version into components
IFS='.' read -r MAJOR MINOR PATCH <<< "$BASE_VERSION"

# Format version with leading zeros for SS_ODBC_VERSION
FORMATTED_VERSION=$(printf "%02d.%02d.%04d" $MAJOR $MINOR $PATCH)

if $TAG_ONLY; then
    # Tag the merged bump commit on master. Tag pushes are not blocked by the
    # branch ruleset, and the tag push starts the release workflow.
    git fetch origin master --tags
    if [ "$(git rev-parse --abbrev-ref HEAD)" != "master" ] || \
       [ "$(git rev-parse HEAD)" != "$(git rev-parse origin/master)" ]; then
        echo "Error: check out master and make sure it is up to date with origin/master (git pull)"
        exit 1
    fi
    if ! grep -q "SET(SS_ODBC_VERSION_MAJOR $MAJOR)" CMakeLists.txt || \
       ! grep -q "SET(SS_ODBC_VERSION_MINOR $MINOR)" CMakeLists.txt || \
       ! grep -q "SET(SS_ODBC_VERSION_PATCH $PATCH)" CMakeLists.txt || \
       ! grep -q "SET(SS_ODBC_VERSION_QUALITY \"$QUALITY\")" CMakeLists.txt; then
        echo "Error: CMakeLists.txt on master does not match $NEW_VERSION. Merge the version bump PR first."
        exit 1
    fi
    read -p "Tag master at $(git rev-parse --short HEAD) as v$NEW_VERSION and push the tag? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git tag "v$NEW_VERSION"
        git push origin "v$NEW_VERSION"
        echo "✓ Tag v$NEW_VERSION created and pushed"
        echo "✓ Publish installers workflow should now be running"
        echo "  See RELEASING.md for next steps (edit the pre-release and mark it as latest)"
        exit 0
    fi
    echo "Tag not created."
    exit 1
fi

echo "Bumping version to $NEW_VERSION (base: $BASE_VERSION, quality: $QUALITY)..."

# Update CMakeLists.txt (using BSD sed syntax for macOS compatibility)
sed -i.bak "s/SET(SS_ODBC_VERSION_MAJOR [0-9][0-9]*)/SET(SS_ODBC_VERSION_MAJOR $MAJOR)/" CMakeLists.txt
sed -i.bak "s/SET(SS_ODBC_VERSION_MINOR [0-9][0-9]*)/SET(SS_ODBC_VERSION_MINOR $MINOR)/" CMakeLists.txt
sed -i.bak "s/SET(SS_ODBC_VERSION_PATCH [0-9][0-9]*)/SET(SS_ODBC_VERSION_PATCH $PATCH)/" CMakeLists.txt
sed -i.bak "s/SET(SS_ODBC_VERSION_QUALITY \"[^\"]*\")/SET(SS_ODBC_VERSION_QUALITY \"$QUALITY\")/" CMakeLists.txt
sed -i.bak "s/SET(SS_ODBC_VERSION \"[0-9.][0-9.]*\")/SET(SS_ODBC_VERSION \"$FORMATTED_VERSION\")/" CMakeLists.txt

# Remove backup files
rm -f CMakeLists.txt.bak

echo "Updated CMakeLists.txt"
echo ""
echo "Changes made:"
git --no-pager diff CMakeLists.txt

# Prompt for confirmation
read -p "Do you want to commit these changes on a new branch and open a PR? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    BRANCH="chore/bump-version-$NEW_VERSION"
    git checkout -b "$BRANCH"
    git add CMakeLists.txt
    git commit -m "chore: bump version to $NEW_VERSION"
    git push -u origin "$BRANCH"

    if command -v gh >/dev/null 2>&1; then
        gh pr create --base master --head "$BRANCH" \
            --title "chore: bump version to $NEW_VERSION" \
            --body "Bump version to $NEW_VERSION ahead of the release. After merging, run \`./bump-version.sh --tag $NEW_VERSION\` on master."
    else
        echo "gh CLI not found: open a PR for branch $BRANCH manually."
    fi

    echo "✓ Version bump committed on $BRANCH and pushed"
    echo "  Next: get the PR approved and merged, then on master run:"
    echo "    ./bump-version.sh --tag $NEW_VERSION"
else
    echo "Changes not committed. You can review them with 'git diff'"
    exit 1
fi
