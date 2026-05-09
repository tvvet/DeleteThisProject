# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

add_compile_options(
    "-Wall"          # Базові попередження
    "-Wextra"         # Додаткові перевірки
    "-Wpedantic"      # Перевірка на відповідність стандарту ISO C
    "-Wshadow"        # Попередження, якщо локальна змінна перекриває глобальну
    "-Wformat=2"      # Поглиблена перевірка функцій printf/scanf
    "-Wundef"         # Попередження, якщо в #if використовується невизначений макрос
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(DeleteThisProject)
