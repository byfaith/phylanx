# Copyright (c) 2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx

expected = np.array([[1, 2], [3, 4]])


@Phylanx(lazy=True)
def foo(m):
    a = np.array([[m, 2], [3, 4]])
    return a


lazy_foo = foo(1)
evaluated_foo = lazy_foo.eval()
assert (evaluated_foo == expected).all()


@Phylanx
def foo(m):
    a = np.array([[m, 2], [3, 4]])
    return a


evaluated_foo = foo(1)
assert (evaluated_foo == expected).all()
