#include <initializer_list>
#include <list>
#include <stdexcept>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    using element = typename std::pair<const KeyType, ValueType>;
    using iterator = typename std::list<element>::iterator;
    using const_iterator = typename std::list<element>::const_iterator;
    using bucket = std::pair<iterator, iterator>;  // хранит итераторы на границы подотрезка элементов с одним хэшом, включительно

    size_t get_position(const KeyType key) const {
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

    HashMap(const std::initializer_list<element>& ndata, Hash hasher = Hash()) : HashMap(ndata.begin(), ndata.end(), hasher) {}

    HashMap(const HashMap& other) : HashMap(other.begin(), other.end(), hasher) {}

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

    void insert(const element& element_to_insert) {
        if (find(element_to_insert.first) != end()) {
            return;
        }
        bucket& current_bucket = containers[get_position(element_to_insert.first)];

        // вставляем вначало текущего "отрекзка"
        current_bucket.first = elements.insert(current_bucket.first, element_to_insert);

        // если этот bucket появился впервые, то нужно сдвинуть указатель с end(),
        //т.к. границы хранятся включительно
        if (current_bucket.second == end()) {
            current_bucket.second--;
        }
        sz++;
        rehash();
    }

    void erase(const KeyType& key) {
        iterator iterator_to_erase = find(key);
        if (iterator_to_erase == end())
            return;

        bucket& cur_bucket = containers[get_position(key)];

        if (cur_bucket.first == cur_bucket.second) {
            // если остался только один элемент в корзинке, то указатели этой корзинки нужно сдвинуть в конец списка
            cur_bucket.first = end();
            cur_bucket.second = end();
        } else if (iterator_to_erase == cur_bucket.first) {
            // если удалился первый элемент, то сдвинуть начало подотрезка bucket-а
            cur_bucket.first++;
        } else if (iterator_to_erase == cur_bucket.second) {
            // eсли удалился последний, то тоже нужно сдвинуть границу
            cur_bucket.second--;
        }
        sz--;
        elements.erase(iterator_to_erase);
    }

    iterator find(const KeyType& key) {
        bucket bucket = containers[get_position(key)];
        if (bucket.first == end())
            return end();
        for (iterator it = bucket.first; it != std::next(bucket.second); ++it)
            if ((*it).first == key)
                return it;
        return end();
    }

    const_iterator find(const KeyType& key) const {
        bucket bucket = containers[get_position(key)];
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
        std::vector<element> temporary_buffer;
        temporary_buffer.reserve(sz);
        for (const auto& element : elements)
            temporary_buffer.push_back(element);
        clear(2 * containers.size());
        for (const auto& element : temporary_buffer)
            insert(element);
    }

    ValueType& operator[](const KeyType& key) {
        iterator ptr = find(key);
        if (ptr != end())
            return ptr->second;
        insert(std::make_pair(key, ValueType()));
        return find(key)->second;  // if there will be rehash
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
    std::vector<bucket> containers;
    std::list<element> elements;
    size_t sz = 0;
    Hash hasher;
};
