# filter log file and create gnuplot files
import sys

data = open('cloud.data', 'w')

response = open('response.plot', 'w')
response.write('set xlabel "Time (s)"\n')
response.write('set ylabel "Average response time (s)"\n')
response.write('set ytics\n')
response.write('set grid\n')
response.write('set xtics\n')
response.write('set terminal png\n')
response.write('set output "graphs/response.png"\n')
response.write('plot "cloud.data" using 1:2 with lp\n')
response.close()

queueavg = open('queueavg.plot', 'w')
queueavg.write('set xlabel "Time (s)"\n')
queueavg.write('set ylabel "Queued Jobs"\n')
queueavg.write('set ytics\n')
queueavg.write('set grid\n')
queueavg.write('set xtics\n')
queueavg.write('set terminal png\n')
queueavg.write('set output "graphs/queueavg.png"\n')
queueavg.write('plot "cloud.data" using 1:7 with lp\n')
queueavg.close()

workers = open('workers.plot', 'w')
workers.write('set xlabel "Time (s)"\n')
workers.write('set ylabel "Workers"\n')
workers.write('set terminal png\n')
workers.write('set ytics\n')
workers.write('set grid\n')
workers.write('set xtics\n')
workers.write('set output "graphs/workers.png"\n')
workers.write('plot "cloud.data" using 1:4 title "offline",')
workers.write('"cloud.data" using 1:5 title "idle",')
workers.write('"cloud.data" using 1:6 title "computing" \n')
workers.close()

def graph():
    log = open(sys.argv[1], 'rt')
    for line in log.readlines():
        if (line[:1] == '-'):
            data.write(line[1:])
            
    log.close()


if '__main__' == __name__:
    graph()
    data.close()
