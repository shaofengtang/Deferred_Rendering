import random


lightFile = open("lights.data", "w")

for i in range(500):
	x = random.uniform(-50.0, 50.0)
	y = random.uniform(1.0, 40.0)
	z = random.uniform(-30.0, 30.0)
	c = 1.0
	l = 0.6
	q = 0.3
	ar = 0.01
	ag = 0.01
	ab = 0.01
	dr = random.uniform(0.05, 1.0)
	dg = random.uniform(0.05, 1.0)
	db = random.uniform(0.05, 1.0)
	sr = dr
	sg = dg
	sb = db
	
	lightFile.write("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n" % (x, y, z, c, l, q, ar, ag, ab, dr, dg, db, sr, sg, sb))


lightFile.close()