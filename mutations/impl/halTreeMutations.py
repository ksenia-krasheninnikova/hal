#!/usr/bin/env python

#Copyright (C) 2012 by Glenn Hickey
#
#Released under the MIT license, see LICENSE.txt
#!/usr/bin/env python

"""Make histogram of inter-event distances in BED file
"""
import argparse
import os
import sys
import copy
import subprocess


def runShellCommand(command):
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE,
                               stderr=sys.stderr, bufsize=-1)
    output, nothing = process.communicate()
    sts = process.wait()
    if sts != 0:
        raise RuntimeError("Command: %s exited with non-zero status %i" %
                           (command, sts))
    return output

def getHalRootName(halPath):
    return runShellCommand("halStats %s --root" % halPath)

def getHalParentName(halPath, genomeName):
    return runShellCommand("halStats %s --parent %s" % (halPath, genomeName))

def getHalChildrenNames(halPath, genomeName):
    return runShellCommand("halStats %s --children %s" %
                           (halPath, genomeName)).split()
                        
def getHalBranchMutations(halPath, genomeName, args):
    command = "halBranchMutations %s %s --maxGap %s" % (halPath, genomeName,
                                                        args.maxGap)
    
    refBedFile = os.path.join(args.outDir,  "%s.bed" % genomeName)
    dest = refBedFile
    if args.doSort is True:
        dest = "stdout"
        
    command += " --refFile %s" % dest
    command += " --delBreakFile %s" % dest
    if args.doSnps:
        command += " --snpFile %s" % dest
    if args.doParentDeletions:
        command += " --parentFile %s" % os.path.join(args.outDir, 
                                                     "%s_pd.bed" % genomeName)

    if args.doSort is True:
        command += " | sortBed > %s" % refBedFile
    print command
    runShellCommand(command)

def getHalTreeMutations(halPath, args, rootName=None):
    root = rootName
    if root is None:
        root = getHalRootName(halPath)
    for child in getHalChildrenNames(halPath, root):
        getHalBranchMutations(halPath, child, args)
        getHalTreeMutations(halPath, args, child)

def main(argv=None):
    if argv is None:
        argv = sys.argv

    parser = argparse.ArgumentParser()
    parser.add_argument("hal", help="input hal")
    parser.add_argument("outDir", help="output dir")    
    parser.add_argument("--root", default=None, type=str, help="root")
    parser.add_argument("--doSnps",action="store_true", default=False)
    parser.add_argument("--doParentDeletions",action="store_true",
                        default=False)
    parser.add_argument("--maxGap", default=10, type=int, help="gap threshold")
    args = parser.parse_args()

    if not os.path.exists(args.outDir):
        os.makedirs(args.outDir)

    args.doSort = True
    try:
        runShellCommand("echo \"x\t0\t1\" | sortBed 2> /dev/null")
    except Exception:
        print ("Warning: output BED files not sorted because sortBed" + 
               " (BedTools) not found")
        args.doSort = False
        
    getHalTreeMutations(args.hal, args, args.root)
    
if __name__ == "__main__":
    sys.exit(main())
