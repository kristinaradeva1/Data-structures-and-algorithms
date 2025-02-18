#pragma once
#include <vector>
#include <forward_list>

using namespace std;
template<typename Key, typename Hash = std::hash<Key>>
class UnorderedSet
{
private:
	vector<forward_list<Key>> hashTable;
	double maxLoadFactor = 0.75;
	Hash getHash;

	void resize();
	size_t getHashCode(const Key& key) const;

	int getElementsCount() const;
public:
	class ConstIterator
	{
	private:
		const UnorderedSet<Key, Hash>& set;

		typename forward_list<Key>::const_iterator currElementIter;
		int bucketIndex;
		ConstIterator(const UnorderedSet<Key, Hash>& _set, typename forward_list<Key>::const_iterator curr);
		friend class UnorderedSet;

		bool isLastElementInBucket() const;
		bool isLastElementInHashTable() const;
		bool isFirstElementInHashTable() const;
		bool isFirstElementInBucket() const;

		bool emptyBucket(size_t bucketIndex) const;
		int lastNonEmptyBucketIndex() const;

		bool isLastElement(const Key& key) const;
	public:
		const Key& operator*() const;

		ConstIterator operator+(int off) const;
		ConstIterator operator-(int off) const;

		ConstIterator& operator++(); //++it
		ConstIterator operator++(int); //it++

		ConstIterator& operator--(); //--it
		ConstIterator operator--(int); //it--

		//const Key* operator->() const;

		bool operator==(const ConstIterator& other) const;
		bool operator!=(const ConstIterator& other) const;
	};

	class Iterator 
	{
	private:
		const UnorderedSet<Key, Hash>& set;

		typename forward_list<Key>::iterator currElementIter;
		size_t bucketIndex;
		Iterator(const UnorderedSet& set, typename forward_list<Key>::iterator curr);
		friend class UnorderedSet;

		bool isLastElementInBucket() const;
		bool isLastElementInHashTable() const;
		bool isFirstElementInHashTable() const;
		bool isFirstElementInBucket() const;

		bool emptyBucket(size_t bucketIndex) const;
		int lastNonEmptyBucketIndex() const;

		bool isLastElement(const Key& key) const;
	public:
		Key& operator*() const;

		Iterator operator+(int off) const;
		Iterator operator-(int off) const;

		Iterator& operator--(); //--it
		Iterator operator--(int); //it--

		Iterator& operator++(); //++it
		Iterator operator++(int); //it++

		//const Key* operator->() const;
		//Key* operator->();

		bool operator==(const Iterator& other) const;
		bool operator!=(const Iterator& other) const;
	};

	UnorderedSet();

	void insert(const Key& key);

	void remove(const Key& key);
	void remove(ConstIterator iter);

	ConstIterator find(const Key& key) const;

	void clearSet();
	bool empty() const;

	template<typename Predicate>
	void erase_if(const Predicate& pred);

	void print() const;

	ConstIterator cbegin() const;
	ConstIterator cend() const;

	Iterator begin();
	Iterator end();

	double loadFactor() const;
};

template<typename Key, typename Hash>
size_t UnorderedSet<Key, Hash>::getHashCode(const Key& key) const
{
	return getHash(key) % hashTable.size();
}

template<typename Key, typename Hash>
void UnorderedSet<Key, Hash>::resize()
{
	vector<forward_list<Key>> newHashTable(hashTable.size() * 2);

	for (int i = 0; i < hashTable.size(); i++)
	{
		for (auto it = hashTable[i].begin(); it != hashTable[i].end(); it++)
		{
			size_t newHashCode = getHash(*it) % newHashTable.size();
			newHashTable[newHashCode].push_front(*it);
		}
	}
	hashTable = move(newHashTable);
}

template<typename Key, typename Hash>
int UnorderedSet<Key, Hash>::getElementsCount() const
{
	int count = 0;
	for (int i = 0; i < hashTable.size(); i++)
	{
		for (auto it = hashTable[i].begin(); it != hashTable[i].end(); it++)
			count++;
	}
	return count;
}

template<typename Key, typename Hash>
UnorderedSet<Key, Hash>::UnorderedSet()
{
	hashTable.resize(8);
}

template<typename Key, typename Hash>
void UnorderedSet<Key, Hash>::insert(const Key& key)
{
	if (loadFactor() >= maxLoadFactor) 
        resize();

	size_t hashCode = getHashCode(key);
	auto& bucket = hashTable[hashCode];
	for (auto it = bucket.begin(); it != bucket.end(); it++)
	{
		if (*it == key)
			return;
	}

	bucket.push_front(key);
}

