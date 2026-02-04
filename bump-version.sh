#!/bin/bash

# Script to bump version for SingleStore ODBC Connector
# Usage: ./bump-version.sh <new_version>
# Example: ./bump-version.sh 1.2.2
#          ./bump-version.sh 1.2.3-beta
#          ./bump-version.sh 1.3.0-rc1

set -euo pipefail

if [ $# -ne 1 ]; then
    echo "Usage: $0 <new_version>"
    echo "Example: $0 1.2.2"
    echo "        $0 1.3.0-rc1"
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

echo "Bumping version to $NEW_VERSION (base: $BASE_VERSION, quality: $QUALITY)..."

# Update CMakeLists.txt (using BSD sed syntax for macOS compatibility)
sed -i.bak "s/SET(SS_ODBC_VERSION_MAJOR [0-9][0-9]*)/SET(SS_ODBC_VERSION_MAJOR $MAJOR)/" CMakeLists.txt
sed -i.bak "s/SET(SS_ODBC_VERSION_MINOR [0-9][0-9]*)/SET(SS_ODBC_VERSION_MINOR $MINOR)/" CMakeLists.txt
sed -i.bak "s/SET(SS_ODBC_VERSION_PATCH [0-9][0-9]*)/SET(SS_ODBC_VERSION_PATCH $PATCH)/" CMakeLists.txt
sed -i.bak "s/SET(SS_ODBC_VERSION_QUALITY \"[^\"]*\")/SET(SS_ODBC_VERSION_QUALITY \"$QUALITY\")/" CMakeLists.txt
sed -i.bak "s/SET(SS_ODBC_VERSION \"[0-9.][0-9.]*\")/SET(SS_ODBC_VERSION \"$FORMATTED_VERSION\")/" CMakeLists.txt

# Update publish.yml
sed -i.bak "s/DRIVER_VERSION: [0-9][0-9.]*\(-[a-zA-Z0-9][a-zA-Z0-9]*\)*/DRIVER_VERSION: $NEW_VERSION/" .github/workflows/publish.yml

# Remove backup files
rm -f CMakeLists.txt.bak .github/workflows/publish.yml.bak

echo "Updated CMakeLists.txt and publish.yml"
echo ""
echo "Changes made:"
git --no-pager diff CMakeLists.txt .github/workflows/publish.yml

# Prompt for confirmation
read -p "Do you want to commit and push these changes? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    # Commit changes
    git add CMakeLists.txt .github/workflows/publish.yml
    git commit -m "Bump version to $NEW_VERSION"
`
    # Create tag
    git tag "v$NEW_VERSION"

    # Push changes and tag
    git push
    git push origin "v$NEW_VERSION"

    echo "✓ Version bumped to $NEW_VERSION"
    echo "✓ Committed and pushed to remote"
    echo "✓ Tag v$NEW_VERSION created and pushed"
else
    echo "Changes not committed. You can review them with 'git diff'"
    exit 1
fi
