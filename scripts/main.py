import sys

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 main.py <filename>")
        exit(-1)

    filename = sys.argv[1]

    with open(filename, "r") as f:
        with open("../FileTests/" + filename + ".rasm", "w") as nf:
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

                a = a.replace("\t", "")
                a = a.replace("\n", "")
                a = a.lower()

                if a == "mv":
                    line = change_mv(line)
                elif a == "call":
                    line = change_call(line)
                elif a == "ret":
                    line = change_ret(line)
                elif a == "mul":
                    line = change_mul(line)
                elif a == "li":
                    line = change_li(line)
                elif a == "jr":
                    line = change_jr(line)
                elif a == "j":
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

def change_jr(line):
    return "jalr \tzero, 0(ra)\n"

def change_li(line):
    line = line.replace(",", " ")
    line = line.replace("\n", " ")
    split = line.split(" ")

    tokens = []
    for s in split:
        if s != "":
            tokens.append(s)
    
    imm = int(tokens[2])
    # This means 11th bit is set, meaning ADDI would sign extend it
    # Get signed representation of lower 12 bits and subtract it from immediate
    # Then shift it to the right by 12 to get the offset that lui needs to have to get correct immediate
    if imm >= 2048:
        lower_12bits = imm & 0xFFF
        lower_12bits = ~lower_12bits
        upper_20bits = imm - lower_12bits
        upper_20bits = (upper_20bits >> 12) & 0xFFF

        line = "lui \t" + tokens[1] + ", " + upper_20bits
        line += "addi \t" + tokens[1] + ", " + tokens[1] + ", " + lower_12bits + "\n"
    else:
        line = "addi \t" + tokens[1] + ", " + tokens[1] + ", " + tokens[2] + "\n"
    return line

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
