def main():
    with open("rv32gc.txt", "r") as f:
        with open("../FileTests/rv32i.rasm", "w") as nf:
            nf.write("_start:\n")
            nf.write("\t\taddi \tsp, zero, 0x700\n")
            nf.write("\t\tjal \tra, main\n")
            nf.write("\t\tebreak\n\n")

            line = f.readline()

            while(line):
                line = change_s0(line)

                if '#' in line:
                    line = remove_comments(line)
                    line += "\n"

                if "(int)" in line:
                    line = line.replace("(int)", "")

                split = line.split(" ")
                for s in split:
                    if s != "":
                        a = s
                        break

                if "mv" in a.lower():
                    line = change_mv(line)
                elif "call" in a.lower():
                    line = change_call(line)
                elif "ret" in a.lower():
                    line = change_ret(line)
                elif "mul" in a.lower():
                    line = change_mul(line)
                elif "j" in a.lower():
                    line = line.replace("j", "beq \tzero, zero, ")

                nf.write(line)
                line = f.readline()

def change_s0(line):
    return line.replace("s0", "fp")

def remove_comments(line):
    return line.split('#')[0]

def change_mv(line):
    line = line.replace("mv", "add")
    line = line.replace("\n", ", zero")
    line += "\n"
    return line

def change_call(line):
    return line.replace("call", "jal \tra, ")

def change_ret(line):
    return line.replace("ret", "jalr \tzero, 0(ra)")

def change_mul(line):
    line = line.replace(",", " ")
    line = line.replace("\n", " ")
    split = line.split(" ")

    a = []

    for s in split:
        if s != "":
            a.append(s)

    line = "\t\taddi sp, sp, -12\n\t\tsw t0, 8(sp)\n\t\tsw t1, 4(sp)\n\t\tsw t2, 0(sp)\n"
    line += "\t\tadd t0, zero, zero\n\t\tadd t1, " + a[2] + ", zero\n\t\tadd t2, " + a[3] + ", zero\n"
    line += "loop:\n\t\tadd t0, t0, t1\n\t\taddi t2, t2, -1\n\t\tbne t2, zero, loop\n"
    line += "\t\tadd " + a[1] + ", t0, zero\n\t\tlw t0, 8(sp)\n\t\tlw t1, 4(sp)\n\t\tlw t2, 0(sp)\n"
    line += "\t\taddi sp, sp, 12\n"

    return line


if __name__ == '__main__':
    main()
