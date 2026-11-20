include(Catch)

function(shekinah_register_test target label)
    catch_discover_tests(${target} PROPERTIES LABELS "${label}")
endfunction()
