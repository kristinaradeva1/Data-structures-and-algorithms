#pragma once
#include <iostream>
#include <memory>

namespace Constants
{
    constexpr size_t GROWTH_FACTOR = 2;
    constexpr unsigned elementsInBucket = 8 * sizeof(uint8_t);
}

template<class AllocatorType = std::allocator<uint8_t>>
class BooleanVector
{
private:
    uint8_t* _buckets;
    size_t _size; //пази стойностите на всички булеви стойности тоест = bucketsCount * 8
    size_t _bucketsCount;
    size_t _capacity; //пази всички възможни битове от заделената памет

    AllocatorType allocator;

    void copy(const BooleanVector& other);
    void move(BooleanVector&& other);
    void free();

    unsigned getBucketIndex(unsigned value) const;
    unsigned getBitIndex(unsigned value) const;

    size_t calculate_capacity() const;

    bool contains(unsigned value) const;
public:
    BooleanVector() = default;
    explicit BooleanVector(size_t count);

    BooleanVector(const BooleanVector& other);
    BooleanVector& operator=(const BooleanVector& other);

    BooleanVector(BooleanVector&& other);
    BooleanVector& operator=(BooleanVector&& other);

    ~BooleanVector();

    void push_back(bool value);
    void pop_back();

    void pop_front();
    void resize(size_t n);
    void print() const;

    bool operator[](size_t index);
    bool operator[](size_t index) const;

    size_t size() const;
    size_t capacity() const;
    bool empty() const;

    class const_boolean_vector_iterator
    {
        friend class BooleanVector;
    private:
        uint8_t* memPointer;
        size_t bitIndex; // [0 .. 7]
        size_t bucketIndex;
        bool value;
        const BooleanVector& vector;
    public:
        const_boolean_vector_iterator(const BooleanVector& vec, uint8_t* passedVal) : vector(vec), memPointer(passedVal)
        {
            if (memPointer == _buckets)
            {
                bitIndex = 0;
                bucketIndex = 0;

                uint8_t mask = (1 << bitIndex);
                value = (mask & vec._buckets[bucketIndex]);
            }
            else
            {
                bitIndex = vec.getBitIndex(vec._size - 1);
                bucketIndex = vec.getBucketIndex(vec._size - 1);

                uint8_t mask = (1 << bitIndex);
                value = (mask & vec._buckets[bucketIndex]);
            }
        }
        const_boolean_vector_iterator(const BooleanVector& vec, uint8_t* passedVal, size_t push) : vector(vec), memPointer(passedVal)
        {
            if (memPointer == vec._buckets)
            {
                bitIndex = vec.getBitIndex(push - 1);
                bucketIndex = 0;

                uint8_t mask = (1 << bitIndex);
                value = (mask & vec._buckets[bucketIndex]);
            }
            else
            {
                bitIndex = vec.getBitIndex(vec._size - push);
                bucketIndex = vec.getBucketIndex(vec._size - 1);

                uint8_t mask = (1 << bitIndex);
                value = (mask & vec._buckets[bucketIndex]);
            }
        }

        int operator-(const_boolean_vector_iterator other) const
        {
            return memPointer - other.memPointer;
        }

        bool operator*() const noexcept
        {
            return value;
        }

        bool operator==(const const_boolean_vector_iterator& other) const
        {
            return (this->memPointer == other.memPointer && this->bitIndex == other.bitIndex);
        }

        bool operator!=(const const_boolean_vector_iterator& other) const
        {
            return (this->memPointer != other.memPointer || this->bitIndex != other.bitIndex);
        }
    };

    class boolean_vector_iterator
    {
        friend class BooleanVector;
    private:
        uint8_t* memPointer;
        int bitIndex; // [0 .. 7]
        size_t bucketIndex;
        bool value;
        const BooleanVector& vector;
    private:
        boolean_vector_iterator(BooleanVector& vec, size_t push) : vector(vec)
        {
            bitIndex = push;
            if (push == 0)
            {
                bucketIndex = 0;
                memPointer = vec._buckets;
                uint8_t mask = (1 << bitIndex);
                value = (mask & vec._buckets[0]);
            }
            else
            {
                bucketIndex = vec.getBucketIndex(bitIndex);
                bitIndex %= Constants::elementsInBucket;
                memPointer = vec._buckets + bucketIndex;
                value = 0;
            }
        }
    public:
        boolean_vector_iterator(BooleanVector& vec, uint8_t* passedVal) : vector(vec), memPointer(passedVal)
        {
            if (memPointer == vec._buckets)
            {
                bitIndex = 0;
                bucketIndex = 0;

                uint8_t mask = (1 << bitIndex);
                value = (mask & vec._buckets[bucketIndex]);
            }
            else
            {
                bitIndex = vec.getBitIndex(vec._size - 1);
                bucketIndex = vec.getBucketIndex(vec._size - 1);

                uint8_t mask = (1 << bitIndex);
                value = (mask & vec._buckets[bucketIndex]);
            }
        };

