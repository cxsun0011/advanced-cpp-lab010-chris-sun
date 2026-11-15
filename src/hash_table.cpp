#include "project/hash_table.h"

#include <cmath>
#include <stdexcept>
#include <utility>

HashTable::HashTable(std::size_t capacity)
    : table_(capacity), size_(0) {
    if (capacity == 0) {
        throw std::invalid_argument("Hash table capacity must be greater than zero");
    }
    for (std::size_t i = 0; i < table_.size(); ++i) {
        table_[i] = HashEntry();
    }
}

HashTable::~HashTable() = default;

std::size_t HashTable::hashFunction(const std::string& key) const {
    std::size_t hash = 2166136261u;
    for (unsigned char ch : key) {
        hash ^= ch;
        hash *= 16777619u;
    }
    return hash % table_.size();
}

std::size_t HashTable::probe(const std::string& key) const {
    std::size_t index = hashFunction(key);
    std::size_t original = index;
    std::size_t step = 1;

    while (table_[index].occupied && table_[index].key != key) {
        index = (original + step * step) % table_.size();
        ++step;
    }
    return index;
}

bool HashTable::insert(const std::string& key, int value) {
    if (static_cast<double>(size_ + 1) / table_.size() > 0.5) {
        rehash();
    }

    std::size_t index = hashFunction(key);
    std::size_t original = index;
    std::size_t step = 1;
    long long firstDeleted = -1;

    while (table_[index].occupied || table_[index].deleted) {
        if (table_[index].occupied && !table_[index].deleted && table_[index].key == key) {
            return false;
        }
        if (table_[index].deleted && firstDeleted == -1) {
            firstDeleted = static_cast<long long>(index);
        }
        index = (original + step * step) % table_.size();
        ++step;
        if (step > table_.size()) break;
    }

    std::size_t targetIndex = (firstDeleted != -1) ? static_cast<std::size_t>(firstDeleted) : index;

    table_[targetIndex] = HashEntry(key, value, true, false);
    ++size_;
    return true;
}

bool HashTable::remove(const std::string& key) {
    std::size_t index = hashFunction(key);
    std::size_t original = index;
    std::size_t step = 1;

    while (table_[index].occupied || table_[index].deleted) {
        if (table_[index].occupied && !table_[index].deleted && table_[index].key == key) {
            table_[index].occupied = false;
            table_[index].deleted = true;
            table_[index].key = "";
            --size_;
            return true;
        }
        index = (original + step * step) % table_.size();
        ++step;
        if (step > table_.size()) break;
    }
    return false;
}

bool HashTable::contains(const std::string& key) const {
    std::size_t index = hashFunction(key);
    std::size_t original = index;
    std::size_t step = 1;

    while (table_[index].occupied || table_[index].deleted) {
        if (table_[index].occupied && !table_[index].deleted && table_[index].key == key) {
            return true;
        }
        index = (original + step * step) % table_.size();
        ++step;
        if (step > table_.size()) break;
    }
    return false;
}

int HashTable::get(const std::string& key) const {
    std::size_t index = hashFunction(key);
    std::size_t original = index;
    std::size_t step = 1;

    while (table_[index].occupied || table_[index].deleted) {
        if (table_[index].occupied && !table_[index].deleted && table_[index].key == key) {
            return table_[index].value;
        }
        index = (original + step * step) % table_.size();
        ++step;
        if (step > table_.size()) break;
    }
    throw std::out_of_range("Key not found in hash table");
}

bool HashTable::update(const std::string& key, int newValue) {
    std::size_t index = hashFunction(key);
    std::size_t original = index;
    std::size_t step = 1;

    while (table_[index].occupied || table_[index].deleted) {
        if (table_[index].occupied && !table_[index].deleted && table_[index].key == key) {
            table_[index].value = newValue;
            return true;
        }
        index = (original + step * step) % table_.size();
        ++step;
        if (step > table_.size()) break;
    }
    return false;
}

std::size_t HashTable::size() const noexcept {
    return size_;
}

std::size_t HashTable::capacity() const noexcept {
    return table_.size();
}

bool HashTable::empty() const noexcept {
    return size_ == 0;
}

double HashTable::loadFactor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(table_.size());
}

std::vector<std::string> HashTable::keys() const {
    std::vector<std::string> result;
    for (const auto& entry : table_) {
        if (entry.occupied && !entry.deleted) {
            result.push_back(entry.key);
        }
    }
    return result;
}

std::vector<int> HashTable::values() const {
    std::vector<int> result;
    for (const auto& entry : table_) {
        if (entry.occupied && !entry.deleted) {
            result.push_back(entry.value);
        }
    }
    return result;
}

void HashTable::rehash() {
    std::size_t newCapacity = nextPrime(table_.size() * 2);
    std::vector<HashEntry> oldTable = std::move(table_);

    table_ = std::vector<HashEntry>(newCapacity, HashEntry());
    size_ = 0;

    for (const auto& entry : oldTable) {
        if (entry.occupied && !entry.deleted) {
            insert(entry.key, entry.value);
        }
    }
}

bool HashTable::isPrime(std::size_t value) const {
    if (value < 2) {
        return false;
    }
    if (value == 2) {
        return true;
    }
    if (value % 2 == 0) {
        return false;
    }
    for (std::size_t i = 3; i * i <= value; i += 2) {
        if (value % i == 0) {
            return false;
        }
    }
    return true;
}

std::size_t HashTable::nextPrime(std::size_t value) const {
    if (value <= 2) {
        return 2;
    }
    std::size_t candidate = value % 2 == 0 ? value + 1 : value;
    while (!isPrime(candidate)) {
        candidate += 2;
    }
    return candidate;
}
