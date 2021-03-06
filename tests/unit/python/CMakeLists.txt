# Copyright (c) 2017-2018 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests
    version
   )

foreach(test ${tests})
  set(script ${test}.py)

  add_phylanx_python_unit_test("python" ${test}
    SCRIPT ${script}
    FOLDER "Tests/Python/Unit"
    DEPENDS phylanx_py python_setup
    WORKING_DIRECTORY ${PHYLANX_PYTHON_EXTENSION_LOCATION}
    ENVIRONMENT "PYTHONPATH=${PHYLANX_PYTHON_EXTENSION_LOCATION}")

  add_phylanx_pseudo_target(tests.unit.python.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.unit.python tests.unit.python.${test}_py)
  add_phylanx_pseudo_dependencies(
    tests.unit.python.${test}_py ${test}_test_py)

endforeach()

set(subdirs
    ast
    execution_tree
    plugins
    util
   )

foreach(subdir ${subdirs})
  add_phylanx_pseudo_target(tests.unit.python_${subdir})
  add_subdirectory(${subdir})
  add_phylanx_pseudo_dependencies(tests.unit.python tests.unit.python_${subdir})
endforeach()

