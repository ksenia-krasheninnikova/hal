/*
 * Copyright (C) 2012 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include "halColumnLiftover.h"

using namespace std;
using namespace hal;

static CLParserPtr initParser()
{
  CLParserPtr optionsParser = hdf5CLParserInstance();
  optionsParser->addArgument("halFile", "input hal file");
  optionsParser->addArgument("srcGenome", "source genome name");
  optionsParser->addArgument("srcBed", "path of input bed file.  write stdin "
                             "to stream from standard input");
  optionsParser->addArgument("tgtGenome", "target genome name");
  optionsParser->addArgument("tgtBed", "path of output bed file.  write stdout"
                             " to stream to standard output.");
  optionsParser->addOptionFlag("noDupes", "do not map between duplications in"
                               " graph.", false);
  optionsParser->addOptionFlag("append", "append results to tgtBed", false);
  optionsParser->addOption("inBedVersion", "bed version of input file "
                           "as integer between 3 and 9 or 12 reflecting "
                           "the number of columns (see bed "
                           "format specification for more details). Will "
                           "be autodetected by default.", 0);
  optionsParser->addOption("outBedVersion", "bed version of output file "
                           "as integer between 3 and 9 or 12 reflecting "
                           "the number of columns (see bed "
                           "format specification for more details). Will "
                           "be same as input by default.", 0);
  optionsParser->addOptionFlag("keepExtra", "keep extra columns. these are "
                               "columns in the input beyond the specified or "
                               "detected bed version, and which are cut by "
                               "default.", false);

  optionsParser->setDescription("Map BED genome interval coordinates between "
                                "two genomes.");
  return optionsParser;
}

int main(int argc, char** argv)
{
  CLParserPtr optionsParser = initParser();

  string halPath;
  string srcGenomeName;
  string srcBedPath;
  string tgtGenomeName;
  string tgtBedPath;
  bool noDupes;
  bool append;
  int inBedVersion;
  int outBedVersion;
  bool keepExtra;
  try
  {
    optionsParser->parseOptions(argc, argv);
    halPath = optionsParser->getArgument<string>("halFile");
    srcGenomeName = optionsParser->getArgument<string>("srcGenome");
    srcBedPath =  optionsParser->getArgument<string>("srcBed");
    tgtGenomeName = optionsParser->getArgument<string>("tgtGenome");
    tgtBedPath =  optionsParser->getArgument<string>("tgtBed");
    noDupes = optionsParser->getFlag("noDupes");
    append = optionsParser->getFlag("append");
    inBedVersion = optionsParser->getOption<int>("inBedVersion");
    outBedVersion = optionsParser->getOption<int>("outBedVersion");
    keepExtra = optionsParser->getFlag("keepExtra");
  }
  catch(exception& e)
  {
    cerr << e.what() << endl;
    optionsParser->printUsage(cerr);
    exit(1);
  }

  try
  {
    AlignmentConstPtr alignment = openHalAlignmentReadOnly(halPath, 
                                                           optionsParser);
    if (alignment->getNumGenomes() == 0)
    {
      throw hal_exception("hal alignmenet is empty");
    }

    const Genome* srcGenome = alignment->openGenome(srcGenomeName);
    if (srcGenome == NULL)
    {
      throw hal_exception(string("srcGenome, ") + srcGenomeName + 
                          ", not found in alignment");
    }
    const Genome* tgtGenome = alignment->openGenome(tgtGenomeName);
    if (tgtGenome == NULL)
    {
      throw hal_exception(string("tgtGenome, ") + tgtGenomeName + 
                          ", not found in alignment");
    }
    
    ifstream srcBed;
    istream* srcBedPtr;
    if (srcBedPath == "stdin")
    {
      srcBedPtr = &cin;
    }
    else
    {
      srcBed.open(srcBedPath.c_str());
      srcBedPtr = &srcBed;
      if (!srcBed)
      {
        throw hal_exception("Error opening srcBed, " + srcBedPath);
      }
    }
    
    ios_base::openmode mode = append ? ios::out | ios::app : ios_base::out;
    ofstream tgtBed;
    ostream* tgtBedPtr;
    if (tgtBedPath == "stdout")
    {
      tgtBedPtr = &cout;
    }
    else
    {      
      tgtBed.open(tgtBedPath.c_str(), mode);
      tgtBedPtr = &tgtBed;
      if (!tgtBed)
      {
        throw hal_exception("Error opening tgtBed, " + tgtBedPath);
      }
    }
    
    ColumnLiftover liftover;
    liftover.convert(alignment, srcGenome, srcBedPtr, tgtGenome, tgtBedPtr,
                     inBedVersion, outBedVersion, keepExtra, !noDupes);

  }
  catch(hal_exception& e)
  {
    cerr << "hal exception caught: " << e.what() << endl;
    return 1;
  }
  catch(exception& e)
  {
    cerr << "Exception caught: " << e.what() << endl;
    return 1;
  }

  return 0;
}
