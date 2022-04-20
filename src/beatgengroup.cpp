#include "beatgengroup.h"

BeatGenGroup::BeatGenGroup(int count) {
    for(int i = 0; i < count; i++) {
        _beatGenVector.push_back(std::make_unique<BeatGen>(i));
    }
}

BeatGenGroup::~BeatGenGroup() {

}
