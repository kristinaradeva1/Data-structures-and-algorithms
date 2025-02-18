#pragma once
#include <ctime>
#include <string>
class TabHandler //a doubly-linked list
{
private:
	size_t count = 0;
	struct Tab //a node
	{	
		Tab(const std::string& _URL) : URL(_URL), unix_timestamp(std::time(nullptr)), next(nullptr), prev(nullptr) {};
		std::string URL;
		std::time_t unix_timestamp;
		Tab* next;
		Tab* prev;
	};
	
	Tab* head = nullptr;
	Tab* tail = nullptr;
	Tab* current = nullptr;

	void copyFrom(const TabHandler& other);
	void moveFrom(TabHandler&& other);
	void free();
public:
	TabHandler();
	TabHandler(const TabHandler& other);
	TabHandler& operator=(const TabHandler& other);

	TabHandler(TabHandler&& other) noexcept;
	TabHandler& operator=(TabHandler&& other) noexcept;

	~TabHandler();
	void go(const std::string& URL);
	void insert(const std::string& URL);
	void back();
	void forward();
	void remove();
	void print() const;
};