        bool operator*() const
        {
            return value;
        }


        boolean_vector_iterator& operator++() //prefix operator 7++
        {
            bitIndex++;
            if ((bucketIndex * Constants::elementsInBucket + bitIndex) >= vector._size)
                throw std::out_of_range("Reaching beyond the vector's size");

            if (bitIndex > (Constants::elementsInBucket - 1))
            {
                bitIndex %= Constants::elementsInBucket;
                memPointer++;
                bucketIndex++;
            }
            uint8_t mask = (1 << bitIndex);
            value = (mask & vector._buckets[bucketIndex]);
            return *this;
        }

        boolean_vector_iterator& operator--()
        {
            if (bitIndex == 0 && bucketIndex == 0)
                throw std::out_of_range("Attempted to reach beyond the beginning of the vector.");

            if (bitIndex == 0)
            {
                bitIndex = Constants::elementsInBucket - 1;
                memPointer--;
                bucketIndex--;
            }
            else
            {
                bitIndex--;
            }
            uint8_t mask = (1 << bitIndex);
            value = (vector._buckets[bucketIndex] & mask);
            return *this;
        }

        boolean_vector_iterator operator++(int) //postfix operator 
        {
            boolean_vector_iterator toReturn(*this);
            bitIndex++;
            if ((bucketIndex * Constants::elementsInBucket + bitIndex) >= vector._size)
                throw std::out_of_range("Reaching beyond the vector's size");

            if (bitIndex > (Constants::elementsInBucket - 1))
            {
                bitIndex %= Constants::elementsInBucket;
                memPointer++;
                bucketIndex++;
            }
         
            uint8_t mask = (1 << bitIndex);
            value = (mask & vector._buckets[bucketIndex]);
            return toReturn;
        }

        boolean_vector_iterator operator--(int)
        {
            boolean_vector_iterator toReturn(*this);
            if (bitIndex == 0 && memPointer == vector._buckets)
                throw std::out_of_range("Attempted to reach beyond the beginning of the vector.");

            if (--bitIndex < 0)
            {
                bitIndex = Constants::elementsInBucket - 1;
                memPointer--;
                bucketIndex--;
            }
            uint8_t mask = (1 << bitIndex);
            value = (vector._buckets[bucketIndex] & mask);
            return toReturn;
        }

        bool operator==(const boolean_vector_iterator& other)
        {
            return (this->memPointer == other.memPointer && this->bitIndex == other.bitIndex);
        }

        bool operator!=(const boolean_vector_iterator& other)
        {
            return (this->memPointer != other.memPointer || this->bitIndex != other.bitIndex);
        }
    };

        class reverse_vector_iterator
        {
        private:
            friend class BooleanVector;
        private:
            uint8_t* memPointer;
            size_t bitIndex; // [0 .. 7]
            size_t bucketIndex;
            bool value;
            const BooleanVector& vector;
        private:
            reverse_vector_iterator(BooleanVector& vec, size_t push) : vector(vec)
            {
                bitIndex = push;
                if (push == -1)
                {
                    bucketIndex = 0;
                    memPointer = vec._buckets;
                    value = 0;
                }
                else
                {
                    bucketIndex = vec.getBucketIndex(bitIndex);
                    bitIndex %= Constants::elementsInBucket;
                    memPointer = vec._buckets + bucketIndex;
                    value = 0;
                }
            };
        public:
            reverse_vector_iterator(BooleanVector& vec, uint8_t* passedVal) : vector(vec), memPointer{ passedVal } 
            {
                if (memPointer == vec._buckets)
                {
                    bitIndex = 0;
                    bucketIndex = 0;

                    uint8_t mask = (1 << bitIndex);
                    value = (mask & vec._buckets[bucketIndex]);
                }
                else
                {
                    bitIndex = vec.getBitIndex(vec._size - 1);
                    bucketIndex = vec.getBucketIndex(vec._size - 1);

                    uint8_t mask = (1 << bitIndex);
                    value = (mask & vec._buckets[bucketIndex]);
                }
            };

