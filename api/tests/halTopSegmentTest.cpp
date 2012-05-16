/*
 * Copyright (C) 2012 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.txt
 */
#include <string>
#include <iostream>
#include <cstdlib>
#include "halTopSegmentTest.h"

using namespace std;
using namespace hal;

// just create a bunch of garbage data.  we don't care 
// about logical consistency for this test, just whether or not
// it's read and written properly. 
void TopSegmentStruct::setRandom()
{
  _length = rand();
  _startPosition = rand();
  _nextParalogyIndex = rand();
  _parentIndex = rand();
  _arrayIndex = rand();
  _bottomParseIndex = rand();
  _bottomParseOffset = rand();
}

void TopSegmentStruct::set(hal_index_t startPosition,
                           hal_size_t length,
                           hal_index_t parentIndex,
                           hal_index_t bottomParseIndex,
                           hal_offset_t bottomParseOffset,
                           hal_index_t nextParalogyIndex)
{
  _startPosition = startPosition;
  _length = length;
  _parentIndex = parentIndex;
  _bottomParseIndex = bottomParseIndex;
  _bottomParseOffset = bottomParseOffset;
  _nextParalogyIndex = nextParalogyIndex;
}

void TopSegmentStruct::applyTo(TopSegmentIteratorPtr it) const
{
  TopSegment* seg = it->getTopSegment();
  seg->setLength(_length);
  seg->setStartPosition(_startPosition);
  seg->setNextParalogyIndex(_nextParalogyIndex);
  seg->setParentIndex(_parentIndex);
  seg->setBottomParseIndex(_bottomParseIndex);
  seg->setBottomParseOffset(_bottomParseOffset);
}

void TopSegmentStruct::compareTo(TopSegmentIteratorConstPtr it, 
                                 CuTest* testCase) const
{
  const TopSegment* seg = it->getTopSegment();
  CuAssertTrue(testCase, _length == seg->getLength());
  CuAssertTrue(testCase, _startPosition == seg->getStartPosition());
  CuAssertTrue(testCase, _nextParalogyIndex == seg->getNextParalogyIndex());
  CuAssertTrue(testCase, _parentIndex == seg->getParentIndex());
  CuAssertTrue(testCase, _bottomParseIndex == seg->getBottomParseIndex());
  CuAssertTrue(testCase, _bottomParseOffset == seg->getBottomParseOffset());
}

void TopSegmentSimpleIteratorTest::createCallBack(AlignmentPtr alignment)
{
  Genome* ancGenome = alignment->addRootGenome("Anc0", 0);
  vector<Sequence::Info> seqVec(1);
  seqVec[0] = Sequence::Info("Sequence", 1000000, 5000, 700000);
  ancGenome->setDimensions(seqVec);
  
  _topSegments.clear();
  for (size_t i = 0; i < ancGenome->getNumTopSegments(); ++i)
  {
    TopSegmentStruct topSeg;
    topSeg.setRandom();
    _topSegments.push_back(topSeg);
  }
  
  TopSegmentIteratorPtr tsIt = ancGenome->getTopSegmentIterator();
  TopSegmentIteratorConstPtr tsEnd = ancGenome->getTopSegmentEndIterator();
  for (size_t i = 0; tsIt != tsEnd; tsIt->toRight(), ++i)
  {
    CuAssertTrue(_testCase, 
                 (size_t)tsIt->getTopSegment()->getArrayIndex() == i);
    _topSegments[i].applyTo(tsIt);
  }
}

void TopSegmentSimpleIteratorTest::checkCallBack(AlignmentConstPtr alignment)
{
  const Genome* ancGenome = alignment->openGenome("Anc0");
  CuAssertTrue(_testCase, ancGenome->getNumTopSegments() == _topSegments.size());
  TopSegmentIteratorConstPtr tsIt = ancGenome->getTopSegmentIterator(0);
  for (size_t i = 0; i < ancGenome->getNumTopSegments(); ++i)
  {
    CuAssertTrue(_testCase, 
                 (size_t)tsIt->getTopSegment()->getArrayIndex() == i);
    _topSegments[i].compareTo(tsIt, _testCase);
    tsIt->toRight();
  }
  tsIt = ancGenome->getTopSegmentIterator(ancGenome->getNumTopSegments() - 1);
  for (hal_index_t i = ancGenome->getNumTopSegments() - 1; i >= 0; --i)
  {
    CuAssertTrue(_testCase, tsIt->getTopSegment()->getArrayIndex() == i);
    _topSegments[i].compareTo(tsIt, _testCase);
    tsIt->toLeft();
  }
}

void TopSegmentSequenceTest::createCallBack(AlignmentPtr alignment)
{
  Genome* ancGenome = alignment->addRootGenome("Anc0", 0);
  vector<Sequence::Info> seqVec(1);
  seqVec[0] = Sequence::Info("Sequence", 1000000, 5000, 700000);
  ancGenome->setDimensions(seqVec);

  ancGenome->setSubString("CACACATTC", 500, 9);
  TopSegmentIteratorPtr tsIt = ancGenome->getTopSegmentIterator(100);
  tsIt->getTopSegment()->setStartPosition(500);
  tsIt->getTopSegment()->setLength(9);
}

void TopSegmentSequenceTest::checkCallBack(AlignmentConstPtr alignment)
{
  const Genome* ancGenome = alignment->openGenome("Anc0");
  TopSegmentIteratorConstPtr tsIt = ancGenome->getTopSegmentIterator(100);
  CuAssertTrue(_testCase, tsIt->getTopSegment()->getStartPosition() == 500);
  CuAssertTrue(_testCase, tsIt->getTopSegment()->getLength() == 9);
  string seq;
  tsIt->getString(seq);
  CuAssertTrue(_testCase, seq == "CACACATTC");
  tsIt->toReverse();
  tsIt->getString(seq);
  CuAssertTrue(_testCase, seq == "GAATGTGTG");
}

void halTopSegmentSimpleIteratorTest(CuTest *testCase)
{
  try 
  {
    TopSegmentSimpleIteratorTest tester;
    tester.check(testCase);
  }
  catch (...) 
  {
    CuAssertTrue(testCase, false);
  } 
}

void halTopSegmentSequenceTest(CuTest *testCase)
{
  try 
  {
    TopSegmentSequenceTest tester;
    tester.check(testCase);
  }
  catch (...) 
  {
    CuAssertTrue(testCase, false);
  } 
}

CuSuite* halTopSegmentTestSuite(void) 
{
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, halTopSegmentSimpleIteratorTest);
  SUITE_ADD_TEST(suite, halTopSegmentSequenceTest);
  return suite;
}

