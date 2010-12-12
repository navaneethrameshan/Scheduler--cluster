# filter log file and create gnuplot files
import sys

data = open('cloud.data', 'w')

response = open('response.plot', 'w')
response.write('set xlabel "Time (s)"\n')
response.write('set ylabel "Average response time (s)"\n')
response.write('set y2label "Queued jobs (#)"\n')
response.write('set y2tics nomirror\n')
response.write('set ytics\n')
response.write('set y2tics\n')
response.write('set grid x y2\n')
response.write('set xtics\n')
response.write('set terminal png\n')
response.write('set output "graphs/response.png"\n')
response.write('plot "cloud.data" using 1:2 with lp axes x1y1 title "response time", "cloud.data" using 1:8 with lp axes x2y2 title "queue size"\n')
response.close()

queueavg = open('queueavg.plot', 'w')
queueavg.write('set xlabel "Time (s)"\n')
queueavg.write('set ylabel "Cost (Euro)"\n')
queueavg.write('set ytics\n')
queueavg.write('set grid\n')
queueavg.write('set xtics\n')
queueavg.write('set terminal png\n')
queueavg.write('set output "graphs/cost.png"\n')
queueavg.write('plot "cloud.data" using 1:3 with lp title "cost"\n')
queueavg.close()

workers = open('workers.plot', 'w')
workers.write('set xlabel "Time (s)"\n')
workers.write('set ylabel "Workers"\n')
workers.write('set terminal png\n')
workers.write('set ytics\n')
workers.write('set grid\n')
workers.write('set xtics\n')
workers.write('set output "graphs/workers.png"\n')
workers.write('plot "cloud.data" using 1:4 title "active",')
workers.write('"cloud.data" using 1:5 title "offline",')
workers.write('"cloud.data" using 1:6 title "idle",')
workers.write('"cloud.data" using 1:7 title "computing" \n')
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
