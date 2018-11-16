/*
 * Copyright (C) 2012 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#ifndef _HALDNAITERATOR_H
#define _HALDNAITERATOR_H
#include "halDefs.h"
#include "halGenome.h"
#include "halStorageDriver.h"

namespace hal {

/** 
 * Interface for general dna iterator
 */
class DNAIterator
{
public:
    DNAIterator(Genome* genome,
                DNAAccessPtr dnaAccess,
                hal_index_t index):
        _genome(genome),
        _dnaAccess(dnaAccess),
        _index(index),
        _reversed(false) {
    }

    /** Destructor */
    virtual ~DNAIterator() {
    }

    /** Ensure cache write data is flushed if needed.  This should be called
     * after a writing loop or an error will be generated.  This is not done
     * automatically on destruct, as error will be caught in DNAAccess.
     */
    void flush() {
        _dnaAccess->flush();
    }
    
    /** Get the DNA character at this position (if revsersed is set
    * the reverse compelement is returned */
    char getChar() const;

   /** Set the DNA character at this position (if revsersed is set
    * the reverse compelement is stored
    * @param c DNA character to set */
    void setChar(char c);

   /** Move to previous position (equiv. to toRight if reversed)*/
    void toLeft() {
        _reversed ? ++_index : --_index;
    }

   /** Move to next position (equiv. to toLeft if reversed)*/
    void toRight() {
        _reversed ? --_index : ++_index;
    }

   /** Jump to any point on the genome (can lead to 
    * inefficient paging from disk if used irresponsibly)
    * @param index position in array to jump to */
    void jumpTo(hal_size_t index) {
        _index = index;
    }

   /** switch to base's reverse complement */
    void toReverse() {
        _reversed = !_reversed;
    }

   /** Check whether iterator is on base's complement */
    bool getReversed() const {
        return _reversed;
    }

   /** Set the iterator's reverse complement status */
    void setReversed(bool reversed) {
        _reversed = reversed;
    }
     
   /** Get the containing (read-only) genome */
    const Genome* getGenome() const {
        return _genome;
    }

   /** Get the containing genome */
    Genome* getGenome() {
        return _genome;
    }

   /** Get the containing sequence */
    const Sequence* getSequence() const {
        return _genome->getSequenceBySite(_index);
    }

   /** Get the index of the base in the dna array */
    hal_index_t getArrayIndex() const {
        return _index;
    }

    /* read a DNA string */
    void readString(std::string& outString, hal_size_t length);

    /* write a DNA string */
    void writeString(const std::string& inString, hal_size_t length);

   /** Compare (array indexes) of two iterators */
    bool equals(DNAIteratorPtr& other) const ;

    /** Compare (array indexes) of two iterators */
    bool leftOf(DNAIteratorPtr& other) const;

    private:
    bool inRange() const {
        return (_index >= 0) && (_index < (hal_index_t)_genome->getSequenceLength());
    }

    Genome* _genome;
    DNAAccessPtr _dnaAccess;
    hal_index_t _index;
    bool _reversed;

};

inline char DNAIterator::getChar() const
{
  assert(inRange());
  char c = _dnaAccess->getBase(_index);
  if (_reversed) {
    c = reverseComplement(c);
  }
  return c;
}

inline void DNAIterator::setChar(char c)
{
  if (not inRange()) {
    throw hal_exception("Trying to set character out of range");
  } else if (not isNucleotide(c)) {
    throw hal_exception(std::string("Trying to set invalid character: ") + c);
  }
  if (_reversed){
    c = reverseComplement(c);
  }
  _dnaAccess->setBase(_index, c);
  assert(getChar() == !_reversed ? c : reverseComplement(c));
}

inline bool DNAIterator::equals(DNAIteratorPtr& other) const
{
  const DNAIterator* mmOther = reinterpret_cast<
     const DNAIterator*>(other.get());
  assert(_genome == mmOther->_genome);
  return _index == mmOther->_index;
}

inline bool DNAIterator::leftOf(DNAIteratorPtr& other) const
{
  const DNAIterator* mmOther = reinterpret_cast<
     const DNAIterator*>(other.get());
  assert(_genome == mmOther->_genome);
  return _index < mmOther->_index;
}

inline void DNAIterator::readString(std::string& outString,
                                    hal_size_t length)
{
  assert(length == 0 || inRange() == true);
  outString.resize(length);

  for (hal_size_t i = 0; i < length; ++i) {
    outString[i] = getChar();
    toRight();
  }
}

inline void DNAIterator::writeString(const std::string& inString,
                                     hal_size_t length)
{
  assert(length == 0 || inRange());
  for (hal_size_t i = 0; i < length; ++i) {
    setChar(inString[i]);
    toRight();
  }
  flush();
}

}

#endif
