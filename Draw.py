from os import listdir
from os.path import isfile, join
from itertools import islice
import pandas as pd
import matplotlib.pyplot as plt


directory = "./EEC277_Project"
figs = "./Figs"
files = [join(directory, f) for f in listdir(directory) if f.endswith(".txt")]
for name in files:
	f = open(name, 'r')
	tmp = [line.strip().split('\t') for line in f]
	df = pd.DataFrame(tmp[1:], columns = tmp[0])

	df[["Spheres", "Iterations"]] = df[["Spheres", "Iterations"]].astype(int)
	df[["Distance", "Frame Rate", "Ray Count"]] = df[["Spheres", "Iterations", "Ray Count"]].astype(int)
	if "Plane" in tmp[0]:
		df[["Plane"]] = df[["Plane"]].astype(int)
	if "Light Moving" in tmp[0]:
		df[["Light Moving"]] = df[["Light Moving"]].astype(int)
	if "Refraction" in tmp[0]:
		df[["Refraction"]] = df[["Refraction"]].astype(int)

	f.close()

	