//
// Created by white on 25. 5. 20.
//

#ifndef OBJECTTAG_H
#define OBJECTTAG_H
#include <string>
struct FNV1aHasher {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const {
        size_t hash = 14695981039346656037ULL; // FNV offset basis
        for (char c : sv) {
            hash ^= static_cast<size_t>(c);
            hash *= 1099511628211ULL; // FNV prime
        }
        return hash;
    }
};

class Tag {
    public:
    size_t hash = 0;
    Tag()=default;

    explicit Tag(size_t v){this->hash = v;};
    bool operator==(const Tag& other) const {
        return other.hash == this->hash;
    }
    bool operator<(const Tag& other) const {
        return other.hash < this->hash;
    }
    operator bool() const { return hash != 0; }
};
class TagManager {
    public:
    inline static std::unordered_map<std::string, Tag, FNV1aHasher, std::equal_to<>> tagMap;
    static Tag RegisterTag(std::string_view tag) {
        if (auto it = tagMap.find(tag); it != tagMap.end()) {
            return it->second;
        }
        // FNV-1a 해시 계산
        size_t h = FNV1aHasher{}(tag);
        if (h == 0) h = 1; // 0은 Deafult용으로 보호

        Tag t(h);
        tagMap.emplace(tag, t);
        return t;
    }
    static Tag GetObjectTagFromString(std::string_view v) {
        auto it = tagMap.find(v);
        if (it != tagMap.end()) {
            return it->second;
        }
        return Tag(0); // Default Tag 반환
    }
};


#endif //OBJECTTAG_H
