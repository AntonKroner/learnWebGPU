other dependencies: 
sudo apt install xorg-dev
sudo apt install xcb libxcb-xkb-dev x11-xkb-utils libx11-xcb-dev libxkbcommon-x11-dev

"vcpkgsetup": "git clone https://github.com/Microsoft/vcpkg.git && ./vcpkg/bootstrap-vcpkg.sh -disableMetrics && rm -rf ./vcpkg/.gi* ./vcpkg/docs && rm -f ./vcpkg/README* ./vcpkg/CONTRIBUTING* ./vcpkg/NOTICE*",
"vcpkguninstall": "rm -rf vcpkg && rm -rf vcpkg_installed",
"vcpkg": "./vcpkg/vcpkg",
"vcpkginstall": "./vcpkg/vcpkg install --clean-after-build"
PATH="./vcpkg:$PATH"

build:
cmake . -B build
cmake --build build
