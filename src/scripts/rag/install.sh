#!/bin/bash
TARGET_PATH=$1
echo "Install Path: $1"
SCRIPT_PATH=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_PATH")

mkdir -p "$TARGET_PATH"
cd "$TARGET_PATH" || { echo "failed to cd $TARGET_PATH"; exit 1; }
curl -L -O "https://mirror.nju.edu.cn/github-release/conda-forge/miniforge/LatestRelease/Miniforge3-$(uname)-$(uname -m).sh"
# Use expect to automatically process input
expect -c "
spawn bash Miniforge3-$(uname)-$(uname -m).sh
expect {
    \"Please, press ENTER to continue\" { send \"\r\"; send \"q\";exp_continue }
    \"Do you accept the license terms?\" { send \"yes\r\"; exp_continue }
    \"Miniforge3 will now be installed into this location:\" { send \"$TARGET_PATH/miniforge\r\"; exp_continue }
    \"conda config --set auto_activate_base false\" { send \"yes\r\"; exp_continue }
}
expect eof
"

source miniforge/etc/profile.d/conda.sh
conda env create -f $SCRIPT_DIR/env.yml
conda config --set auto_activate_base false

echo "Done!"