            reverse_vector_iterator& operator++()
            {
                if (bitIndex == 0 && memPointer == vector._buckets)
                    throw std::out_of_range("Attempted to reach beyond the beginning of the vector.");

                if (--bitIndex < 0)
                {
                    bitIndex = Constants::elementsInBucket - 1;
                    memPointer--;
                    bucketIndex--;
                }
                uint8_t mask = (1 << bitIndex);
                value = (vector._buckets[bucketIndex] & mask);
                return *this;
            }
    
            reverse_vector_iterator operator++(int)
            {
                reverse_vector_iterator toReturn(*this);
                if (bitIndex == 0 && bucketIndex == 0)
                    throw std::out_of_range("Attempted to reach beyond the beginning of the vector.");

                if (bitIndex == 0)
                {
                    bitIndex = Constants::elementsInBucket - 1;
                    memPointer--;
                    bucketIndex--;
                }
                else
                {
                    bitIndex--;
                }
                uint8_t mask = (1 << bitIndex);
                value = (vector._buckets[bucketIndex] & mask);
                return toReturn;
            }
    
            reverse_vector_iterator& operator--()
            {
                if (++bitIndex > (Constants::elementsInBucket - 1) && memPointer == vector._buckets + vector._size - 1)
                    throw std::out_of_range("Attempted to reach beyond the beginning of the vector.");

                if (bitIndex > (Constants::elementsInBucket - 1))
                {
                    bitIndex %= Constants::elementsInBucket;
                    memPointer++;
                    bucketIndex++;
                }
                uint8_t mask = (1 << bitIndex);
                value = (mask & vector._buckets[bucketIndex]);
                return *this;
            }
    
            reverse_vector_iterator& operator--(int)
            {
                reverse_vector_iterator toReturn(*this);
                bitIndex++;
                if ((bucketIndex * Constants::elementsInBucket + bitIndex) >= vector._size) 
                    throw std::out_of_range("Reaching beyond the vector's size");

                if (bitIndex > (Constants::elementsInBucket - 1))
                {
                    bitIndex %= Constants::elementsInBucket;
                    memPointer++;
                    bucketIndex++;
                }

                uint8_t mask = (1 << bitIndex);
                value = (mask & vector._buckets[bucketIndex]);
                return toReturn;
            }
    
            bool operator*() const
            {
                return value;
            }
    
            bool operator==(const reverse_vector_iterator& other) const
            {
                return (this->memPointer == other.memPointer && this->bitIndex == other.bitIndex);
            }
    
            bool operator!=(const reverse_vector_iterator& other) const
            {
                return (this->memPointer != other.memPointer || this->bitIndex != other.bitIndex);
            }
        };

        BooleanVector::boolean_vector_iterator begin()
        {
            size_t push = 0;;
            return boolean_vector_iterator(*this, push);
        }
    
        BooleanVector::boolean_vector_iterator end()
        {
            return boolean_vector_iterator(*this, _size);
        }

        BooleanVector::const_boolean_vector_iterator c_begin() const
        {
            return boolean_vector_iterator(*this, _buckets);
        }
    
        BooleanVector::const_boolean_vector_iterator c_end() const
        {
            return boolean_vector_iterator(*this, _buckets, _size);
        }
    
        BooleanVector::reverse_vector_iterator rbegin()
        {
            return reverse_vector_iterator(*this, _size - 1);
        }

        BooleanVector::reverse_vector_iterator rend()
        {
            return reverse_vector_iterator(*this, -1);
        }
    
