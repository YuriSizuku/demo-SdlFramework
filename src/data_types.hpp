#include<map>
#include<list>
using std::list;
using std::map;
#ifndef _DATA_TYPES_H
#define _DATA_TYPES_H

template<typename T>
class CMapList
{
private:
	map<int, list<T>> m_maplist; // SDL randering objects
public:
	map<int, list<T>>& get() {return m_maplist;}
	void pushObject(T object, int type = 0)
	{
		m_maplist[type].push_back(object);
	}
	T popObject(int type = 0)
	{
		auto p = *m_maplist[type].rbegin();
		m_maplist[type].pop_back();
		return p;
	}
	bool insertObject(int i, T object, int type = 0)
	{
		if (m_maplist.find(type) == m_maplist.end()) return false;
		if (i == -1)
		{
			m_maplist[type].push_back(object);
			return true;
		}
		if (static_cast<size_t>(i) < m_maplist[type].size())
		{
			auto it = atObjectIter(i, type);
			if (it == m_maplist[type].end()) return false;
			m_maplist[type].insert(it, object);
			return true;
		}
		return false;
	}
	typename list<T>::iterator atObjectIter(int i, int type = 0)
	{
		if (m_maplist.find(type) == m_maplist.end())
		{
			// if not exist, create one
			list<T> t;
			m_maplist[type] = t;
			return t.end();
		}

		int j = 0;
		auto  it = m_maplist[type].begin();
		for (; it != m_maplist[type].end(); it++)
		{
			if (j == i) return it;
			if (j > i) break;
			j++;
		}
		return it;
	}
	T atObject(int i, int type = 0)
	{
		auto it = atObjectIter(i, type);
		if (it == m_maplist[type].end()) return NULL;
		return *it;
	}
	T removeObject(int i, int type = 0)
	{
		auto it = atObjectIter(i, type);
		if (it == m_maplist[type].end()) return NULL;
		return removeObject(it, type);
	}
	T removeObject(typename list<T>::iterator& it, int type = 0)
	{
		auto p = *it;
		it = m_maplist[type].erase(it);
		return p;
	}
	bool releaseObject(int i, int type = 0)
	{
		auto it = atObjectIter(i, type);
		if (it == m_maplist[type].end()) return false;
		if (removeObject(it, type)) return true;
		else return false;
	}
	bool releaseObject(typename list<T>::iterator& it, int type = 0)
	{
		auto p = removeObject(it, type);
		if (p == NULL) return false;
		delete p;
		return true;
	}
	void releaseAllObjects()
	{
		for (auto it = m_maplist.begin(); it != m_maplist.end(); it++)
		{
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				delete* it2;
			}
			(*it).second.clear();
		}
		m_maplist.clear();
	}
};

template<typename T>
class CScene
{
protected:
	// scene status, check if the scene should exit or hang on
	int m_type=0, m_id=0, m_status=0;
	uint32_t m_lastUpdateTicks = 0;
	T m_pObjects;
public:
	T& getpObjects() { return m_pObjects; }
	virtual void handleEvent(void* event) {};
	virtual void update(uint32_t currentTicks) { m_lastUpdateTicks = currentTicks; }
	virtual void render()=0;
};

#endif