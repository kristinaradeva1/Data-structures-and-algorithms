#include "TabHandler.h"
#include <iostream>

void TabHandler::copyFrom(const TabHandler& other) 
{
	Tab* otherIter = other.head;
	while (otherIter != nullptr)
	{
		insert(otherIter->URL);
		otherIter = otherIter->next;
	}
}

void TabHandler::moveFrom(TabHandler&& other)
{
	this->head = other.head;
	this->tail = other.tail;
	this->count = other.count;
	this->current = other.current;
	other.head = other.tail = other.current = nullptr;
	other.count = 0;
}

void TabHandler::free()
{
	Tab* iter = head;
	while(iter != nullptr)
	{
		Tab* toDelete = iter;
		iter = iter->next;
		delete toDelete;
	}
	head = tail = current = nullptr;
	count = 0;
}

TabHandler::TabHandler() : count(1)
{
	head = tail = current = new Tab("about:blank");
}

TabHandler::TabHandler(const TabHandler& other)
{
	copyFrom(other);
}

TabHandler& TabHandler::operator=(const TabHandler& other)
{
	if (this != &other)
	{
		free();
		copyFrom(other);
	}
	return *this;
}

TabHandler::TabHandler(TabHandler&& other) noexcept
{
	moveFrom(std::move(other));
}

TabHandler& TabHandler::operator=(TabHandler&& other) noexcept
{
	if (this != &other)
	{
		free();
		moveFrom(std::move(other));
	}
	return *this;
}

TabHandler::~TabHandler()
{
	free();
}

void TabHandler::go(const std::string& URL)
{
	current->URL = URL;
	current->unix_timestamp = std::time(nullptr);
}

void TabHandler::insert(const std::string& URL)
{
	if (count == 1)
	{
		Tab* toAdd = new Tab(URL);
		current->next = toAdd;
		toAdd->prev = current;
		current = tail = current->next;
	}
	else if (current == tail)
	{
		Tab* toAdd = new Tab(URL);
		current->next = toAdd;
		toAdd->prev = current;
		current = tail = current->next;
	}
	else
	{
		Tab* toAdd = new Tab(URL);
		Tab* currentCopy = current;

		toAdd->next = current->next;
		current->next = toAdd;
		current->next->prev = toAdd;
		toAdd->prev = currentCopy;
		current = toAdd;
	}
	count++;
}

void TabHandler::back()
{
	if (count == 0 || current == head || count == 1)
		return;

	current = current->prev;
}

void TabHandler::forward()
{
	if (count == 0 || current == tail)
		return;

	current = current->next;
}

void TabHandler::remove()
{
	if (count == 0)
		return; 

	if (count == 1)
	{
		head = tail = current;
		current->URL = "about:blank";
		current->unix_timestamp = std::time(nullptr);
		return;
	}

	if (current == head)
	{
		current = current->next;

		delete head;
		head = current;
		current->prev = nullptr;
	}
	else if (current == tail)
	{
		current->prev->next = nullptr;
		current = current->prev;
		delete tail;
		tail = current;
	}
	else
	{
		Tab* toRemove = current;
		current->prev->next = current->next;
		current->next->prev = current->prev;
		current = current->next;

		delete toRemove;
	}
	count--;
}

void TabHandler::print() const
{
	Tab* iter = head;
	while (iter)
	{
		if (iter == current)
			std::cout << '>';

		std::cout << iter->URL << ' ' 
			<< iter->unix_timestamp
			<< std::endl;

		iter = iter->next;
	}
}