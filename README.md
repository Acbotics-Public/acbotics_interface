# Acbotics AcSense Interface Library

The Acbotics AcSense Interface Library provides client-side utilities for working with acoustic and non-acoustic sensor data streams produced by the Acbotics AcSense. The library is written in C++, and also provides Python bindings for ease of use in new application development.

This repository replaces an earlier interface written entirely in Python, which remains available as a public archive at [https://github.com/Acbotics-Public/acbotics_interface_py.git](https://github.com/Acbotics-Public/acbotics_interface_py.git).


# Features

The Acbotics AcSense interface library includes:

- Device-specific UDP data protocols
- Multi-threaded architecture for data intake & processing:
  - Includes mutex-controlled thread-safe queues
  - Audio file logging (FLAC, WAV) powered by libsndfile
  - FFT processing powered by pocketfft
- Python bindings for ease of use in extension utilities (e.g. the AcSense Display)

Additional proprietary extensions available from Acbotics:
- Broadband beamformer with controllable parameters (NFFT, frequency boundaries, element coordinates)
- Energy detector monitors array signals for increased levels, indicative of potential new contacts
- Contact manager & tracker


# Using the library


## Building

The `build.sh` script can be used to compile the package for the host architecture; dependencies must be installed manually. These are captured in the Dockerfile entries in `./docker`, and include the following (Ubuntu/Debian package names shown):

- libgoogle-glog-dev
- libsndfile1-dev
- libgps-dev
- libeigen3-dev

Cross-compilation is available via Docker buildx, using the `build_docker.sh` script. This will place the compiled output in the appropriate subdirectories by OS (Ubuntu, Debian) and architecture (arm64, armv8, amd64). QEMU may be required for cross-compilation; if not already installed, add the following packages to your host system:

- qemu-user-static
- qemu-system


### Local registry cache

Local registry caching is available as part of the `build_docker.sh` script. Assuming the local registry is running as a basic, unsecured instance, you may need to check your `docker` and `buildkit` settings to ensure unsecured HTTP access is allowed. The relevant launching script and settings are provided below:

#### `start_docker_registry.sh`

This command will start a local registry in a container; the default port 5000 will be exposed and registry data that the user may want to preserve across container creation and destruction cycles will be stored on the host at `/data/docker_registry` on the host (change this path to a suitable one on your system as needed). See [https://hub.docker.com/_/registry](https://hub.docker.com/_/registry) for more.

```bash
#!/usr/bin/env bash

docker run -d \
	-p 5000:5000 \
	--restart always \
	--name registry \
	-v /data/docker_registry:/var/lib/registry \
	registry:2

```

#### `/etc/docker/daemon.json`

This allows the docker engine to connect to the local registry via HTTP; this enables commands such as `docker pull` and `docker push` to interact with the local registry.

```json
{
    "insecure-registries": [
        "127.0.0.1:5000",
        "localhost:5000",
        "[::1]:5000"
    ]
}
```

#### `/etc/buildkit/buildkitd.toml`

This allows the docker buildkit to connect to the local registry via HTTP; this enables push and pull commands executed by `docker buildx` to interact with the local registry.

```toml
[registry."127.0.0.1:5000"]
  http = true
[registry."localhost:5000"]
  http = true
  ipv6 = false
```


## Installing

By default, the `build.sh` script will set things up to install locally; it will create the installation target file structure under the `_install/` directory within the repo's tree. The `build_docker.sh` will likewise create the target file structure within the `dist/` tree, separating the output by the different operating system and architecture configurations supported. The basic tree structure is as follows:

```
_install
├── bin
│   └── <compiled binaries>
├── include
│   └── acbotics_interface
│       └── <header files>
└── lib
    ├── libacbotics.a
    └── pythonX.YY/site-packages
        └── <python module>


```

To use the Python module, make sure the output direcory is in your environment's `PYTHONPATH`:

```bash
# Replace pythonX.YY with the appropriate version numbers
export PYTHONPATH+=:<PATH_TO_REPO>/_install/lib/pythonX.YY/site-packages
```
