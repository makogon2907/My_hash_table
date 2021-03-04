#include <initializer_list>
#include <list>
#include <stdexcept>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    using Element = typename std::pair<const KeyType, ValueType>;
    using iterator = typename std::list<Element>::iterator;
    using const_iterator = typename std::list<Element>::const_iterator;
    using Bucket = std::pair<iterator, iterator>;

    size_t myhash(const KeyType key) const {
        return hasher(key) % containers.size();
    }

    HashMap(Hash hasher = Hash()) : hasher(hasher) {
        containers.assign(default_size, {end(), end()});
    }

    template <typename Iter>
    HashMap(Iter start, Iter finish, Hash hasher = Hash()) : hasher(hasher) {
        containers.assign(default_size, {end(), end()});
        while (start != finish)
            insert(*start++);
    }

    HashMap(const std::initializer_list<Element>& ndata, Hash hasher = Hash()) : HashMap(ndata.begin(), ndata.end(), hasher) {}

    HashMap(const HashMap& other) : hasher(other.hasher) {
        if (this == &other)
            return;
        sz = other.sz;
        containers.assign(other.containers.size(), {end(), end()});
        for (const auto& el : other)
            insert(el);
    }

    HashMap& operator=(const HashMap& other) {
        if (this == &other)
            return *this;
        clear();
        hasher = other.hasher;
        for (const auto& el : other)
            insert(el);
        return *this;
    }

    size_t size() const {
        return sz;
    }
    bool empty() const {
        return (sz == 0);
    }

    Hash hash_function() const {
        return hasher;
    }

    iterator begin() {
        return elements.begin();
    }
    iterator end() {
        return elements.end();
    }
    const_iterator begin() const {
        return elements.begin();
    }
    const_iterator end() const {
        return elements.end();
    }

    void insert(const Element& el, bool dorehash = true) {
        if (find(el.first) != end())
            return;
        Bucket& cur_bucket = containers[myhash(el.first)];
        cur_bucket.first = elements.insert(cur_bucket.first, el);
        if (cur_bucket.second == end())
            cur_bucket.second--;
        sz++;
        if (dorehash)
            rehash();
    }

    void erase(const KeyType& key) {
        iterator cur = find(key);
        if (cur == end())
            return;
        Bucket& cur_bucket = containers[myhash(key)];
        // if there is only one element in this bucket
        if (cur_bucket.first == cur_bucket.second)
            cur_bucket.first = cur_bucket.second = end();
        else if (cur == cur_bucket.first)
            cur_bucket.first++;
        else if (cur == cur_bucket.second)
            cur_bucket.second--;
        sz--;
        elements.erase(cur);
    }

    iterator find(const KeyType& key) {
        Bucket bucket = containers[myhash(key)];
        if (bucket.first == end())
            return end();
        for (iterator it = bucket.first; it != std::next(bucket.second); ++it)
            if ((*it).first == key)
                return it;
        return end();
    }

    const_iterator find(const KeyType& key) const {
        Bucket bucket = containers[myhash(key)];
        if (bucket.first == end())
            return end();
        for (const_iterator it = bucket.first; it != std::next(bucket.second); ++it)
            if ((*it).first == key)
                return it;
        return end();
    }

    void rehash() {
        if (sz <= containers.size())
            return;
        std::vector<Element> tmp;
        tmp.reserve(sz);
        for (const auto& element : elements)
            tmp.push_back(element);
        clear(2 * containers.size());
        for (const auto& element : tmp)
            insert(element, false);
    }

    ValueType& operator[](const KeyType& key) {
        iterator ptr = find(key);
        if (ptr != end())
            return (*ptr).second;
        insert(std::make_pair(key, ValueType()));
        return (*find(key)).second;  // if there will be rehash
    }

    const ValueType& at(const KeyType& key) const {
        const_iterator ptr = find(key);
        if (ptr == end()) {
            throw std::out_of_range("there is no such element in HashMap");
        }
        return (*ptr).second;
    }

    void clear(size_t nsize = default_size) {
        sz = 0;
        elements.clear();
        containers.assign(nsize, {end(), end()});
    }

    // all variables 
private:
    static const size_t default_size = 5;
    std::vector<Bucket> containers;
    std::list<Element> elements;
    size_t sz = 0;
    Hash hasher;
};
