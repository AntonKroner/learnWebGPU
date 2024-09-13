all: build webgpu.exe
build:
	cmake . -B build
webgpu.exe: build
	cmake --build build
vcpkg: vcpkg
	git clone https://github.com/Microsoft/vcpkg.git --depth=1 && \
	./vcpkg/bootstrap-vcpkg.sh -disableMetrics && \
	rm -rf ./vcpkg/.gi* ./vcpkg/docs && rm -f ./vcpkg/README* ./vcpkg/CONTRIBUTING* ./vcpkg/NOTICE*
install: vcpkg
	./vcpkg/vcpkg install --clean-after-build

