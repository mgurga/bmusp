mkdir -p build
cd build

rm bmusp

for command in "$@"
do
    case $command in
        "configure")
            echo "configuring..."
            cmake ..
            ;;
        "build")
            echo "building..."
            cmake --build .
            ;;
        "clean-build")
            echo "clean building..."
            cmake --build . --clean-first
            ;;
        "release-build")
            echo "clean building release..."
            cmake --build . --clean-first --config Release
            ;;
        "run")
            echo "running..."
            ./bmusp
            ;;
        *)
            echo "unknown command: $command"
            ;;
    esac
done

cd ..

if [[ "$@" == "" ]]
then
    ./build.sh configure build run
fi