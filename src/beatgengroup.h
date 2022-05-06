#ifndef _BEATGENGROUP_H_
#define _BEATGENGROUP_H_
#pragma once

#include <memory>
#include "beatgen.h"


class BeatGenGroup {
    public:
        BeatGenGroup(int numberOfBeatGens);
        ~BeatGenGroup();

        bool isSoloed() const {
            bool ret = false;
            for(int i = 0; i < _beatGenVector.size(); i++) ret |= _beatGenVector.at(i)->isSolo();
            return ret;
        }

        int size() const {
            return (int)_beatGenVector.size();
        }

        BeatGen &operator[](int index) {
            return *_beatGenVector[index];
        }

        const BeatGen &operator[](int index) const {
            return *_beatGenVector.at(index);
        }

    private:
        typedef std::unique_ptr<BeatGen> BeatGenPtr;

        std::vector<BeatGenPtr>     _beatGenVector;
        
};

#endif
