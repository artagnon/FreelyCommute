# %%

from numpy import array
from scipy.optimize import leastsq
import matplotlib.pyplot as plt

y = array([12, 8, 11, 7, 5, 2, 3, 5, 6, 4, 5, 7, 8, 13, 19, 22, 25])
x = array(range(len(y)))

params = [0, 0, 0]


def func1(params, x, y):
    a, b, c = params[0], params[1], params[2]
    return y-(a*x**2+b*x+c)


result = leastsq(func1, params, (x, y))

a, b, c = result[0][0], result[0][1], result[0][2]
yfit1 = a*x**2+b*x+c
plt.plot(x, yfit1)
plt.show()
