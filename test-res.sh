
SOURCE_FILE="p3210024-t8210130.c"
EXECUTABLE="p3210024-t8210130"

gcc -pthread -o $EXECUTABLE $SOURCE_FILE


if [ $? -ne 0 ]; then
    echo "Error: Compilation failed."
    exit 1
fi


./$EXECUTABLE 100 10
