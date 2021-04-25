/*
MIT License

Copyright (c) 2018 Meng Rao <raomeng1@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

// SHMMPSCQueue is not "crash safe", which means:
// Participant process crash could cause memory leak or even deadlock of other participants
// If deadlock happens(under theoretical possibility), kill all participants and remove the shm file under /dev/shm/
template<class EntryData, uint32_t SIZE>
class SHMMPSCQueue
{
public:
    struct Entry
    {
        uint32_t next;
        EntryData data;
    };

    // SHMMPSCQueue's memory must be zero initialized, e.g. as a global variable or by shm_open/mmap
    // so init_state will be 0 at first
    SHMMPSCQueue() {
        if(!__sync_bool_compare_and_swap(&init_state, 0, 1)) {
            while(init_state != 2)
                ;
            return;
        }
        pending_tail = 0;
        empty_top = 1;
        for(uint32_t i = 1; i < SIZE; i++) {
            entries[i].next = i + 1;
        }
        asm volatile("" : : : "memory"); // memory fence
        init_state = 2;
    }

    Entry* Alloc() {
        uint32_t top = FetchAndLock(empty_top, EmptyLock);
        if(top == SIZE) {
            empty_top = SIZE;
            return nullptr;
        }
        empty_top = entries[top].next;
        return &entries[top];
    }

    void Push(Entry* entry) {
        uint32_t entry_idx = entry - entries;
        uint32_t tail = FetchAndLock(pending_tail, PendingLock);
        entries[tail].next = entry_idx;
        asm volatile("" : : "m"(entries[tail].next), "m"(pending_tail) :); // memory fence
        pending_tail = entry_idx;
    }

    Entry* PopAll() {
        if(pending_tail == 0) return nullptr;
        uint32_t tail = FetchAndLock(pending_tail, PendingLock);
        Entry* ret = &entries[entries[0].next];
        asm volatile("" : : "m"(pending_tail) :); // memory fence
        pending_tail = 0;
        entries[tail].next = SIZE;
        return ret;
    }

    Entry* NextEntry(Entry* entry) {
        if(entry->next == SIZE) return nullptr;
        return &entries[entry->next];
    }

    void Recycle(Entry* first, Entry* last) {
        uint32_t top = FetchAndLock(empty_top, EmptyLock);
        last->next = top;
        asm volatile("" : : "m"(last->next), "m"(empty_top) :); // memory fence
        empty_top = first - entries;
    }

private:
    template<class T>
    static T FetchAndLock(volatile T& var, T lock_val) {
        T val = var;
        while(true) {
            while(__builtin_expect(val == lock_val, 0)) val = var;
            T new_val = __sync_val_compare_and_swap(&var, val, lock_val);
            if(__builtin_expect(new_val == val, 1)) break;
            val = new_val;
        }
        return val;
    }

private:
    static constexpr uint32_t PendingLock = SIZE + 1;
    static constexpr uint32_t EmptyLock = SIZE + 2;

    int volatile init_state;

    alignas(64) uint32_t volatile pending_tail;

    alignas(64) uint32_t volatile empty_top;

    alignas(64) Entry entries[SIZE];
};

