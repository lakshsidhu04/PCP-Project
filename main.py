import os
os.environ.setdefault("MPLCONFIGDIR", "/private/tmp/matplotlib")
import subprocess
import matplotlib.pyplot as plt

scal_File = "scalability.cpp"
thru_File = "throughput.cpp"


def build_and_run(source_file, output_file):
    subprocess.run(
        ["g++", "-std=c++17", "-pthread", source_file, "-o", output_file],
        check=True,
    )
    subprocess.run([f"./{output_file}"], check=True)


def run_scal():
    build_and_run(scal_File, "scalability.o")
    timeHem = []
    timeK42 = []
    with open("scalability_hemlock.txt", "r") as f:
        for line in f:
            timeHem.append(float(line.split()[0]))
    with open("scalability_k42.txt", "r") as f:
        for line in f:
            timeK42.append(float(line.split()[0]))

    
    threadCounts = [1, 2, 4, 8, 16, 32, 64]
    plt.plot(threadCounts, timeHem, label="Hemlock")
    plt.plot(threadCounts, timeK42, label="K42")
    plt.xlabel("Number of Threads")
    plt.ylabel("Average Wait Time (us)")
    plt.title("Average Wait Time vs Number of Threads")
    plt.legend()
    plt.xscale("log", base=2)
    plt.xticks(threadCounts)
    plt.grid()
    plt.savefig("scalability_plot.png")
    plt.close()


def run_thru():
    build_and_run(thru_File, "throughput.o")
    thrHem = []
    thrK42 = []
    with open("throughput_hemlock.txt", "r") as f:
        for line in f:
            thrHem.append(float(line.split()[0]))
    with open("throughput_k42.txt", "r") as f:
        for line in f:
            thrK42.append(float(line.split()[0]))
    
    threadCounts = [1, 2, 4, 8, 16, 32, 64]
    plt.plot(threadCounts, thrHem, label="Hemlock")
    plt.plot(threadCounts, thrK42, label="K42")
    plt.xlabel("Number of Threads")
    plt.ylabel("Throughput (ops/sec)")
    plt.title("Throughput vs Number of Threads")
    plt.legend()
    plt.xscale("log", base=2)
    plt.xticks(threadCounts)
    plt.grid()
    plt.savefig("throughput_plot.png")
    plt.close()

if __name__ == "__main__":
    run_scal()
    run_thru()
