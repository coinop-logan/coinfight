import sys, math, time, os
from peeps import peeps

def main():
    # WHO

    if len(sys.argv) == 1:
        who = 'me'
    else:
        who = sys.argv[1]
    
    cmdLine = ""
    try:
        cmdLine += peeps[who] + " "
    except KeyError:
        if who[:2] != "0x":
            print("that doesn't look like an address or a name I know.")
            return
        else:
            cmdLine += who + " "

    
    # HOW MUCH

    amount = float(sys.argv[2])
    
    # convert to wei and add to cmdLine
    weiAmount = math.floor(amount * (10**18))
    cmdLine += str(weiAmount)

    
    # WRITE FILE

    filename = str(time.time()) + ".dat"
    with open(filename, 'w') as f:
        f.write(cmdLine)
    
    os.system("mv " + filename + " ../bin/events_out/withdrawals/")

main()