template<typename Key, typename Hash>
void UnorderedSet<Key, Hash>::remove(const Key& key)
{
	size_t hashCode = getHashCode(key);
	auto& bucket = hashTable[hashCode];

	auto prev = bucket.before_begin();
	for(auto curr = bucket.begin(); curr != bucket.end(); curr++)
	{
		if (*curr == key)
		{
			bucket.erase_after(prev);
			return;
		}

		prev = curr;
	}
}

template<typename Key, typename Hash>
void UnorderedSet<Key, Hash>::remove(ConstIterator iter)
{
	size_t hashCode = getHashCode(*iter);
	auto& bucket = hashTable[hashCode];

	auto prev = bucket.before_begin();
	for (auto curr = bucket.begin(); curr != bucket.end(); curr++)
	{
		if (*curr == *iter)
		{
			bucket.erase_after(prev);
			return;
		}

		prev = curr;
	}
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator UnorderedSet<Key, Hash>::find(const Key& key) const
{
	size_t hashCode = getHashCode(key);
	for (auto it = hashTable[hashCode].cbegin(); it != hashTable[hashCode].cend(); it++)
	{
		if (*it == key)
			return ConstIterator(it);
	}
	return cend();
}

template<typename Key, typename Hash>
void UnorderedSet<Key, Hash>::clearSet()
{
	for (int i = 0; i < hashTable.size(); i++)
		hashTable[i].clear();

	hashTable.resize(8);
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::empty() const
{
	for (const auto& bucket : hashTable)
	{
		if (!bucket.empty())
			return false;
	}
	return true;
}

template<typename Key, typename Hash>
template<typename Predicate>
void UnorderedSet<Key, Hash>::erase_if(const Predicate& pred)
{
	for (int i = 0; i < hashTable.size(); i++)
	{
		for (auto it = hashTable[i].begin(); it != hashTable[i].end(); it++)
		{
			if (pred(*it))
			{
				ConstIterator iter(it);
				remove(iter);
			}
		}
	}
}

template<typename Key, typename Hash>
void UnorderedSet<Key, Hash>::print() const
{
	for (int i = 0; i < hashTable.size(); i++) {
		for (auto it = hashTable[i].begin(); it != hashTable[i].end(); it++)
			cout << *it << ' ';

		if (hashTable[i].empty())
			continue;

		cout << endl;
	}
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator UnorderedSet<Key, Hash>::cbegin() const
{
	for (size_t i = 0; i < hashTable.size(); i++) 
	{
		if (!hashTable[i].empty()) 
		{
			return ConstIterator(*this, hashTable[i].cbegin());
		}
	}
	return cend();
}
	
template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator UnorderedSet<Key, Hash>::cend() const
{
	size_t lastNonEmptyIdx;
	for (int i = hashTable.size() - 1; i >= 0; i--)
	{
		if (!hashTable[i].empty())
		{
			lastNonEmptyIdx = i;
			break;
		}
	}
	auto& bucket = hashTable[lastNonEmptyIdx];
	typename std::forward_list<Key>::const_iterator lastNonEmptyBucketIter = bucket.cbegin();
	for (auto it = bucket.begin(); it != bucket.end(); it++)
	{
		typename forward_list<Key>::const_iterator iter = it;
		iter++;
		if (iter == bucket.end())
			break;
		lastNonEmptyBucketIter++;
	}
	return ConstIterator(*this, lastNonEmptyBucketIter);
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::Iterator UnorderedSet<Key, Hash>::begin()
{
	for (auto& bucket : hashTable) 
	{
		if (!bucket.empty())
		{
			return Iterator(bucket.begin());
		}
	}
	return end();
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::Iterator UnorderedSet<Key, Hash>::end()
{
	return Iterator(hashTable.back().end());
}

template<typename Key, typename Hash>
double UnorderedSet<Key, Hash>::loadFactor() const
{
	int elementsCount = getElementsCount();
	return static_cast<double>(elementsCount) / hashTable.size();
}

///////////////////////////////////////////////////////////////////////////////
template<typename Key, typename Hash>
UnorderedSet<Key, Hash>::ConstIterator::ConstIterator(const UnorderedSet& _set, typename forward_list<Key>::const_iterator curr) : set(_set)
{
	if (isLastElement(*curr))
	{
		size_t bucketIdx = set.getHashCode(*curr);
		currElementIter = set.hashTable[bucketIdx].cend();
		bucketIndex = -1;
	}
	else
	{
		currElementIter = curr;
		bucketIndex = set.getHashCode(*curr);
	}
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::ConstIterator::isLastElementInBucket() const
{
	auto& bucket = set.hashTable[bucketIndex];

	for (auto it = bucket.cbegin(); it != bucket.cend(); it++)
	{
		if (*it == *currElementIter && (++it) == bucket.cend())
			return true;
	}
	return false;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::ConstIterator::isLastElementInHashTable() const
{
	if (!isLastElementInBucket())
		return false;

	for (int i = bucketIndex + 1; i < set.hashTable.size(); i++)
	{
		if (!set.hashTable[i].empty())
			return false;
	}
	return true;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::ConstIterator::isFirstElementInBucket() const
{
	return currElementIter == set.hashTable[bucketIndex].cbegin();
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::ConstIterator::isFirstElementInHashTable() const
{
	if (!isFirstElementInBucket())
		return false;

	for (int i = 0; i < bucketIndex; i++)
	{
		if (!set.hashTable[i].empty())
			return false;
	}
	return true;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::ConstIterator::emptyBucket(size_t bucketIndex) const
{
	return set.hashTable[bucketIndex].empty();
}

template<typename Key, typename Hash>
int UnorderedSet<Key, Hash>::ConstIterator::lastNonEmptyBucketIndex() const
{
	for (int i = set.hashTable.size() - 1; i >= 0; i--)
	{
		if (!set.hashTable[i].empty())
			return i;
	}
	return -1;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::ConstIterator::isLastElement(const Key& key) const
{
	size_t lastNonEmptyIdx = lastNonEmptyBucketIndex();
	size_t keyIdx = set.getHashCode(key);
	if (lastNonEmptyIdx != keyIdx)
		return false;

	auto& bucket = set.hashTable[lastNonEmptyIdx];
	for (auto it = bucket.begin(); it != bucket.end(); it++)
	{
		typename forward_list<Key>::const_iterator iter = it;
		iter++;
		if (iter == bucket.end() && *it == key)
			return true;
	}
	return false;
}

template<typename Key, typename Hash>
const Key& UnorderedSet<Key, Hash>::ConstIterator::operator*() const
{
	return *currElementIter;
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator UnorderedSet<Key, Hash>::ConstIterator::operator+(int off) const
{
	while (off != 0)
	{
		++this;
		off--;
	}
	return ConstIterator(*this);
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator UnorderedSet<Key, Hash>::ConstIterator::operator-(int off) const
{
	while (off != 0)
	{
		--this;
		off--;
	}
	return ConstIterator(*this);
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator& UnorderedSet<Key, Hash>::ConstIterator::operator++()
{
	if (isLastElementInHashTable())
		return *this;

	if (isLastElementInBucket())
	{
		bucketIndex++;
		while (emptyBucket(bucketIndex))
		{
			bucketIndex++;
		}
		currElementIter = set.hashTable[bucketIndex].cbegin();
	}
	else
	{
		currElementIter++;
	}
	return *this;
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator UnorderedSet<Key, Hash>::ConstIterator::operator++(int)
{
	ConstIterator temp = *this;
	++(*this);
	return temp;
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator& UnorderedSet<Key, Hash>::ConstIterator::operator--()
{
	if (*this == set.cend())
	{
		size_t lastBucketIdx = lastNonEmptyBucketIndex();
		auto& bucket = set.hashTable[lastBucketIdx];
		for (auto it = bucket.cbegin(); it != bucket.cend(); it++)
		{
			currElementIter = it;
		}
		bucketIndex = lastBucketIdx;
		return *this;
	}

	if (isFirstElementInHashTable())
		return *this;

	if (isFirstElementInBucket())
	{
		bucketIndex--;
		auto& bucket = set.hashTable[bucketIndex];

		auto iter = bucket.cbegin();
		for (auto it = bucket.cbegin(); it != bucket.cend(); it++)
			iter = it;

		currElementIter = iter;
	}
	else
	{
		auto& bucket = set.hashTable[bucketIndex];
		auto prev = bucket.before_begin();
		for (auto it = bucket.begin(); it != bucket.end(); it++)
		{
			if (it == currElementIter)
			{
				currElementIter = prev;
				break;
			}

			prev = it;
		}
	}
	return *this;
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::ConstIterator UnorderedSet<Key, Hash>::ConstIterator::operator--(int)
{
	ConstIterator temp = *this; 
	--(*this);               
	return temp;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::ConstIterator::operator==(const ConstIterator& other) const
{
	return currElementIter == other.currElementIter && bucketIndex == other.bucketIndex;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::ConstIterator::operator!=(const ConstIterator& other) const
{
	return currElementIter != other.currElementIter && bucketIndex == other.bucketIndex;
}

////////////////////////////////////////////////////////////////////////
template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::Iterator::isLastElementInBucket() const
{
	auto& bucket = set.hashTable[bucketIndex];

	for (auto it = bucket.begin(); it != bucket.end(); it++)
	{
		if (*it == *currElementIter && (++it) == bucket.end())
			return true;
	}
	return false;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::Iterator::isLastElementInHashTable() const
{
	if (!isLastElementInBucket())
		return false;

	for (int i = bucketIndex + 1; i < set.hashTable.size(); i++)
	{
		if (!set.hashTable[i].empty())
			return false;
	}
	return true;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::Iterator::isFirstElementInHashTable() const
{
	if (!isFirstElementInBucket())
		return false;

	for (int i = 0; i < bucketIndex; i++)
	{
		if (!set.hashTable[i].empty())
			return false;
	}
	return true;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::Iterator::isFirstElementInBucket() const
{
	return currElementIter == set.hashTable[bucketIndex].begin();
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::Iterator::emptyBucket(size_t bucketIndex) const
{
	return set.hashTable[bucketIndex].empty();
}

template<typename Key, typename Hash>
int UnorderedSet<Key, Hash>::Iterator::lastNonEmptyBucketIndex() const
{
	for (int i = set.hashTable.size() - 1; i >= 0; i--)
	{
		if (!set.hashTable[i].empty())
			return i;
	}
	return -1;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::Iterator::isLastElement(const Key& key) const
{
	size_t lastNonEmptyIdx = lastNonEmptyBucketIndex();
	size_t keyIdx = set.getHashCode(key);
	if (lastNonEmptyIdx != keyIdx)
		return false;

	auto& bucket = set.hashTable[lastNonEmptyIdx];
	for (auto it = bucket.begin(); it != bucket.end(); it++)
	{
		typename forward_list<Key>::iterator iter = it;
		iter++;
		if (iter == bucket.end() && *it == key)
			return true;
	}
	return false;
}

template<typename Key, typename Hash>
UnorderedSet<Key, Hash>::Iterator::Iterator(const UnorderedSet& _set, typename forward_list<Key>::iterator curr) : set(_set)
{
	if (isLastElement(*curr))
	{
		size_t bucketIdx = set.getHashCode(*curr);
		currElementIter = set.hashTable[bucketIdx].cend();
		bucketIndex = -1;
	}
	else
	{
		currElementIter = curr;
		bucketIndex = set.getHashCode(*curr);
	}
}

template<typename Key, typename Hash>
Key& UnorderedSet<Key, Hash>::Iterator::operator*() const
{
	return *currElementIter;
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::Iterator UnorderedSet<Key, Hash>::Iterator::operator+(int off) const
{
	while (off != 0)
	{
		++this;
		off--;
	}
	return Iterator(*this);
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::Iterator UnorderedSet<Key, Hash>::Iterator::operator-(int off) const
{
	while (off != 0)
	{
		--this;
		off--;
	}
	return Iterator(*this);
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::Iterator& UnorderedSet<Key, Hash>::Iterator::operator--()
{
	if (*this == set.cend())
	{
		size_t lastBucketIdx = lastNonEmptyBucketIndex();
		auto& bucket = set.hashTable[lastBucketIdx];
		for (auto it = bucket.begin(); it != bucket.end(); it++)
		{
			currElementIter = it;
		}
		bucketIndex = lastBucketIdx;
		return *this;
	}

	if (isFirstElementInHashTable())
		return *this;

	if (isFirstElementInBucket())
	{
		bucketIndex--;
		auto& bucket = set.hashTable[bucketIndex];

		auto iter = bucket.begin();
		for (auto it = bucket.begin(); it != bucket.end(); it++)
			iter = it;

		currElementIter = iter;
	}
	else
	{
		auto& bucket = set.hashTable[bucketIndex];
		auto prev = bucket.before_begin();
		for (auto it = bucket.begin(); it != bucket.end(); it++)
		{
			if (it == currElementIter)
			{
				currElementIter = prev;
				break;
			}

			prev = it;
		}
	}
	return *this;
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::Iterator UnorderedSet<Key, Hash>::Iterator::operator--(int)
{
	Iterator temp = *this;
	--(*this);
	return temp;
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::Iterator& UnorderedSet<Key, Hash>::Iterator::operator++()
{
	if (isLastElementInHashTable())
		return *this;

	if (isLastElementInBucket())
	{
		bucketIndex++;
		while (emptyBucket(bucketIndex))
		{
			bucketIndex++;
		}
		currElementIter = set.hashTable[bucketIndex].cbegin();
	}
	else
	{
		currElementIter++;
	}
	return *this;
}

template<typename Key, typename Hash>
typename UnorderedSet<Key, Hash>::Iterator UnorderedSet<Key, Hash>::Iterator::operator++(int)
{
	Iterator temp = *this;
	++(*this);
	return temp;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::Iterator::operator==(const Iterator& other) const
{
	return currElementIter == other.currElementIter && bucketIndex == other.bucketIndex;
}

template<typename Key, typename Hash>
bool UnorderedSet<Key, Hash>::Iterator::operator!=(const Iterator& other) const
{
	return currElementIter != other.currElementIter && bucketIndex == other.bucketIndex;
}