
SOURCE_FILE="mainprogram.c"
EXECUTABLE="mainprogram"

gcc -pthread -o $EXECUTABLE $SOURCE_FILE


if [ $? -ne 0 ]; then
    echo "Error: Compilation failed."
    exit 1
fi


./$EXECUTABLE 100 10
