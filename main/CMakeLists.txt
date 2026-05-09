idf_component_register(SRCS "oneshot_read_main.cpp"
                    INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE     
    "-Wall"           # Базові попередження
    "-Wextra"         # Додаткові перевірки
    "-Wpedantic"      # Перевірка на відповідність стандарту ISO C
    "-Wconversion"    # Перевірка неявних перетворень
    "-Wfloat-conversion" # Окремо для перетворень float -> int
    "-Wsign-conversion"  # Попередить, якщо ви змішуєте signed та unsigned
    "-Wshadow"        # Попередження, якщо локальна змінна перекриває глобальну
    "-Wformat=2"      # Поглиблена перевірка функцій printf/scanf
    "-Wundef"         # Попередження, якщо в #if використовується невизначений макрос)
)