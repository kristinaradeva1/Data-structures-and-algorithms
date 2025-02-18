#include <iostream>
#include <list>
#include <vector>
#include <forward_list>
#include <algorithm>

template<typename Key, typename Hash = std::hash<Key>>
class InsertionOrderedSet
{
private:
	std::list<Key> data;
	std::vector<std::forward_list<typename std::list<Key>::iterator>> hashTable;
	double maxLoadFactor = 0.75;

	Hash getHash;
	void resize();
	size_t getHashCode(const Key& element) const;
public:
	class Iterator 
	{
	private:
		typename std::list<Key>::iterator currElementIter;
		Iterator(typename std::list<Key>::iterator curr) : currElementIter(curr) {};
		friend class InsertionOrderedSet;
	public:
		Key& operator*() const;

		Iterator operator+(int off) const;
		Iterator operator-(int off) const;

		Iterator& operator--(); //--it
		Iterator operator--(int); //it--

		Iterator& operator++(); //++it
		Iterator operator++(int); //it++

		const Key* operator->() const;
		Key* operator->();

		bool operator==(const Iterator& other) const;
		bool operator!=(const Iterator& other) const;
	};

	class ConstIterator
	{
	private:
		typename std::list<Key>::const_iterator currElementIter;
		ConstIterator(typename std::list<Key>::const_iterator curr) : currElementIter(curr) {};
		friend class InsertionOrderedSet;
	public:
		const Key& operator*() const;

		ConstIterator operator+(int off) const;
		ConstIterator operator-(int off) const;

		ConstIterator& operator++(); //++it
		ConstIterator operator++(int); //it++

		ConstIterator& operator--(); //--it
		ConstIterator operator--(int); //it--

		const Key* operator->() const;

		bool operator==(const ConstIterator& other) const;
		bool operator!=(const ConstIterator& other) const;
	};
	
	InsertionOrderedSet();
	void print() const;
	void insert(const Key& key);
	void remove(const Key& key);
	void remove(Iterator it);
	ConstIterator find(const Key& element) const;
	void clear();
	bool empty() const;

	template<typename Functor>
	void erase_if(Functor predicate);

	Iterator begin();
	Iterator end();
	
	ConstIterator cbegin() const;
	ConstIterator cend() const;

	double loadFactor() const;
};

template<typename Key, typename Hash>
void InsertionOrderedSet<Key, Hash>::resize()
{
	size_t newSize = hashTable.size() * 2;
	std::vector<std::forward_list<typename std::list<Key>::iterator>> newHashTable(newSize);
	for (auto it = data.begin(); it != data.end(); ++it) 
	{
		size_t newHashCode = getHash(*it) % newSize;
		newHashTable[newHashCode].push_front(it);
	}

	hashTable = std::move(newHashTable);
}

template<typename Key, typename Hash>
size_t InsertionOrderedSet<Key, Hash>::getHashCode(const Key& element) const
{
	return getHash(element) % hashTable.size();
}

template<typename Key, typename Hash>
InsertionOrderedSet<Key, Hash>::InsertionOrderedSet()
{
	hashTable.resize(8);
}

template<typename Key, typename Hash>
void InsertionOrderedSet<Key, Hash>::print() const
{
	for (auto it = data.begin(); it != data.end(); it++) 
		std::cout << *it << ' ';

	std::cout << std::endl;
}

template<typename Key, typename Hash>
void InsertionOrderedSet<Key, Hash>::insert(const Key& key) //?
{
	if (loadFactor() >= maxLoadFactor)
		resize();

	size_t hashCode = getHashCode(key);
	auto& bucket = hashTable[hashCode];
	auto iter = std::find_if(bucket.begin(), bucket.end(), [key](auto& iter) { return key == *iter;});

	if (iter != hashTable[hashCode].end())
		return;
	else
	{
		data.emplace_back(key);
		hashTable[hashCode].push_front(--data.end());
	}
}

