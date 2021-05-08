#include <initializer_list>
#include <list>
#include <stdexcept>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>

class HashMap {
private:
    using element = typename std::pair<const KeyType, ValueType>;

public:
    using iterator = typename std::list<element>::iterator;
    using const_iterator = typename std::list<element>::const_iterator;

private:
    using bucket = std::pair<iterator, iterator>;  // хранит итераторы на границы подотрезка элементов с одним хэшом, включительно

public:
    size_t get_position(const KeyType key) const {
        return hasher(key) % buckets.size();
    }

    HashMap(Hash hasher = Hash()) : hasher(hasher) {
        buckets.assign(kDefaultSize, {end(), end()});
    }

    template <typename Iter>
    HashMap(Iter start, Iter finish, Hash hasher = Hash()) : hasher(hasher) {
        buckets.assign(kDefaultSize, {end(), end()});
        while (start != finish) {
            insert(*start++);
        }
    }

    HashMap(const std::initializer_list<element>& ndata, Hash hasher = Hash()) : HashMap(ndata.begin(), ndata.end(), hasher) {}

    HashMap(const HashMap& other) : HashMap(other.begin(), other.end(), hasher) {}

    HashMap& operator=(const HashMap& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        hasher = other.hasher;
        for (const auto& el : other) {
            insert(el);
        }
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

    iterator insert(const element& element_to_insert) {
        iterator element_pointer = find(element_to_insert.first);
        if (element_pointer != end()) {
            return element_pointer;
        }

        bucket& current_bucket = buckets[get_position(element_to_insert.first)];

        // вставляем вначало текущего "отрезка"
        current_bucket.first = elements.insert(current_bucket.first, element_to_insert);

        // если этот bucket появился впервые, то нужно сдвинуть указатель с end(),
        // т.к. границы хранятся включительно
        if (current_bucket.second == end()) {
            current_bucket.second--;
        }
        sz++;
        rehash();
        return find(element_to_insert.first);
    }

    void erase(const KeyType& key) {
        iterator iterator_to_erase = find(key);
        if (iterator_to_erase == end()) {
            return;
        }
        bucket& current_bucket = buckets[get_position(key)];

        if (current_bucket.first == current_bucket.second) {
            // если остался только один элемент в корзинке, то указатели этой корзинки нужно сдвинуть в конец списка
            current_bucket.first = end();
            current_bucket.second = end();
        } else if (iterator_to_erase == current_bucket.first) {
            // если удалился первый элемент, то сдвинуть начало подотрезка bucket-а
            current_bucket.first++;
        } else if (iterator_to_erase == current_bucket.second) {
            // eсли удалился последний, то тоже нужно сдвинуть границу
            current_bucket.second--;
        }
        sz--;
        elements.erase(iterator_to_erase);
    }

    iterator find(const KeyType& key) {
        bucket current_bucket = buckets[get_position(key)];
        if (current_bucket.first == end()) {
            return end();
        }
        for (iterator it = current_bucket.first; it != std::next(current_bucket.second); ++it) {
            if (it->first == key) {
                return it;
            }
        }
        return end();
    }

    const_iterator find(const KeyType& key) const {
        bucket current_bucket = buckets[get_position(key)];
        if (current_bucket.first == end()) {
            return end();
        }
        for (const_iterator it = current_bucket.first; it != std::next(current_bucket.second); ++it) {
            if (it->first == key) {
                return it;
            }
        }
        return end();
    }

    void rehash() {
        if (sz <= buckets.size()) {
            return;
        }
        std::vector<element> temporary_buffer;
        temporary_buffer.reserve(sz);
        for (const auto& element : elements) {
            temporary_buffer.push_back(element);
        }
        clear(2 * buckets.size());
        for (const auto& element : temporary_buffer) {
            insert(element);
        }
    }

    ValueType& operator[](const KeyType& key) {
        iterator pointer = find(key);
        if (pointer != end()) {
            return pointer->second;
        }
        return insert(std::make_pair(key, ValueType()))->second;
    }

    const ValueType& at(const KeyType& key) const {
        const_iterator pointer = find(key);
        if (pointer == end()) {
            throw std::out_of_range("there is no such element in HashMap");
        }
        return pointer->second;
    }

    void clear(size_t nsize = kDefaultSize) {
        sz = 0;
        elements.clear();
        buckets.assign(nsize, {end(), end()});
    }

    // all variables
private:
    static constexpr size_t kDefaultSize = 5;
    std::vector<bucket> buckets;
    std::list<element> elements;
    size_t sz = 0;
    Hash hasher;
};
