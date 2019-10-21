cd ../src
find . \( -name "*.cpp" -o -name "*.c" -o -name "*.hpp" -o -name "*.h" -o -name "*.inl" \) -exec clang-format -style=file -i {} \;
cd ../lib
find . \( -name "*.cpp" -o -name "*.hpp" -o -name "*.inl" \) -exec clang-format -style=file -i {} \;
cd ../test
find . \( -name "*.cpp" -o -name "*.c" -o -name "*.hpp" -o -name "*.h" -o -name "*.inl" \) -exec clang-format -style=file -i {} \;
