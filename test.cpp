#include <stdint.h>
#include <cstring>
#include <iostream>
#include "circularbuffer.h"
#include <cassert>

using namespace std;

class CircularBufferTest : private CircularBuffer<uint32_t> {
    static const size_t MAX_BUFFERED_AUDIO_SAMPLES = 10;

public:
    CircularBufferTest() : CircularBuffer(MAX_BUFFERED_AUDIO_SAMPLES) { }

    void Test() {

        const uint32_t audio[]={0x1,0x2,0x3,0x4,
                                0x5,0x6,0x7,0x8,
                                0x9,0xA,0xB,0xC,
                                0xD,0xE,0xF,0x10};
        assert(_size == 0);
        pushBuffer(audio,16);
        assert(_size == MAX_BUFFERED_AUDIO_SAMPLES);
        assert(_data[0] == audio[6]);
        assert(_data[MAX_BUFFERED_AUDIO_SAMPLES-1] == audio[15]);
        assert(_writePos == _data);

        uint32_t sample[6];
        memset(sample, 0, sizeof(sample));

        assert(5 == popBuffer(sample,5));
        assert(_size == MAX_BUFFERED_AUDIO_SAMPLES-5);
        assert(_readPos == _data + 5);

        assert(sample[0]==audio[6]);
        assert(sample[4]==audio[10]);
        assert(sample[5]==0);

        pushBuffer(audio + 7, 3);
        assert(_size == MAX_BUFFERED_AUDIO_SAMPLES-2);
        assert(_data[0]==audio[7]);
        assert(_data[2]==audio[9]);
        assert(_data[3]==audio[9]);
        assert(_readPos ==  _data + 5);
        assert(_writePos ==  _data + 3);

        memset(sample, 0, sizeof(sample));
        assert(4 == popBuffer(sample,4));

        assert(_size == MAX_BUFFERED_AUDIO_SAMPLES-6);
        assert(_readPos ==  _data + 9);
        assert(_writePos ==  _data + 3);

        assert(sample[0]==_data[5]);
        assert(sample[3]==_data[8]);
        assert(sample[4] == 0 && sample[5] == 0);

        memset(sample, 0, sizeof(sample));
        assert(4 == popBuffer(sample,6));

        assert(_size == 0);
        assert(_readPos ==  _data + 3);
        assert(_writePos ==  _readPos);

        const uint32_t audio2[]={0x11,0x12,0x13,0x14,
                                 0x15,0x16,0x17,0x18,
                                 0x19,0x1A,0x1B,0x1C,
                                 0x1D,0x1E,0x1F,0x20};

        pushBuffer(audio2 + 7, 4);

        assert(_size == 4);
        assert(_readPos ==  _data + 3);
        assert(_writePos ==  _data + 7);

        memset(sample, 0, sizeof(sample));
        assert(1 == popBuffer(sample,1));
        assert(_size == 3);
        assert(_readPos ==  _data + 4);
        assert(_writePos ==  _data + 7);

        memset(sample, 0, sizeof(sample));
        assert(2 == popBuffer(sample,2));
        assert(_size == 1);
        assert(_readPos ==  _data + 6);
        assert(_writePos ==  _data + 7);

        assert(sample[0]==_data[4]);
        assert(sample[2] == 0 && sample[5] == 0);

        memset(sample, 0, sizeof(sample));

        assert(1 == popBuffer(sample,5));
        assert(_size == 0);
        assert(_readPos ==  _writePos);
        assert(_writePos ==  _data + 7);

        assert(sample[0]==_data[6]);
        assert(sample[1] == 0 && sample[5] == 0);
    }
};

int main(int argc, char *argv[])
{
    CircularBufferTest().Test();
    cout << "TEST OK" << endl;
    return 0;
}
