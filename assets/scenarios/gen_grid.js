var fs = require('fs');
var stream = fs.createWriteStream("debug.in");

var min_x = 0.05;
var max_x = 0.45;
var min_y = 0.05;
var max_y = 0.85;
var min_z = 0.05;
var max_z = 0.85;
var w = max_x - min_x;
var h = max_y - min_y;
var d = max_z - min_z;

console.log(w);
console.log(h);
console.log(d);

var p = 100000;
var p_x = Math.round(Math.pow((p * Math.pow(w,2)) / (h * d), 1/3));
var p_y = Math.round(p_x * (h/w));
var p_z = Math.round(p_x * (d/w));

console.log(p_x);
console.log(p_y);
console.log(p_z);

var s_x = w / (p_x - 1);
var s_y = h / (p_y - 1);
var s_z = d / (p_z - 1);

console.log(s_x);
console.log(s_y);
console.log(s_z);

var radius = Math.min(s_x,s_y,s_z) / 10;

var mass = ((w * h * d) * 1000) / p;

console.log(mass);

var precision = 10 * 5;

stream.once('open', function(fd) {
    for (var x = min_x; x < max_x; x+= s_x) {
        for (var y = min_y; y < max_y; y += s_y) {
            for (var z = min_z; z < max_z; z += s_z) {
                stream.write(mass+' ' + radius + ' ' + x + ' ' + y + ' ' + z +' 0 0 0\n');
            }
        }
    }
});