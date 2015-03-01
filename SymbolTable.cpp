#include <string>
#include <vector>
#include <iostream>
#include "SymbolTable.hpp"

class SymbolTable::Impl
{
public:
	Impl(int size);
	~Impl();
	bool add(const std::string& name, int type);
	int peek(const std::string& name);

private:
	struct SymbolTableData
	{
		SymbolTableData(const std::string& name, int type);
		~SymbolTableData();
		std::string m_name;
		int m_type;
		SymbolTableData* m_next;
	};
	SymbolTableData** m_symTab;
	size_t m_size;
	size_t m_filled;

	int hash(const std::string& name);
	void growTable();
};

SymbolTable::Impl::SymbolTableData::SymbolTableData(const std::string& name, int type)
	: m_name(name), m_type(type), m_next(NULL) {}

SymbolTable::Impl::SymbolTableData::~SymbolTableData()
{
	delete m_next;
}

SymbolTable::Impl::Impl(int size)
{
	m_size = size;
	m_symTab = new SymbolTableData*[m_size];
	m_filled = 0;
	for(size_t i=0; i<m_size; ++i){
		m_symTab[i] = NULL;
	}
}

SymbolTable::Impl::~Impl()
{
	for(size_t i=0; i<m_size; ++i){
		delete m_symTab[i];
	}
	delete[] m_symTab;
}

int SymbolTable::Impl::hash(const std::string& name)
{
	//djb2 algorithm
	//http://www.cse.yorku.ca/~oz/hash.html
	unsigned int hash = 5381;
	for(std::string::const_iterator it = name.begin(); it != name.end(); ++it){
		hash = ((hash << 5) + hash) + *it;
	}
	return hash % m_size;
}

void SymbolTable::Impl::growTable()
{
	size_t oldSize = m_size;
	m_size *= 2;
	m_filled = 0;
	SymbolTableData** oldSymTab = m_symTab;
	m_symTab = new SymbolTableData*[m_size];

	for(size_t i=0; i<oldSize; ++i){
		if(!oldSymTab[i]){
			continue;
		}
		SymbolTableData* currSym = oldSymTab[i];
		add(currSym->m_name, currSym->m_type);
		while((currSym = currSym->m_next)){
			add(currSym->m_name, currSym->m_type);
		}
		delete oldSymTab[i];
	}
	delete[] oldSymTab;
	std::cout << "I ran successfully!" << std::endl;
}

bool SymbolTable::Impl::add(const std::string& name, int type)
{
	if(m_filled >= m_size*2){
		growTable();
	}

	int key = hash(name);
	if(!m_symTab[key]){
		m_symTab[key] = new SymbolTableData(name, type);
	}
	else{
		SymbolTableData* currSym = m_symTab[key];
		if(!currSym->m_name.compare(name)){
			return false;
		}
		while(currSym->m_next){
			currSym = currSym->m_next;
			if(!currSym->m_name.compare(name)){
				return false;
			}
		}
		currSym->m_next = new SymbolTableData(name, type);
	}
	m_filled++;
	return true;
}

int SymbolTable::Impl::peek(const std::string& name)
{
	for(size_t i=0; i<m_size; ++i){
		if(!m_symTab[i]){
			continue;
		}

		SymbolTableData* currSym = m_symTab[i];
		if(!name.compare(currSym->m_name)){
			return currSym->m_type;
		}
		while((currSym = currSym->m_next)){
			if(!name.compare(currSym->m_name)){
				return currSym->m_type;
			}
		}
	}
	return NOT_FOUND;
}

SymbolTable::SymbolTable(int size) : pimpl(new Impl(size)) {}

SymbolTable::~SymbolTable()
{
	delete pimpl;
}

bool SymbolTable::add(const std::string& name, int type)
{
	return pimpl->add(name, type);
}

int SymbolTable::peek(const std::string& name)
{
	return pimpl->peek(name);
}
