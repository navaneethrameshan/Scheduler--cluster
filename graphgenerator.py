# filter log file and create gnuplot files
import sys

queuedata = open('queueavg.data', 'w')
queueavg = open('queueavg.plot', 'w')
queueavg.write('set xlabel "Time (ticks)"\n')
queueavg.write('set ylabel "Queued Jobs"\n')
queueavg.write('set ytics\n')
queueavg.write('set grid\n')
queueavg.write('set xtics\n')
queueavg.write('set terminal png\n')
queueavg.write('set output "graphs/queueavg.png"\n')
queueavg.write('plot "queueavg.data" using 1:2 with lp\n')
queueavg.close()

workersdata = open('workers.data', 'w')
workers = open('workers.plot', 'w')
workers.write('set xlabel "Time (ticks)"\n')
workers.write('set ylabel "Workers"\n')
workers.write('set terminal png\n')
workers.write('set ytics\n')
workers.write('set grid\n')
workers.write('set xtics\n')
workers.write('set output "graphs/workers.png"\n')
workers.write('plot "workers.data" using 1:2 with lp\n')
workers.close()

def graph():
    log = open(sys.argv[1], 'rt')
    for line in log.readlines():
        if (line[:1] == '-'):
            store(line[1:].split())
            
    log.close()

def store(values):
    workersdata.write(values[0] + "\t" + values[1] + "\n")
    queuedata.write(values[0] + "\t" + values[2] + "\n")


if '__main__' == __name__:
    graph()
    queuedata.close()
    workersdata.close()
