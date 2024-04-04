# Docker

To build the Linux version of Visual Pinball Standalone using a docker container:

```
docker buildx build --platform linux/amd64 . -t vpx-standalone-ubuntu:22.04
docker run -it --platform linux/amd64 --rm vpx-standalone-ubuntu:22.04

./build.sh
```

The supplied `Dockerfile` is based on Ubuntu 22.04, and includes Oh My Zsh, and the latest CMake. 

If you would like to do development, you can use Visual Studio Code and connect it to the container.

Install the C++ Extension Pack, and you should be all set.

You can then transfer to a Linux host distro such as Batocera using `rsync`:

```
cd /home/ubuntu/vpinball/build
rsync -avz --progress --exclude "CMake*" --exclude "cmake*" --exclude "Makefile" --exclude "*.a" --exclude ".cmake" --exclude "tables" --exclude "docs" --exclude "compile_commands.json" . root@192.168.1.201:/usr/bin/vpinball
```