template<typename Key, typename Hash>
void InsertionOrderedSet<Key, Hash>::remove(const Key& key)
{
	size_t hashCode = getHashCode(key);
	auto& bucket = hashTable[hashCode];
	bucket.remove_if([this, key](const auto& it) {
		if (*it == key) {
			data.erase(it);
			return true;
		}
		else {
			return false;
		}
	});
}

template<typename Key, typename Hash>
void InsertionOrderedSet<Key, Hash>::remove(Iterator it)
{
	const Key& key = *it;
	size_t hashCode = getHashCode(key);
	auto& bucket = hashTable[hashCode];
	bucket.remove_if([&key](const Key& elem) {
		return elem == key;
	});
	data.erase(it);
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::ConstIterator InsertionOrderedSet<Key, Hash>::find(const Key& element) const
{
	size_t hashCode = getHashCode(element);
	auto& bucket = hashTable[hashCode];

	auto it = std::find_if(bucket.begin(), bucket.end(), [&element](auto& iter) { return element == *iter; }); //wrong
	if (it == bucket.end())
		return ConstIterator(data.end());
	else
		return ConstIterator(it);
}

template<typename Key, typename Hash>
void InsertionOrderedSet<Key, Hash>::clear()
{
	data.clear();
	hashTable.clear(); 
	hashTable.resize(8);
}

template<typename Key, typename Hash>
bool InsertionOrderedSet<Key, Hash>::empty() const
{
	return data.size() == 0;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::Iterator InsertionOrderedSet<Key, Hash>::begin()
{
	return Iterator(data.begin());
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::Iterator InsertionOrderedSet<Key, Hash>::end()
{
	return Iterator(data.end());
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::ConstIterator InsertionOrderedSet<Key, Hash>::cbegin() const
{
	InsertionOrderedSet<Key, Hash>::ConstIterator toReturn(data.begin());
	return toReturn;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key,Hash>::ConstIterator InsertionOrderedSet<Key, Hash>::cend() const
{
	InsertionOrderedSet<Key, Hash>::ConstIterator toReturn(data.end());
	return toReturn;
}

template<typename Key, typename Hash>
double InsertionOrderedSet<Key, Hash>::loadFactor() const
{
	return static_cast<double>(data.size()) / hashTable.size();
}

template<typename Key, typename Hash>
template<typename Functor>
void InsertionOrderedSet<Key, Hash>::erase_if(Functor predicate)
{
	for (int i = 0; i < hashTable.size(); i++) 
	{
		auto& curBucket = hashTable[i];
		curBucket.remove_if([&predicate](const Key& elem) { predicate(elem); });
	}
}

template<typename Key, typename Hash>
const Key& InsertionOrderedSet<Key, Hash>::ConstIterator::operator*() const
{
	return *currElementIter;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::ConstIterator InsertionOrderedSet<Key, Hash>::ConstIterator::operator+(int off) const
{
	//проверка дали излизаме извън рамките на data
	ConstIterator res = *this;

	while (off > 0) {
		++res.currElementIter;
		--off;
	}
	while (off < 0) {
		--res.currElementIter;
		++off;
	}

	return res;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::ConstIterator InsertionOrderedSet<Key, Hash>::ConstIterator::operator-(int off) const
{
	ConstIterator res = *this;
	while (off > 0) {
		--res.currElementIter;
		++off;
	}
	while (off < 0) {
		++res.currElementIter;
		--off;
	}

	return res;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key,Hash>::ConstIterator& InsertionOrderedSet<Key, Hash>::ConstIterator::operator++()
{
	//проверка дали излизаме извън рамките на data
	if (currElementIter == data.cend())
		return *this;

	++currElementIter;
	return *this;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key,Hash>::ConstIterator InsertionOrderedSet<Key, Hash>::ConstIterator::operator++(int)
{
	//проверка дали излизаме извън рамките на data
	if (currElementIter == data.cend())
		return *this;

	InsertionOrderedSet<Key, Hash>::ConstIterator temp = *this;
	++(*this);
	return temp;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::ConstIterator& InsertionOrderedSet<Key, Hash>::ConstIterator::operator--()
{
	//проверка дали излизаме извън рамките на data
	if (currElementIter == data.cbegin())
		return *this;

	--currElementIter;
	return *this;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::ConstIterator InsertionOrderedSet<Key, Hash>::ConstIterator::operator--(int)
{
	if (currElementIter == data.cbegin())
		return *this;

	InsertionOrderedSet<Key, Hash>::ConstIterator temp = *this;
	--(*this);
	return temp;
}

template<typename Key, typename Hash>
const Key* InsertionOrderedSet<Key, Hash>::ConstIterator::operator->() const
{
	return &(*currElementIter);
}

template<typename Key, typename Hash>
bool InsertionOrderedSet<Key, Hash>::ConstIterator::operator==(const ConstIterator& other) const
{
	return this->currElementIter == other.currElementIter;
}

template<typename Key, typename Hash>
bool InsertionOrderedSet<Key, Hash>::ConstIterator::operator!=(const ConstIterator& other) const
{
	return !(this->currElementIter == other.currElementIter);
}

template<typename Key, typename Hash>
const Key* InsertionOrderedSet<Key, Hash>::Iterator::operator->() const
{
	return &(*currElementIter);
}

template<typename Key, typename Hash>
Key* InsertionOrderedSet<Key, Hash>::Iterator::operator->()
{
	return &(*currElementIter);
}

template<typename Key, typename Hash>
bool InsertionOrderedSet<Key, Hash>::Iterator::operator==(const Iterator& other) const
{
	return this->currElementIter == other.currElementIter;
}

template<typename Key, typename Hash>
bool InsertionOrderedSet<Key, Hash>::Iterator::operator!=(const Iterator& other) const
{
	return !(this->currElementIter == other.currElementIter);;
}

template<typename Key, typename Hash>
Key& InsertionOrderedSet<Key, Hash>::Iterator::operator*() const
{
	return *currElementIter;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::Iterator InsertionOrderedSet<Key, Hash>::Iterator::operator+(int off) const
{
	//проверка дали излизаме извън рамките на data
	ConstIterator res = *this;

	while (off > 0) {
		++res.currElementIter;
		--off;
	}
	while (off < 0) {
		--res.currElementIter;
		++off;
	}

	return res;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::Iterator InsertionOrderedSet<Key, Hash>::Iterator::operator-(int off) const
{
	ConstIterator res = *this;
	while (off > 0) {
		--res.currElementIter;
		++off;
	}
	while (off < 0) {
		++res.currElementIter;
		--off;
	}

	return res;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::Iterator& InsertionOrderedSet<Key, Hash>::Iterator::operator--()
{
	if (currElementIter == data.begin())
		return *this;

	--currElementIter;
	return *this;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::Iterator InsertionOrderedSet<Key, Hash>::Iterator::operator--(int)
{
	if (currElementIter == data.begin())
		return *this;

	InsertionOrderedSet<Key, Hash>::Iterator temp = *this;
	++(*this);
	return temp;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::Iterator& InsertionOrderedSet<Key, Hash>::Iterator::operator++()
{
	//проверка дали излизаме извън рамките на data
	if (currElementIter == data.end())
		return *this;

	++currElementIter;
	return *this;
}

template<typename Key, typename Hash>
typename InsertionOrderedSet<Key, Hash>::Iterator InsertionOrderedSet<Key, Hash>::Iterator::operator++(int)
{
	//проверка дали излизаме извън рамките на data
	if (currElementIter == data.end())
		return *this;

	InsertionOrderedSet<Key, Hash>::Iterator temp = *this;
	++(*this);
	return temp;
}