import sys, math, time, os

peeps = {
    'imri': '0xf665624F3247eb143c09B0733d2b3D81a6248ab9',
    'me': '0xBB5eb03535FA2bCFe9FE3BBb0F9cC48385818d92',
    'me2': '0xc835c3dCfD49Bb7b3E4E90532Db48e270160f946'
}

def main():
    # WHO

    if len(sys.argv) == 1:
        who = 'me'
    else:
        who = sys.argv[1]
    
    cmdLine = ""
    if who == "honeypot":
        cmdLine += "honeypot "
    else:
        try:
            cmdLine += peeps[who] + " "
        except KeyError:
            if who[:2] != "0x":
                print("that doesn't look like an address or a name I know.")
                return
            else:
                cmdLine += who + " "

    
    # HOW MUCH

    if len(sys.argv) == 2:
        amount = 10
    else:
        amount = float(sys.argv[2])
    
    # convert to wei and add to cmdLine
    weiAmount = math.floor(amount * (10**18))
    cmdLine += str(weiAmount)

    
    # WRITE FILE

    filename = str(time.time()) + ".dat"
    with open(filename, 'w') as f:
        f.write(cmdLine)
    
    os.system("mv " + filename + " ../bin/accounting/pending_deposits/")

main()