        void insert(boolean_vector_iterator& iter, bool value); 
        void remove(boolean_vector_iterator& iter); 
    };

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::copy(const BooleanVector& other)
    {
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_bucketsCount = other._bucketsCount;

        _buckets = allocator.allocate(_bucketsCount);
        for (int i = 0; i < _bucketsCount; i++)
            push_back(other[i]);
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::move(BooleanVector&& other)
    {
        this->_buckets = other._buckets;
        other._buckets = nullptr;

        this->_size = other._size;
        this->_bucketsCount = other._bucketsCount;

        this->_capacity = other._capacity;
        other._size = other._bucketsCount = other._capacity = 0;
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::free()
    {
        if (_buckets) {
            allocator.deallocate(_buckets, _bucketsCount);
            _buckets = nullptr;
        }
        _size = _bucketsCount = 0;
    }

    template<class AllocatorType>
    unsigned BooleanVector<AllocatorType>::getBucketIndex(unsigned value) const
    {
        return value / Constants::elementsInBucket;
    }

    template<class AllocatorType>
    unsigned BooleanVector<AllocatorType>::getBitIndex(unsigned value) const
    {
        return value % Constants::elementsInBucket;
    }

    template<class AllocatorType>
    size_t BooleanVector<AllocatorType>::calculate_capacity() const
    {
        if (capacity() == 0)
            return 1;
        return capacity() * Constants::GROWTH_FACTOR;
    }

    template<class AllocatorType>
    BooleanVector<AllocatorType>::BooleanVector(size_t count)
        : _size(0),
        _bucketsCount(count / Constants::elementsInBucket + 1),
        _capacity(_bucketsCount* Constants::elementsInBucket)
    {
        _buckets = allocator.allocate(_bucketsCount);

        for (int i = 0; i < _bucketsCount; i++)
            _buckets[i] = 0;
    }

    template<class AllocatorType>
    BooleanVector<AllocatorType>::BooleanVector(const BooleanVector& other)
    {
        copy(other);
    }

    template<class AllocatorType>
    BooleanVector<AllocatorType>& BooleanVector<AllocatorType>::operator=(const BooleanVector& other)
    {
        if (this != &other)
        {
            free();
            copy(other);
        }
        return *this;
    }

    template<class AllocatorType>
    BooleanVector<AllocatorType>::BooleanVector(BooleanVector&& other)
    {
        move(std::move(other));
    }

    template<class AllocatorType>
    BooleanVector<AllocatorType>& BooleanVector<AllocatorType>::operator=(BooleanVector&& other)
    {
        if (this != &other)
        {
            free();
            move(std::move(other));
        }
        return *this;
    }

    template<class AllocatorType>
    BooleanVector<AllocatorType>::~BooleanVector()
    {
        free();
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::push_back(bool value)
    {
        //проверка дали е заделена памет за този бъкет
        if (_size == _capacity)
            resize(calculate_capacity());

        unsigned firstFreeIndex = getBitIndex(_size);
        unsigned bucketToPut = getBucketIndex(_size);
        uint8_t mask;
        if (value)
        {
            mask = (1 << firstFreeIndex);
            _buckets[bucketToPut] |= mask;
        }
        _size++;
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::pop_back()
    {
        if (_size == 0)
            throw std::out_of_range("The vector is empty!");

        if (contains(_size - 1))
        {
            unsigned lastIndex = ((_size - 1) % Constants::elementsInBucket);
            uint8_t mask = ~(1 << lastIndex);

            _buckets[lastIndex] &= mask;
        }
        _size--;
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::pop_front()
    {
        if (_size == 0)
            throw std::out_of_range("The vector is empty!");

        uint8_t mask;
        for (size_t i = 1; i < _size; i++)
        {
            unsigned bucketIndexToPut = getBucketIndex(i - 1);
            unsigned bitIndexToPut = getBitIndex(i - 1);
            if (contains(i))
            {
                mask = (1 << bitIndexToPut);
                _buckets[bucketIndexToPut] |= mask;
            }
            else
            {
                mask = ~(1 << bitIndexToPut);
                _buckets[bucketIndexToPut] &= mask;
            }
        }
        _size--;
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::print() const
    {
        for (int i = 0; i < _size; i++) {
            if (contains(i))
                std::cout << "true" << ' ';
            else
                std::cout << "false" << ' ';
        }
        std::cout << std::endl;
    }

    template<class AllocatorType>
    bool BooleanVector<AllocatorType>::operator[](size_t index) 
    {
        size_t bucketIndex = getBucketIndex(index);
        size_t bitIndex = getBitIndex(index);

        uint8_t mask = (1 << bitIndex);
        return _buckets[bucketIndex] & mask;
    }

    template<class AllocatorType>
    bool BooleanVector<AllocatorType>::operator[](size_t index) const
    {
        size_t bucketIndex = getBucketIndex(index);
        size_t bitIndex = getBitIndex(index);

        uint8_t mask = (1 << bitIndex);
        return _buckets[bucketIndex] & mask;
    }

    template<class AllocatorType>
    size_t BooleanVector<AllocatorType>::size() const
    {
        return _size;
    }

    template<class AllocatorType>
    size_t BooleanVector<AllocatorType>::capacity() const
    {
        return _capacity;
    }

    template<class AllocatorType>
    bool BooleanVector<AllocatorType>::empty() const
    {
        return (_size == 0);
    }

    template<class AllocatorType>
    bool BooleanVector<AllocatorType>::contains(unsigned value) const
    {
        unsigned bucketIndex = getBucketIndex(value);
        unsigned bitIndex = getBitIndex(value);

        uint8_t mask = (1 << bitIndex);
        return (_buckets[bucketIndex] & mask);
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::insert(boolean_vector_iterator& iter, bool val) 
    {
        if (iter.bitIndex > _size % Constants::elementsInBucket)// && iter == end())
            throw std::out_of_range("Reaching outside the vector's size");

        uint8_t mask;
        //НО ТУК begin и end съвпадат и затова крашва и при end()
        if (_size == 0 && iter == begin()) 
            throw std::out_of_range("Reaching outside the vector's size");

        if (_size == 0)
        {
            if (val)
            {
                mask = (1 << 0);
                _buckets[0] |= mask;
            }
            else
            {
                mask = ~(1 << 0);
                _buckets[0] &= mask;
            }
            iter.value = val;
            _size++;
            return;
        }

        for (int i = _size - 1; i >= iter.bitIndex; i--)
        {
            if (i < 0)
                break;
            unsigned bucketIndexToPut = getBucketIndex(i + 1);
            unsigned bitIndexToPut = getBitIndex(i + 1);
            if (contains(i))
            {
                if ((i + 1) % Constants::elementsInBucket == 0 && _size == _capacity)
                    resize(calculate_capacity());

                mask = (1 << bitIndexToPut);
                _buckets[bucketIndexToPut] |= mask;
            }
            else
            {
                if ((i + 1) % Constants::elementsInBucket == 0 && _size == _capacity)
                    resize(calculate_capacity());

                mask = ~(1 << bitIndexToPut);
                _buckets[bucketIndexToPut] &= mask;
            }
        }

        if (val)
        {
            mask = (1 << iter.bitIndex);
            _buckets[iter.bucketIndex] |= mask;
        }
        else
        {
            mask = ~(1 << iter.bitIndex);
            _buckets[iter.bucketIndex] &= mask;
        }
        iter.value = val;
        _size++;
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::remove(boolean_vector_iterator& iter) 
    {
        if (_size == 0)
            throw std::out_of_range("The vector has no elements!");

        uint8_t mask;
        for (size_t i = iter.bitIndex; i < _size; i++)
        {
            unsigned bucketIndexToPut = getBucketIndex(i);
            unsigned bitIndexToPut = getBitIndex(i);
            if (contains(i + 1))
            {
                mask = (1 << bitIndexToPut);
                _buckets[bucketIndexToPut] |= mask;
            }
            else
            {
                mask = ~(1 << bitIndexToPut);
                _buckets[bucketIndexToPut] &= mask;
            }
        }
        _size--;
    }

    template<class AllocatorType>
    void BooleanVector<AllocatorType>::resize(size_t n) 
    {
        if (n < _size)
        {
            for (size_t i = n; i < _size; i++)
            {
                if (contains(i))
                {
                    unsigned bucketIndex = getBucketIndex(i);
                    unsigned bitIndex = getBitIndex(i);

                    uint8_t mask = ~(1 << bitIndex);
                    _buckets[bucketIndex] &= mask;
                }
            }
            _size = n;
        }
        else if (n > _size && n > _capacity)
        {
            size_t newBucketsCount = n / Constants::elementsInBucket;
            uint8_t* new_data = allocator.allocate(newBucketsCount);
            for (size_t i = 0; i < _bucketsCount; i++)
                new_data[i] = _buckets[i];

            for (size_t i = _bucketsCount; i < newBucketsCount; i++)
                new_data[i] = 0;

            allocator.deallocate(_buckets, _bucketsCount);
            _buckets = new_data;
            _capacity = n;
            _bucketsCount = newBucketsCount;
        }
    }