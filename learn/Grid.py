# %%

import numpy as np
from numpy import array, linspace
from sklearn.neighbors.kde import KernelDensity
from matplotlib.pyplot import plot
from scipy.signal import argrelextrema

a = array([10, 11, 9, 23, 21, 11, 45, 20, 11, 12]).reshape(-1, 1)
kde = KernelDensity(kernel='gaussian', bandwidth=4).fit(a)
s = linspace(0, 50)
e = kde.score_samples(s.reshape(-1, 1))
plot(s, e)
mi, ma = argrelextrema(e, np.less), argrelextrema(e, np.greater)
print(mi, ma)

# %%
print("Minima:", s[mi])
print("Maxima:", s[ma])
# %%
print(a[a < mi[0]], a[(a >= mi[0]) * (a <= mi[1])], a[a >= mi[1]])
