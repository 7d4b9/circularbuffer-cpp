#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <iterator>
#include <algorithm>

/*
 * CircularBuffer: audio buffer that allows to store the N last pushed samples, N being the
 *  _capacity (or less if the buffer is not full). When the buffer is full, the oldest samples are
 *  overwritten.
 * See https://en.wikipedia.org/wiki/Circular_buffer
 */
template<class SampleType>
class CircularBuffer
{
    friend class CircularBufferTest;
public:
  CircularBuffer(size_t capacity)
    : _capacity(capacity)
    , _data(new SampleType[capacity])
    , _end(_data + capacity -1)
    , _size(0)
    , _readPos(_data)
    , _writePos(_data)
  { }

  virtual ~CircularBuffer(){
      delete[]_data;
  }

  /* pushBuffer: push samples at the "end" of the circular buffer (which is not necessarily the
   *  end of the contiguous zone in memory).
   * buffer: C-style array containing the samples to be pushed
   * bufferSize: number of samples in buffer
   */
  void pushBuffer(SampleType const* bufferPtr, size_t bufferSize) {
      // Derivate the maximum storable element from the incomming buffer
      size_t totalInsertionsNum = std::min(bufferSize, _capacity);
      // trunkate the incomming buffer to its last values if its size is bigger
      // than this container capacity.
      if(bufferSize>_capacity) {
          bufferPtr += bufferSize - _capacity;
      }
      size_t bottomCapacity = std::distance(_writePos , _end) + 1;
      size_t bottomInsertionsNum = std::min(bottomCapacity, totalInsertionsNum);
      // bottom samples insertion
      std::copy(bufferPtr, bufferPtr + bottomInsertionsNum, _writePos);
      _size += bottomInsertionsNum;
      bufferPtr += bottomInsertionsNum;
      if(bottomInsertionsNum == bottomCapacity) {
          // inserted samples have reach the bottom of the underlying C-style
          // data array.
          if(_readPos >= _writePos) {
              // old values of the circular buffer have been overwriten
              _readPos = _data;
          }
          _writePos = _data;
          // perform the last insertions at the top of the underlying C-style
          // data array
          size_t frontInsertionsNum = totalInsertionsNum - bottomInsertionsNum;
          if(frontInsertionsNum) {
              _size += frontInsertionsNum;
              std::copy(bufferPtr, bufferPtr + frontInsertionsNum, _data);
              if(_readPos < _data + frontInsertionsNum) {
                  // old values have been overwriten
                  _readPos += frontInsertionsNum;
              }
              _writePos = _data + frontInsertionsNum;
          }
      } else {
          // values have not fullfilled the container bottom.
          if(_readPos > _writePos && std::distance(_writePos, _readPos) <= bottomInsertionsNum) {
              // old values have been overwriten
              _writePos += bottomInsertionsNum;
              _readPos = _writePos;
          } else {
              _writePos += bottomInsertionsNum;
          }
      }
  }

  /* popBuffer: copy oldest samples of the circular buffer to the pointer passed as an argument,
   *  then remove them from the circular buffer.
   * buffer: address to copy the samples to
   * bufferSize: number of samples to retrieve
   *
   * return the total number of samples actually readen
   */
  size_t popBuffer(SampleType* buffer, size_t bufferSize) {
      if(_size == 0) {
          return 0;
      }
      // Derivate the number of pop actually possible (could be less than the
      // bufferSize if not enougth value available)
      size_t totalToPopValNum = std::min(bufferSize, _size);
      // Total contigus values available from the read position to the end of
      // the contigus underlying C-style memory array.
      size_t bottomPopCapacity = std::distance(_readPos, std::min(_readPos + _size ,_end)) + 1;
      size_t popAtBottomNum = std::min(totalToPopValNum, bottomPopCapacity);
      _size -= popAtBottomNum;
      std::copy(_readPos, _readPos + popAtBottomNum, buffer);
      buffer += popAtBottomNum;
      if(popAtBottomNum < totalToPopValNum) {
          // Still remaing buffer to pop
          _readPos = _data;
          return popAtBottomNum + popBuffer(buffer, totalToPopValNum - popAtBottomNum);
      }
      _readPos += popAtBottomNum ;
      return totalToPopValNum;
  }

private:
  /* capacity: the maximum number of storable samples
   */
  const size_t _capacity;

  /* data: C-style array containing the samples. We don't use STL containers because we want to
   *  work on large numbers of samples efficiently (we want to be able to store several seconds of
   *  audio)
   */
  SampleType*const _data;

  /* limit: is the last C-style data array last element.
   * Formaly it is assumed that capacity = (limit - data) / sample-size
   */
  SampleType*const _end;

  /* size: the available stored samples number
   */
  size_t _size;

  /* readPos: position to perform read from (oldest pushed value position)
   */
  SampleType* _readPos;

  /* writePos: position to perform next write (position following the last pushed value)
   */
  SampleType* _writePos;
};

#endif // CIRCULARBUFFER_H
