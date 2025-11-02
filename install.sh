if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

if ! command -v gcc &> /dev/null
then
    echo "gcc could not be found"
    exit
fi

if ! command -v cmake &> /dev/null
then
    echo "cmake could not be found"
    exit
fi

if ! command -v make &> /dev/null
then
    echo "make could not be found"
    exit
fi

if ! command -v git &> /dev/null
then
    echo "git could not be found"
    exit
fi

git clone https://github.com/tatewilhelm/chip8emu
cd chip8emu
cmake CMakeLists.txt
make
make install
cd ../ 
rm -rf chip8emu/ 
