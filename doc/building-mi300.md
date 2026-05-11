# Building UME for MI300 AMD GPU from Source

This guide covers how to build UME from source with all of its dependencies for the MI300. It assumes that you are using a RHEL-based Linux system and `flux` for the workload manager. Before starting, make sure that you have `git`, `cmake`, and `make`  installed on your system.

## Preparing the Dependencies

First, lets set a couple of environment variables to make it easier to set a custom installation path for UME. These variables can be set to any path available for writing, but we're going to use the home directory for now. Then, create the directories to store our build and install files for the next few steps.

```bash
export source_path=$HOME/src
export install_path=$HOME/install
export build_path=$HOME/build
mkdir -p $source_path $install_path $build_path
```

UME uses Kokkos and optionally Catch2 as dependencies. We will need a C++ compiler such as `hip++` provided by [ROCM](https://github.com/ROCm/rocm). Catch2 is only needed if we intend to run the UME tests when building. Otherwise, we can disable it via the build flags. 

Clone all of the repositories from GitHub somewhere into the source directory. 

```bash
pushd $source_path
git clone --branch 4.7.00 https://github.com/kokkos/Kokkos.git $source_path/kokkos
git clone https://github.com/catchorg/Catch2.git $source_path/catch2
git clone https://github.com/lanl/UME.git $source_path/ume
popd
```

We also need to specify our compilers for building. Set the following variables to the compilers provided by ROCM. You will also need to set the C++ MPI compiler if you want to compile `ume_mpi`.

```bash
CC=amd-clang
CXX=amd-clang++
```

Configure and build Kokkos with the following command. 

```bash
mkdir -p $build_path/kokkos
cmake $source_path/kokkos \
		-B $build_path/kokkos \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="$install_path" \
		-DCMAKE_CXX_COMPILER=$CXX \
		-DCMAKE_CXX_STANDARD=20 \
		-DCMAKE_CXX_EXTENSIONS=OFF \
		-DKokkos_ENABLE_OPENMP=OFF \
		-DKokkos_ENABLE_EXAMPLES=OFF \
		-DKokkos_ENABLE_TESTS=OFF \
		-DKokkos_ENABLE_DEPRECATED_CODE_4=OFF \
		-DKokkos_ENABLE_SERIAL=ON \
		-DKokkos_ENABLE_HIP=ON \
		-DKokkos_ENABLE_DEBUG=OFF \
		-DKokkos_ARCH_AMD_GFX942_APU=ON
```

Configure and build Catch2 with the following command.

```bash
mkdir -p $build_path/catch2
USE_CATCH2=YES
	BUILD_TESTING=ON
	if [ "$UME_SETUP_SKIP_TESTS" = true ]; then
		USE_CATCH2=NO
		BUILD_TESTING=OFF
	fi
	cmake $UME_SETUP_SRC_PATH/catch2 \
		-B $UME_SETUP_BUILD_PATH/catch2 \
		-DCMAKE_INSTALL_PREFIX="$install_path" \
		-DBUILD_TESTING=OFF
	make -j $UME_SETUP_CONCURRENT_JOBS
    make install
```

Now, we should be able to build UME using Kokkos and Catch2 that was compiled above. If the build fails due to errors coming from the tests, set `-DUSE_CATCH2=NO` and `-DCMAKE_BUILD_TESTING=NO`. If you get a memory leak error when running UME, try setting `-DUME_SANITIZE=NO` and running again.

```bash
mkdir -p $build_path/ume
cmake $source_path/ume \
		-B $build_path/ume \
		-DUSE_CATCH2=YES \
		-DUSE_MPI=YES \
		-DUSE_OPENACC=NO \
		-DCMAKE_C_COMPILER=$CC \
		-DCMAKE_CXX_COMPILER=$CXX \
		-DCMAKE_CXX_STANDARD=20 \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_BUILD_TESTING=YES \
		-DCMAKE_PREFIX_PATH=$install_path \
		-DUME_HUGEPAGES=YES
```

> If `ctest` fails when compiling, try disabling the UME tests by setting `UME_TESTS=OFF`. Catch2 is no longer needed if this option is set.

Copy all of the binary files over to the install directory. This isn't necessary, but it makes it easier to invoke the executables if the install path is added to you PATH.

```bash
# copy all of the binaries to the appropriate install path
	cp $PWD/src/ume_serial $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/txt2bin $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/scale_mesh $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/ume_mpi $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/kokkos_hello $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/openacc_hello $UME_SETUP_INSTALL_PATH/bin
```

Optionally, we can clean up all of the build files if we no longer need them.

```bash
rm -rf $build_path/*
```

Here is a script that will run through this entire process in one go. You can save the script locally and run it with `./ume-setup.sh run --all`.

<details>
<summary>UME Setup Script</summary>
```bash
#!/bin/sh
##
## This script builds UME and its dependencies with a single invocation.
## The script requires compatible versions of GCC, MPI, and ROCM to run.
##

UME_SETUP_RUN=${1:-"run"}
UME_SETUP_SRC_PATH="$HOME/src"
UME_SETUP_INSTALL_PATH="$HOME/install"
UME_SETUP_BUILD_PATH=/var/tmp/$USER/ume/build
UME_SETUP_SKIP_TESTS=true
UME_SETUP_DELETE_CACHE=true
UME_SETUP_CONCURRENT_JOBS=16
MPICC=$(command -v mpicc)
MPICXX=$(command -v mpicxx)
GCC_ROOT=$(command -v gcc)
GCC_VERSION=13.3.1-magic               # set version for compatibility
CRAY_MPICH=8.1.33                      # set version for compatibility
ROCM_VERSION=6.4.2                     # set version for compatibility
ROCM_PATH=/opt/rocm-${ROCM_VERSION}/
C_COMPILER=${ROCM_PATH/bin/amdclang
CXX_COMPILER=${ROCM_PATH}/bin/amdclang++

export CRAYPE_LINK_TYPE=dynamic
export HSA_XNACK=1
export KOKKOS_PRINT_CONFIGURATION=1

function help() {
	echo "Script to build UME with Kokkos and Catch2. Requires a compatible version of GCC, "
	echo "MPI, and ROCM to build. This script has been tested to work with the following"
	echo "module versions:"
	echo ""
	echo "  - gcc/13.3.1, cray_mpich/8.1.33, rocm/6.4.2"
	echo ""
	echo "usage: $(basename $0) [-s path] [-i path] [-b path] [-j job_count] [-k] <command>"
	echo ""
	echo "commands:"
	echo "  run      Run the entire process."
	echo "  prepare  Prepare dependencies but do not build or install."
	echo "  build    Build UME and its dependencies."
	echo "  clean    Clean temporary build files."
	echo "  vars     Show script variables and exit."
	echo "  help     Show this message then exit."
	echo ""
	echo "examples:"
	echo "  $(basename $0) run "
	echo "  $(basename $0) build -k -d"
	echo "  $(basename $0) -s ./src -i ./install -b ./build -j 8 -k"
	echo ""
	echo "flags: "
	echo "  -r       Run the specified portion of the script. (default: all, values: all,prepare,build,vars)"
	echo "  -k       Skip UME tests when building. (default: true)"
	echo "  -d       Delete extra files after building. (default: false)"
	echo "  -s       Path to download and store source code. (default: \$HOME/src)"
	echo "  -i       Path to install binaries, executables, libraries, etc. (default: \$HOME/install)"
	echo "  -b       Path to build from source. (default: /var/tmp/\$USER/ume/build)"
	echo "  -j       Set the number of concurrent jobs for building. (e.g. 'make -j', default: 16)"
	echo "  -h       Show this message then exit."
}

function show_vars() {
	echo "UME_SETUP_SRC_PATH:        ${UME_SETUP_SRC_PATH}"
	echo "UME_SETUP_INSTALL_PATH:    ${UME_SETUP_INSTALL_PATH}"
	echo "UME_SETUP_BUILD_PATH:      ${UME_SETUP_BUILD_PATH}"
	echo "UME_SETUP_SKIP_TESTS:      ${UME_SETUP_SKIP_TESTS}"
	echo "UME_SETUP_DELETE_CACHE:    ${UME_SETUP_DELETE_CACHE}"
	echo "UME_SETUP_RUN:             ${UME_SETUP_RUN}"
	echo "UME_SETUP_CONCURRENT_JOBS: ${UME_SETUP_CONCURRENT_JOBS}"
	echo "CC_COMPILER:               ${CC_COMPILER}"
	echo "CXX_COMPILER:              ${CXX_COMPILER}"
	echo "MPICC:                     ${MPICC}"
	echo "MPICXX:                    ${MPICXX}"
	echo "GCC_ROOT:                  ${GCC_ROOT}"
	echo "GCC_VERSION:               ${GCC_VERSION}"
	echo "CRAY_MPICH:                ${CRAY_MPICH}"
	echo "ROCM_VERSION:              ${ROCM_VERSION}"
	echo "ROCM_PATH:                 ${ROCM_PATH}"
	echo "CRAYPE_LINK_TYPE:          ${CRAYPE_LINK_TYPE}"
	echo "HSA_XNACK:                 ${HSA_XNACK}"
}

function clean_tmp() {
	pushd $UME_SETUP_BUILD_PATH
	rm -rf kokkos catch2 ume
	popd
}

function prepare() {
	# create the dependency directory
	mkdir -p "$UME_SETUP_SRC_PATH" "$UME_SETUP_INSTALL_PATH" "$UME_SETUP_BUILD_PATH"
	pushd "$UME_SETUP_SRC_PATH"

	# clone catch2
	if [ "$UME_SETUP_SKIP_TESTS" = true ]; then
		continue
	else
		if [ -d "$UME_SETUP_SRC_PATH/catch2" ]; then
			echo "Catch2 directory already exist."
		else
			git clone https://github.com/catchorg/Catch2.git catch2
		fi
	fi

	# clone kokkos (see ref: kokkos.org/kokkos-core-wiki/get-started/building-from-source
	if [ -d "$UME_SETUP_SRC_PATH/kokkos" ]; then
		echo "Kokkos directory already exist."
	else
		git clone --depth=2 --branch 5.1.0 https://github.com/kokkos/kokkos.git kokkos
	fi

	# clone UME
	if [ -d "$UME_SETUP_SRC_PATH/ume" ]; then
		echo "UME directory already exist."
	else
		git clone https://github.com/lanl/UME ume
	fi
	popd
}

function validate() {
	# check if required vars are set
	if [ -z "$UME_SETUP_SRC_PATH" ]; then
		echo "error: source path (UME_SETUP_SRC_PATH) is not set"
		help
		exit 1
	fi

	if [ -z "$UME_SETUP_INSTALL_PATH" ]; then
		echo "error: install path (UME_SETUP_INSTALL_PATH) is not set"
		help
		exit 1
	fi

	if [ -z "$UME_SETUP_BUILD_PATH" ]; then
		echo "error: build path (UME_SETUP_BUILD_PATH) is not set"
		help
		exit 1
	fi

	if [ -z "$UME_SETUP_SKIP_TESTS" ]; then
		echo "error: required var (UME_SETUP_SKIP_TESTS) is not set"
		help
	fi
}

function build() {
	mkdir -p $UME_SETUP_BUILD_PATH
	pushd "$UME_SETUP_BUILD_PATH"

	# build kokkos
	mkdir -p kokkos
	pushd kokkos
	cmake $UME_SETUP_SRC_PATH/kokkos \
		-B $UME_SETUP_BUILD_PATH/kokkos \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="$UME_SETUP_INSTALL_PATH" \
		-DCMAKE_CXX_COMPILER=$CXX_COMPILER \
		-DCMAKE_CXX_STANDARD=20 \
		-DCMAKE_CXX_EXTENSIONS=OFF \
		-DKokkos_ENABLE_OPENMP=OFF \
		-DKokkos_ENABLE_EXAMPLES=OFF \
		-DKokkos_ENABLE_TESTS=OFF \
		-DKokkos_ENABLE_DEPRECATED_CODE_4=OFF \
		-DKokkos_ENABLE_SERIAL=ON \
		-DKokkos_ENABLE_HIP=ON \
		-DKokkos_ENABLE_DEBUG=OFF \
		-DKokkos_ARCH_AMD_GFX942_APU=ON

	make -j $UME_SETUP_CONCURRENT_JOBS
	make install
	popd

	# build catch2
	mkdir -p catch2
	pushd catch2
	cmake $UME_SETUP_SRC_PATH/catch2 \
		-B $UME_SETUP_BUILD_PATH/catch2 \
		-DCMAKE_INSTALL_PREFIX="$UME_SETUP_INSTALL_PATH" \
		-DBUILD_TESTING=OFF
	make -j $UME_SETUP_CONCURRENT_JOBS
	make install
	popd

	# build ume
	mkdir -p ume
	pushd ume

	# disable ctest if skipping tests
	USE_CATCH2=YES
	BUILD_TESTING=ON
	if [ "$UME_SETUP_SKIP_TESTS" = true ]; then
		USE_CATCH2=NO
		BUILD_TESTING=OFF
	fi
	cmake $UME_SETUP_SRC_PATH/ume \
		-B $UME_SETUP_BUILD_PATH/ume \
		-DUSE_CATCH2=$USE_CATCH2 \
		-DUSE_MPI=YES \
		-DUSE_OPENACC=NO \
		-DCMAKE_C_COMPILER=$C_COMPILER \
		-DCMAKE_CXX_COMPILER=$CXX_COMPILER \
		-DCMAKE_CXX_STANDARD=20 \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_BUILD_TESTING=$BUILD_TESTING \
		-DCMAKE_PREFIX_PATH=$UME_SETUP_INSTALL_PATH \
		-DUME_HUGEPAGES=YES

	make -j $UME_SETUP_CONCURRENT_JOBS

	# show what we have in the UME build directory
	tree -L 2 $PWD

	# copy all of the binaries to the appropriate install path
	cp $PWD/src/ume_serial $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/txt2bin $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/scale_mesh $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/ume_mpi $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/kokkos_hello $UME_SETUP_INSTALL_PATH/bin
	cp $PWD/src/openacc_hello $UME_SETUP_INSTALL_PATH/bin
	popd

	if [ "$UME_SETUP_DELETE_CACHE" = true ]; then
		rm -rf kokkos
		rm -rf catch2
		rm -rf ume
	fi

	popd
}

function run() {
	# make sure variables are set before running
	validate
	if [ "$UME_SETUP_RUN" = "run" ]; then
		show_vars
		prepare
		build
	elif [ "$UME_SETUP_RUN" = "vars" ]; then
		show_vars
	elif [ "$UME_SETUP_RUN" = "prepare" ]; then
		prepare
	elif [ "$UME_SETUP_RUN" = "build" ]; then
		build
	elif [ "$UME_SETUP_RUN" = "clean" ]; then
		clean
	elif [ "$UME_SETUP_RUN" = "help" ]; then
		help
	else
		echo "error: invalid value for '-r' flag (possible values: all,vars,prepare,build)"
	fi
}

# process optional short arguments
while getopts "hkd:sibjr:" opt; do
	case $opt in
		h) help; exit;;
		s) UME_SETUP_SRC_PATH="$OPTARG";;
		i) UME_SETUP_INSTALL_PATH="$OPTARG";;
		b) UME_SETUP_BUILD_PATH="$OPTARG";;
		j) UME_SETUP_CONCURRENT_JOBS="$OPTARG";;
		k) UME_SETUP_SKIP_TESTS=false;;
		d) UME_SETUP_DELETE_CACHE=true;;
		r) UME_SETUP_RUN="$OPTARG";;
		:)  echo "missing argument for option -$OPTARG"; exit 1;;
		\?) echo "error: invalid option '-$opt' (try '-h' for options)"; exit;;
	esac
done
run
```
</details>

## Running UME Example

If you are on an HPC system with Lmod, you will need load the modules that provides your compiler, MPI, and ROCM support. We can do this by creating a batch script for the workload manager like Flux. Here is a simple example script. 

```bash
# flux: -N 1                       # number of nodes
# flux: -t 1h                      # walltime (HH:MM:SS)
# flux: -g 4                       # number of gpus per slot
# flux: --job-name=ume-{{id}}      # job name
# flux: --exclusive
# flux: --setattr=hugepages=512GB
# flux: --setattr=gpumode=TPX
# flux: --output={{id}}.out
# flux: --error={{id}}.err
# flux: --env=HUGETLB_MORECORE=yes
# flux: --env=HUGETLB_DEBUG=yes
# flux: --env=HUGETLB_VERBOSE=4
# flux: --env=HUGETLB_ELFMAP=no

# Set the UME setup variables
export UME_SETUP="${HOME}/bin/setup/ume.sh"
export UME_SETUP_RUN="run"

# Set the UME variables
export UME_BIN="${HOME}/install/bin/ume_mpi"
export UME_INPUT="${HOME}/AggAdv_short_208_00001"
export UME_OUTPUT="${HOME}/jobs/ume/outputs/kokkos_omp_1s.txt}"

# Update the LD_LIBRARY_PATH variable to find the locally built libs
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${install_path}/lib:${install_path}/lib64

# (Re)load all of the required modules
module purge --force
module load gcc/13.3.1 cray-mpich/8.1.33 rocm/6.4.2

# Run multiple instances of UME using the SLURM compatibility CLI
for i in {1..5}; do echo `srun -n 1 ${UME_BIN} ${UME_INPUT} -i 20`; done &> ${UME_OUTPUT}
```

Try running this script with `flux batch ume-launch.sh` and view the results specified by the `UME_OUTPUT` variable.
