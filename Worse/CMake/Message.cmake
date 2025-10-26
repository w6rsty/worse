include_guard()

function(PrintHeader MSG WIDTH)
    string(LENGTH "${MSG}" MSG_LEN)

    set(FILL_CHAR "=")
    set(LEFT_BRACKET "[")
    set(RIGHT_BRACKET "]")

    math(EXPR BRACKETED_LEN "${MSG_LEN} + 2")
    math(EXPR TOTAL_FILL "${WIDTH} - ${BRACKETED_LEN}")
    if (TOTAL_FILL LESS 0)
        set(TOTAL_FILL 0)
    endif()

    math(EXPR LEFT_FILL "${TOTAL_FILL} / 2")
    math(EXPR RIGHT_FILL "${TOTAL_FILL} - ${LEFT_FILL}")

    string(REPEAT "${FILL_CHAR}" ${LEFT_FILL} LEFT_PART)
    string(REPEAT "${FILL_CHAR}" ${RIGHT_FILL} RIGHT_PART)

    set(FINAL_LINE "${LEFT_PART}${LEFT_BRACKET}${MSG}${RIGHT_BRACKET}${RIGHT_PART}")
    message(STATUS "${FINAL_LINE}")
endfunction()

macro(MessageScope FUNC)
    PrintHeader("Start ${FUNC}" 80)

    cmake_language(CALL ${FUNC} ${ARGN})

    PrintHeader("Done ${FUNC}" 80)
    message(STATUS "\n")
endmacro()