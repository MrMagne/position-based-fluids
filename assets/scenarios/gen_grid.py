name = 'dam_miles'

h = 0.02
s = h / 2

min_x = 0.20
max_x = 0.40
min_y = 0.20
max_y = 0.40
min_z = 0.20
max_z = 0.40
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
f.write("time_end              {}\n".format(10.0))
f.write("timestep_length       {}\n".format(0.01))
f.write("x_min                 {}\n".format(0))
f.write("x_max                 {}\n".format(0.5))
f.write("y_min                 {}\n".format(0))
f.write("y_max                 {}\n".format(0.5))
f.write("z_min                 {}\n".format(0))
f.write("z_max                 {}\n".format(0.5))
f.write("x_n                   {}\n".format(int(round((0.5-0.0)/h))))
f.write("y_n                   {}\n".format(int(round((0.5-0.0)/h))))
f.write("z_n                   {}\n".format(int(round((0.5-0.0)/h))))
f.write("density               {}\n".format(p * (width * height * depth)))
f.close()
