from os import listdir
from os.path import isfile, join
from itertools import islice
import pandas as pd
import matplotlib.pyplot as plt

global y11
global y22

directory = "./EEC277_Project"
figs = "./Figs"
GPU = "Intel Iris Graphics 6100"
files = [join(directory, f) for f in listdir(directory) if f.endswith(".txt")]
for name in files:
	f = open(name, 'r')
	testname = (name.split('/')[-1]).split('.')[0]
	tmp = [line.strip().split('\t') for line in f]
	df = pd.DataFrame(tmp[1:], columns = tmp[0])

	df[["Spheres", "Iterations", "Ray Count"]] = df[["Spheres", "Iterations", "Ray Count"]].astype(int)
	df[["Distance", "Frame Rate"]] = df[["Distance", "Frame Rate"]].astype(float)
	if "Plane" in tmp[0]:
		df[["Plane"]] = df[["Plane"]].astype(int)
	if "Light Moving" in tmp[0]:
		df[["Light Moving"]] = df[["Light Moving"]].astype(int)
	if "Refraction" in tmp[0]:
		df[["Refraction"]] = df[["Refraction"]].astype(int)

	f.close()

	if testname == "DistanceTest":
		x = df["Distance"]
		y1 = df["Frame Rate"]
		y2 = df["Ray Count"]

		fig = plt.figure(figsize = (10.5, 8.5))
		ax1 = fig.add_subplot(111)
		ax2 = ax1.twinx()
		fr = y1.plot(kind="bar", ax=ax1, width=0.15, position=-0.5, color="#6688AA", label="Frame Rate")
		rc = y2.plot(kind="bar", ax=ax2, width=0.15, position=0.5, color="#AA8844", label="Ray Count")

		ax1.set_ylabel("Frame Rate")
		ax2.set_ylabel("Ray Count")

		ax1.yaxis.grid()
		ax1.legend(loc=1, prop={'size':12})
		ax2.legend(loc=2, prop={'size':12})
		plt.title("Distance Test")
		plt.savefig(figs + "/Distance Test " + GPU + " .png")
		plt.show()

	elif testname == "IterationTest":
		y11 = df["Frame Rate"]
		y22 = df["Ray Count"]
	elif testname == "IterationTest_R":
		x = df["Iterations"]
		y1 = df["Frame Rate"]
		y2 = df["Ray Count"]

		fig = plt.figure(figsize = (10.5, 8.5))
		ax1 = fig.add_subplot(111)
		ax2 = ax1.twinx()

		y1 = df["Frame Rate"]
		y2 = df["Ray Count"]
		frr = y11.plot(kind="bar", ax=ax1, width=0.15, position=1.5, color="#6688EE", label="Frame Rate (Refraction)")
		fr = y1.plot(kind="bar", ax=ax1, width=0.15, position=2.5, color="#6688AA", label="Frame Rate (No Refraction)")
		rcc = y22.plot(kind="bar", ax=ax2, width=0.15, position=-1.5, color="#EECC44", label="Ray Count (Refraction)")
		rc = y2.plot(kind="bar", ax=ax2, width=0.15, position=-0.5, color="#AA8844", label="Ray Count (No Refraction)")

		plt.xlim(-1, 8)

		ax1.set_ylabel("Frame Rate")
		ax2.set_ylabel("Ray Count")

		ax1.yaxis.grid()
		ax1.legend(loc=1, prop={'size':12})
		ax2.legend(loc=9, prop={'size':12})
		plt.title("Iteration Test (Refraction VS No Refraction)")
		plt.savefig(figs + "/Iteration Test (Refraction VS No Refraction) " + GPU + " .png")
		plt.show()
	elif testname == "NumberTest":
		y11 = df["Frame Rate"]
		y22 = df["Ray Count"]
	elif testname == "NumberTest_R":
		x = df["Spheres"]
		y1 = df["Frame Rate"]
		y2 = df["Ray Count"]

		fig = plt.figure(figsize = (10.5, 8.5))
		ax1 = fig.add_subplot(111)
		ax2 = ax1.twinx()

		y1 = df["Frame Rate"]
		y2 = df["Ray Count"]
		frr = y11.plot(kind="bar", ax=ax1, width=0.15, position=1.5, color="#6688EE", label="Frame Rate (Refraction)")
		fr = y1.plot(kind="bar", ax=ax1, width=0.15, position=2.5, color="#6688AA", label="Frame Rate (No Refraction)")
		rcc = y22.plot(kind="bar", ax=ax2, width=0.15, position=-1.5, color="#EECC44", label="Ray Count (Refraction)")
		rc = y2.plot(kind="bar", ax=ax2, width=0.15, position=-0.5, color="#AA8844", label="Ray Count (No Refraction)")

		plt.xlim(-1, 6)

		ax1.set_ylabel("Frame Rate")
		ax2.set_ylabel("Ray Count")

		ax1.yaxis.grid()
		ax1.legend(loc=1, prop={'size':12})
		ax2.legend(loc=9, prop={'size':12})
		plt.title("Number Test (Refraction VS No Refraction)")
		plt.savefig(figs + "/Number Test (Refraction VS No Refraction) " + GPU + " .png")
		plt.show()
	elif testname == "Standard":
		y11 = df["Frame Rate"]
		y22 = df["Ray Count"]
	elif testname == "Standard_1":
		labels = ["All Enabled", "No Plane", "Light Fixed", "No Refraction", "No Ray Counting (1-Pass)", "All Disabled"]
		fig = plt.figure(figsize = (10.5, 8.5))
		ax1 = fig.add_subplot(111)
		ax2 = ax1.twinx()

		y1 = df["Frame Rate"]
		y2 = df["Ray Count"]
		frr = y11.plot(kind="bar", ax=ax1, width=0.15, position=1.5, color="#6688EE", label="Frame Rate (125 Spheres)")
		fr = y1.plot(kind="bar", ax=ax1, width=0.15, position=2.5, color="#6688AA", label="Frame Rate (1 Sphere)")
		rcc = y22.plot(kind="bar", ax=ax2, width=0.15, position=-1.5, color="#EECC44", label="Ray Count (125 Spheres)")
		rc = y2.plot(kind="bar", ax=ax2, width=0.15, position=-0.5, color="#AA8844", label="Ray Count (1 Sphere)")

		plt.xlim(-1, 6)

		ax1.set_ylabel("Frame Rate")
		ax2.set_ylabel("Ray Count")

		ax1.set_xticklabels(labels) 
		plt.setp(ax1.xaxis.get_majorticklabels(), rotation=18)

		ax1.yaxis.grid()
		ax1.legend(loc=1, prop={'size':12})
		ax2.legend(loc=2, prop={'size':12})
		plt.title("Standard Test (1 Sphere VS 125 Spheres)")
		plt.savefig(figs + "/Standard Test (1 Sphere VS 125 Spheres) " + GPU + " .png")
		plt.show()