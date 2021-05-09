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

    HashMap(const HashMap& other) : HashMap(other.begin(), other.end(), other.hasher) {}

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

private:
    // возвращает -1, если элемента нет, иначе его расстояние от начала bucket-а
    int inside_find(const KeyType& key) const {
        bucket current_bucket = buckets[get_position(key)];
        if (current_bucket.first == end()) {
            return -1;
        }
        int distance = 0;
        auto bucket_end = std::next(current_bucket.second);
        for (const_iterator it = current_bucket.first; it != bucket_end; ++it, distance++) {
            if (it->first == key) {
                return distance;
            }
        }
        return -1;
    }

public:
    iterator find(const KeyType& key) {
        iterator start_bucket = buckets[get_position(key)].first;
        int distance = inside_find(key);
        return distance == -1 ? end() : std::next(start_bucket, distance);
    }

    const_iterator find(const KeyType& key) const {
        const_iterator start_bucket = buckets[get_position(key)].first;
        int distance = inside_find(key);
        return distance == -1 ? end() : std::next(start_bucket, distance);
    }

private:
    // тот же insert, только с указанием места, private и без rehash() потому что нужен только для rehash()
    void copy_insert(const element& element_to_insert, std::list<element>& place_to_insert) {
        bucket& current_bucket = buckets[get_position(element_to_insert.first)];
        current_bucket.first = place_to_insert.insert(current_bucket.first, element_to_insert);
        if (current_bucket.second == place_to_insert.end()) {
            current_bucket.second--;
        }
        sz++;
    }

public:
    void rehash() {
        if (sz <= buckets.size()) {
            return;
        }
        std::list<element> moved_elements;
        sz = 0;
        buckets.assign(2 * buckets.size(), {moved_elements.end(), moved_elements.end()});
        for (element& current_element : elements) {
            copy_insert(current_element, moved_elements);
        }
        swap(elements, moved_elements);
        // потому что при list::swap нет гарантий на end(), а все остальные итераторы сохранятся
        for (auto& segment_pointers : buckets) {
            if (segment_pointers.first == moved_elements.end()) {
                segment_pointers.first = elements.end();
            }
            if (segment_pointers.second == moved_elements.end()) {
                segment_pointers.second = elements.end();
            }
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
