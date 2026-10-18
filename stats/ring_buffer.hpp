/*

Fixed-size ring buffer backed by std::array, no dynamic allocation.
Supports push (overwrites oldest when full), operator[] in insertion order, size() and iteration.
Where: stats/include/stats/ring_buffer.hpp

RingBuffer<float, 3> rb;
rb.push(1); rb.push(2); rb.push(3); rb.push(4);
// contents: 2, 3, 4  ->  rb[0] == 2

*/