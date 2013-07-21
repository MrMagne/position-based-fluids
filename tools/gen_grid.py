name = 'dam_coarse'

h = 1.0
s = h / 2

min_x = 10
max_x = 20
min_y = 10
max_y = 20
min_z = 10
max_z = 20
width = max_x - min_x
height = max_y - min_y
depth = max_z - min_z

p_x = int(round(width / s)) + 1
p_y = int(round(height / s)) + 1
p_z = int(round(depth / s)) + 1

p = p_x * p_y * p_z
m = 1.0
r = s

f = open(name+'.in', 'w')
f.write('{}\n'.format(p))

for x in range(p_x):
    for y in range(p_y):
        for z in range(p_z):
            f.write("{} {} {} {} {} {} {} {}\n".format(
                m, s, min_x + x * s, min_y + y * s, min_z + z * s, 0, 0, 0))

f.close()

f = open(name+'.par', 'w')
f.write("part_input_file       {}\n".format(name+'.in'))
f.write("time_end              {}\n".format(1.0))
f.write("timestep_length       {}\n".format(0.01))
f.write("x_min                 {}\n".format(0.0))
f.write("x_max                 {}\n".format(30.0))
f.write("y_min                 {}\n".format(0.0))
f.write("y_max                 {}\n".format(30.0))
f.write("z_min                 {}\n".format(0.0))
f.write("z_max                 {}\n".format(30.0))
f.write("x_n                   {}\n".format(int(round((30.0-0.0)/h))))
f.write("y_n                   {}\n".format(int(round((30.0-0.0)/h))))
f.write("z_n                   {}\n".format(int(round((30.0-0.0)/h))))
f.write("density               {}\n".format(p / (width * height * depth)))
f.close()
